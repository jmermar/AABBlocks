#include "renderer.hpp"
#include "utils/files.hpp"
#include "utils/logger.hpp"
#include "vk/buffers.hpp"
#include "vk/descriptors.hpp"
#include "vk/init.hpp"
#include "vk/one_time_commands.hpp"
#include "vk/textures.hpp"
#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
constexpr bool bUseValidationLayers = true;
namespace vblck
{
namespace render
{

struct alignas(16) UniformGlobalData
{
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projViewMatrix;
	glm::mat4 iProjViewMatrix;
	glm::mat4 iViewMatrix;
	glm::mat4 iProjMatrix;
	Frustum camFrustum;
	glm::vec3 camPos;
	float ambient;
	glm::vec3 lightDir;
	float lightIntensity;
	glm::vec3 fogColor;
	float fogIntensity;
	float exposure;
};

void GlobalRenderData::create()
{
	vk::DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(
		0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	globalDescriptorLayout = layoutBuilder.build(
		Renderer::get()->device,
		VK_SHADER_STAGE_ALL_GRAPHICS |
			VK_SHADER_STAGE_COMPUTE_BIT);

	globalBuffer.create(
		sizeof(UniformGlobalData));

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{
		globalDescriptors[i] =
			Renderer::get()->allocateDescriptor(
				globalDescriptorLayout);
	}
}

void GlobalRenderData::destroy()
{
	globalBuffer.destroy();
	vkDestroyDescriptorSetLayout(
		Renderer::get()->device,
		globalDescriptorLayout,
		nullptr);
}

VkDescriptorSet
GlobalRenderData::getGlobalDescriptor()
{
	return globalDescriptors
		[Renderer::get()->getFrameIndex()];
}

void GlobalRenderData::writeDescriptors(
	VkCommandBuffer cmd)
{
	auto* render = Renderer::get();
	vk::DescriptorWriter writer;
	writer.startWrites(2);
	writer.writeBuffer(
		0,
		globalBuffer.buffer.buffer,
		globalBuffer.size,
		globalBuffer.getBaseAddr(
			render->getFrameIndex()),
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	writer.write(render->device,
				 getGlobalDescriptor());
}

Renderer* Renderer::renderInstance = 0;
void Renderer::destroySwapchain()
{
	backbuffer.destroy(&frameDeletionQueue);
	deferredBuffers.destroy(&frameDeletionQueue);
	if(swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(
			device, swapchain, nullptr);
	}
	swapchain = 0;

	for(size_t i = 0;
		i < swapchainImageViews.size();
		i++)
	{

		vkDestroyImageView(device,
						   swapchainImageViews[i],
						   nullptr);
		vkDestroySemaphore(
			device, renderSemaphores[i], nullptr);
	}
	renderSemaphores.clear();
	swapchainImageViews.clear();
	swapchainImages.clear();
}

void Renderer::cleanup()
{
	vkDeviceWaitIdle(device);
	skybox.destroy(&mainDeletionQueue);
	textureAtlas.destroy(&mainDeletionQueue);
	normalAtlas.destroy(&mainDeletionQueue);
	metallicRoughnessAtlas.destroy(
		&mainDeletionQueue);
	deferredRenderer.destroy();
	worldRenderer.destroy();
	renderData.destroy();
	descriptorAllocator.destroy(device);
	destroySwapchain();
	deletePendingObjects();
	mainDeletionQueue.deleteQueue(device, vma);
	vmaDestroyAllocator(vma);
}

void Renderer::renderLogic(CommandBuffer* cmd)
{
	backbuffer.transition(
		cmd->getCmd(), VK_IMAGE_LAYOUT_GENERAL);
	backbuffer.clearColor(
		cmd->getCmd(), 0, 0, 0, 0);
	backbuffer.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	worldRenderer.render(cmd->getCmd());

	deferredBuffers.albedo.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredBuffers.normal.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredBuffers.pos.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredBuffers.material.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	deferredBuffers.depthBuffer.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	skybox.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	deferredRenderer.render(cmd->getCmd());

	if(state.drawDebug)
	{
		debugRenderer.render(cmd->getCmd());
	}
}

void Renderer::renderImGUI(
	VkCommandBuffer cmd,
	VkImageView targetImageView)
{
	VkRenderingAttachmentInfo colorAttachment =
		vk::Init::attachementInfo(
			targetImageView,
			nullptr,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo{};
	renderInfo.sType =
		VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments =
		&colorAttachment;
	renderInfo.renderArea =
		VkRect2D{VkOffset2D{0, 0}, screenExtent};
	renderInfo.layerCount = 1;

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(
		ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}

void Renderer::imGUIDefaultRender()
{
	if(state.drawDebug)
	{
		debugRenderer.drawDebugUI();
	}
}

void Renderer::initRenderers()
{
	worldRenderer.create();
	deferredRenderer.create();
}

vk::Texture2D
Renderer::loadTexture2D(const char* path)
{
	auto image = readImageFromFile(path);
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, image.data.size());
	buffer.write((std::span<uint8_t>)image.data);
	vk::Texture2D tex;
	tex.createRGBATexture(
		device, vma, {image.w, image.h}, 4);
	bufferWritter.writeBufferToImage(
		buffer.data.buffer, tex);
	buffer.destroy(&frameDeletionQueue);
	return tex;
}

vk::Texture2D Renderer::loadTexture2DArray(
	const char* path, int ncols, int nrows)
{
	auto image = readImageArrayFromFile(
		path, ncols, nrows);
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, image.data.size());
	buffer.write((std::span<uint8_t>)image.data);
	vk::Texture2D tex;
	tex.createRGBATexture(device,
						  vma,
						  {image.w, image.h},
						  image.layers,
						  4);
	bufferWritter.writeBufferToTexture2DArray(
		buffer.data.buffer, tex);
	buffer.destroy(&frameDeletionQueue);
	return tex;
}

vk::Texture2D Renderer::loadTextureFromImageArray(
	ImageArrayData& data)
{
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, data.data.size());
	buffer.write((std::span<uint8_t>)data.data);
	vk::Texture2D tex;
	tex.createRGBATexture(device,
						  vma,
						  {data.w, data.h},
						  data.layers,
						  4);
	bufferWritter.writeBufferToTexture2DArray(
		buffer.data.buffer, tex);
	buffer.destroy(&frameDeletionQueue);
	return tex;
}

vk::Texture2D
Renderer::loadCubeMap(const char* path)
{
	auto image = readCubeMapFromFile(path);
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, image.data.size());
	buffer.write((std::span<uint8_t>)image.data);
	vk::Texture2D tex;
	tex.createCubeMap(
		device, vma, {image.w, image.h}, 1);
	bufferWritter.writeBufferToImage(
		buffer.data.buffer, tex);
	buffer.destroy(&frameDeletionQueue);
	return tex;
}

void Renderer::recreateSwapchain(int w, int h)
{
	if(swapchain != VK_NULL_HANDLE)
	{
		vk::oneTimeSubmission(
			device,
			graphicsQueue,
			getCurrentFrame().commandPool,
			[&](VkDevice device,
				VkCommandBuffer cmd) {
				bufferWritter.performWrites(cmd);
			});
		vkDeviceWaitIdle(device);
		deletePendingObjects();
		destroySwapchain();
	}

	vkb::SwapchainBuilder swapchainBuilder{
		chosenGPU, device, surface};

	swapchainImageFormat =
		VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain =
		swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{
				.format = swapchainImageFormat,
				.colorSpace =
					VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
			.set_desired_present_mode(
				VK_PRESENT_MODE_IMMEDIATE_KHR)
			.set_desired_extent(w, h)
			.add_image_usage_flags(
				VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

	screenExtent = vkbSwapchain.extent;
	swapchain = vkbSwapchain.swapchain;
	swapchainImages =
		vkbSwapchain.get_images().value();
	swapchainImageViews =
		vkbSwapchain.get_image_views().value();

	renderSemaphores.resize(
		swapchainImages.size());
	VkSemaphoreCreateInfo semaphoreCreateInfo =
		vk::Init::semaphoreCreateInfo();
	for(size_t i = 0; i < swapchainImages.size();
		i++)
	{
		VKTRY(vkCreateSemaphore(
			device,
			&semaphoreCreateInfo,
			nullptr,
			&renderSemaphores[i]));
	}

	backbuffer.createRGBATexture(
		device, vma, screenExtent, 1);
	deferredBuffers.create(
		device, vma, screenExtent);
	deferredRenderer.writeDescriptorSets();
}

void Renderer::renderFrame(RenderState& state)
{
	this->state = state;
	computedUtils.camFrustum =
		state.cullCamera.getFrustum();
	vkWaitForFences(
		device,
		1,
		&getCurrentFrame().renderFence,
		true,
		1000000000);
	vkResetFences(device,
				  1,
				  &getCurrentFrame().renderFence);

	getCurrentFrame().deletionQueue.deleteQueue(
		device, vma);
	getCurrentFrame().deletionQueue =
		std::move(frameDeletionQueue);

	uint32_t swapchainImageIndex;

	VkResult e = vkAcquireNextImageKHR(
		device,
		swapchain,
		1000000000,
		getCurrentFrame().swapchainSemaphore,
		nullptr,
		&swapchainImageIndex);
	if(e == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain(screenExtent.width,
						  screenExtent.height);
		return;
	}

	VKTRY(vkResetFences(
		device,
		1,
		&getCurrentFrame().renderFence));

	auto* cmd =
		getCurrentFrame().mainCommandBuffer.get();

	UniformGlobalData uniformGlobalData;
	uniformGlobalData.viewMatrix =
		state.camera.getView();
	uniformGlobalData.projMatrix =
		state.camera.getProjection();
	uniformGlobalData.projViewMatrix =
		uniformGlobalData.projMatrix *
		uniformGlobalData.viewMatrix;
	uniformGlobalData.camFrustum =
		computedUtils.camFrustum;
	uniformGlobalData.camPos =
		glm::vec4(state.camera.position, 1.0f);
	uniformGlobalData.ambient =
		debugRenderer.ambient;
	uniformGlobalData.lightDir =
		debugRenderer.lightDir;
	uniformGlobalData.lightIntensity =
		debugRenderer.lightIntensity;
	uniformGlobalData.exposure =
		debugRenderer.exposure;
	uniformGlobalData.iProjViewMatrix =
		glm::inverse(
			uniformGlobalData.projViewMatrix);
	uniformGlobalData.iViewMatrix = glm::inverse(
		uniformGlobalData.viewMatrix);
	uniformGlobalData.iProjMatrix = glm::inverse(
		uniformGlobalData.projMatrix);
	uniformGlobalData.fogColor =
		debugRenderer.fogColor;
	uniformGlobalData.fogIntensity =
		debugRenderer.fogIntensity;

	memcpy(renderData.globalBuffer.getData(),
		   &uniformGlobalData,
		   sizeof(UniformGlobalData));

	renderData.globalBuffer.flush();

	cmd->begin();
	renderData.writeDescriptors(cmd->getCmd());

	bufferWritter.performWrites(cmd->getCmd());

	renderLogic(cmd);

	backbuffer.transition(
		cmd->getCmd(),
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

	cmd->transitionImage(
		swapchainImages[swapchainImageIndex],
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vk::copyImageToImage(
		cmd->getCmd(),
		backbuffer.data.image,
		swapchainImages[swapchainImageIndex],
		backbuffer.extent,
		screenExtent,
		0,
		0);

	cmd->transitionImage(
		swapchainImages[swapchainImageIndex],
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	renderImGUI(
		cmd->getCmd(),
		swapchainImageViews[swapchainImageIndex]);
	cmd->transitionImage(
		swapchainImages[swapchainImageIndex],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd->submit(
		graphicsQueue,
		getCurrentFrame().renderFence,
		renderSemaphores[swapchainImageIndex],
		getCurrentFrame().swapchainSemaphore);

	VkPresentInfoKHR presentInfo =
		vk::Init::presentInfo();

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores =
		&renderSemaphores[swapchainImageIndex];
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices =
		&swapchainImageIndex;

	vkQueuePresentKHR(graphicsQueue,
					  &presentInfo);

	frameNumber++;
}

void Renderer::initVMA()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = chosenGPU;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.flags =
		VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	VKTRY(
		vmaCreateAllocator(&allocatorInfo, &vma));
}

void Renderer::initCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo =
		vk::Init::commandPoolCreateInfo(
			graphicsQueueFamily,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{

		VKTRY(vkCreateCommandPool(
			device,
			&commandPoolInfo,
			nullptr,
			&frames[i].commandPool));

		frames[i].mainCommandBuffer =
			std::make_unique<CommandBuffer>(
				device, frames[i].commandPool);

		mainDeletionQueue.commandPools.push_back(
			frames[i].commandPool);
	}
}
void Renderer::initSyncStructures()
{
	VkFenceCreateInfo fenceCreateInfo =
		vk::Init::fenceCreateInfo(
			VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo =
		vk::Init::semaphoreCreateInfo();

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{
		VKTRY(vkCreateFence(
			device,
			&fenceCreateInfo,
			nullptr,
			&frames[i].renderFence));

		VKTRY(vkCreateSemaphore(
			device,
			&semaphoreCreateInfo,
			nullptr,
			&frames[i].swapchainSemaphore));

		mainDeletionQueue.fences.push_back(
			frames[i].renderFence);
		mainDeletionQueue.semaphores.push_back(
			frames[i].swapchainSemaphore);
	}
}
glm::mat4 Camera::getProjection()
{
	return glm::perspectiveFov(
		fov, aspect, 1.f, znear, zfar);
}
glm::mat4 Camera::getView()
{
	return glm::lookAt(position,
					   position + forward,
					   glm::vec3(0.f, -1.f, 0.f));
}
Frustum Camera::getFrustum()
{
	auto viewProjMatrix =
		getProjection() * getView();
	Frustum frustum;
	glm::vec4 rowX = glm::row(viewProjMatrix, 0);
	glm::vec4 rowY = glm::row(viewProjMatrix, 1);
	glm::vec4 rowZ = glm::row(viewProjMatrix, 2);
	glm::vec4 rowW = glm::row(viewProjMatrix, 3);

	// Left
	frustum.left.normal = glm::vec3(rowW + rowX);
	frustum.left.d = (rowW + rowX).w;

	// Right
	frustum.right.normal = glm::vec3(rowW - rowX);
	frustum.right.d = (rowW - rowX).w;

	// Bottom
	frustum.bottom.normal =
		glm::vec3(rowW + rowY);
	frustum.bottom.d = (rowW + rowY).w;

	// Top
	frustum.top.normal = glm::vec3(rowW - rowY);
	frustum.top.d = (rowW - rowY).w;

	// Near
	frustum.front.normal = glm::vec3(rowW + rowZ);
	frustum.front.d = (rowW + rowZ).w;

	// Far
	frustum.back.normal = glm::vec3(rowW - rowZ);
	frustum.back.d = (rowW - rowZ).w;

	frustum.back.norm();

	return frustum;
}
void DeferredBuffers::create(VkDevice device,
							 VmaAllocator vma,
							 VkExtent2D size)
{
	depthBuffer.createDepthTexture(
		device, vma, size, 1);
	albedo.createRGBATexture(
		device, vma, size, 1);
	normal.createRGBATexture(
		device, vma, size, 1);
	material.createRGBATexture(
		device, vma, size, 1);
	pos.createRGBA32Texture(device, vma, size, 1);
}

void DeferredBuffers::destroy(
	vk::DeletionQueue* deletion)
{
	depthBuffer.destroy(deletion);
	albedo.destroy(deletion);
	normal.destroy(deletion);
	material.destroy(deletion);
	pos.destroy(deletion);
}
} // namespace render
} // namespace vblck