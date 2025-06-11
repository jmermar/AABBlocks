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
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
constexpr bool bUseValidationLayers = true;
namespace vblck
{
namespace render
{

struct UniformGlobalData
{
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projViewMatrix;
};

void GlobalRenderData::create()
{
	vk::DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	globalDescriptorLayout =
		layoutBuilder.build(Renderer::get()->device, VK_SHADER_STAGE_ALL_GRAPHICS);

	globalBuffer.create(sizeof(UniformGlobalData));
	std::vector<vk::DescriptorAllocator::PoolSizeRatio> ratios;
	ratios.resize(3);
	ratios[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ratios[0].ratio = 1;
	ratios[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ratios[1].ratio = 1;
	ratios[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ratios[2].ratio = 1;

	allocator.initPool(Renderer::get()->device, 8, ratios);

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{
		globalDescriptors[i] = allocator.allocate(Renderer::get()->device, globalDescriptorLayout);
	}
}

void GlobalRenderData::destroy()
{
	globalBuffer.destroy();
	vkDestroyDescriptorPool(Renderer::get()->device, allocator.pool, nullptr);
	vkDestroyDescriptorSetLayout(Renderer::get()->device, globalDescriptorLayout, nullptr);
}

VkDescriptorSet GlobalRenderData::getGlobalDescriptor()
{
	return globalDescriptors[Renderer::get()->getFrameIndex()];
}

void GlobalRenderData::writeDescriptors(VkCommandBuffer cmd)
{
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = globalBuffer.buffer.buffer;
	bufferInfo.offset = globalBuffer.getBaseAddr(Renderer::get()->getFrameIndex());
	bufferInfo.range = globalBuffer.alignedSize;

	VkWriteDescriptorSet write = {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write.dstSet = getGlobalDescriptor();
	write.descriptorCount = 1;
	write.dstBinding = 0;
	write.dstArrayElement = 0;
	write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	write.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(Renderer::get()->device, 1, &write, 0, 0);
}

Renderer* Renderer::renderInstance = 0;
void Renderer::destroySwapchain()
{
	backbuffer.destroy(&frameDeletionQueue);
	if(swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
	swapchain = 0;

	for(size_t i = 0; i < swapchainImageViews.size(); i++)
	{

		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
		vkDestroySemaphore(device, renderSemaphores[i], nullptr);
	}
	renderSemaphores.clear();
	swapchainImageViews.clear();
	swapchainImages.clear();
}

void Renderer::cleanup()
{
	vkDeviceWaitIdle(device);
	textureAtlas.destroy(&mainDeletionQueue);
	worldRenderer = 0;
	renderData.destroy();
	destroySwapchain();
	deletePendingObjects();
	mainDeletionQueue.deleteQueue(device, vma);
	vmaDestroyAllocator(vma);
}

void Renderer::renderLogic(CommandBuffer* cmd)
{
	backbuffer.transition(cmd->getCmd(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	backbuffer.clear(cmd->getCmd(), 0, 0, 0);
	backbuffer.transition(
		cmd->getCmd(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	worldRenderer->render(cmd->getCmd());

	backbuffer.transition(cmd->getCmd(),
						  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
}

void Renderer::renderImGUI(VkCommandBuffer cmd, VkImageView targetImageView)
{
	VkRenderingAttachmentInfo colorAttachment = vk::Init::attachementInfo(
		targetImageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachment;
	renderInfo.renderArea = VkRect2D{VkOffset2D{0, 0}, screenExtent};
	renderInfo.layerCount = 1;

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

	vkCmdEndRendering(cmd);
}

void Renderer::initRenderers()
{
	worldRenderer = std::make_unique<WorldRenderer>(device, vma);
}

vk::Texture2D Renderer::loadTexture2D(const char* path)
{
	auto image = readImageFromFile(path);
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, image.data.size());
	buffer.write((std::span<uint8_t>)image.data);
	vk::Texture2D tex;
	tex.createTexture(device, vma, {image.w, image.h}, 4);
	bufferWritter.writeBufferToImage(buffer.data.buffer, tex);
	buffer.destroy(&frameDeletionQueue);
	return tex;
}

vk::Texture2DArray Renderer::loadTexture2DArray(const char* path, int ncols, int nrows)
{
	auto image = readImageArrayFromFile(path, ncols, nrows);
	vk::StagingBuffer buffer{};
	buffer.create(device, vma, image.data.size());
	buffer.write((std::span<uint8_t>)image.data);
	vk::Texture2DArray tex;
	tex.createTexture(device, vma, {image.w, image.h}, image.layers, 4);
	bufferWritter.writeBufferToTexture2DArray(buffer.data.buffer, tex);
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
			[&](VkDevice device, VkCommandBuffer cmd) { bufferWritter.performWrites(cmd); });
		vkDeviceWaitIdle(device);
		deletePendingObjects();
		destroySwapchain();
	}

	vkb::SwapchainBuilder swapchainBuilder{chosenGPU, device, surface};

	swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

	vkb::Swapchain vkbSwapchain =
		swapchainBuilder
			.set_desired_format(VkSurfaceFormatKHR{.format = swapchainImageFormat,
												   .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
			.set_desired_extent(w, h)
			.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
			.build()
			.value();

	screenExtent = vkbSwapchain.extent;
	swapchain = vkbSwapchain.swapchain;
	swapchainImages = vkbSwapchain.get_images().value();
	swapchainImageViews = vkbSwapchain.get_image_views().value();

	renderSemaphores.resize(swapchainImages.size());
	VkSemaphoreCreateInfo semaphoreCreateInfo = vk::Init::semaphoreCreateInfo();
	for(size_t i = 0; i < swapchainImages.size(); i++)
	{
		VKTRY(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderSemaphores[i]));
	}

	backbuffer.createTexture(device, vma, VkExtent2D{(unsigned int)w, (unsigned int)h}, 1);
}

void Renderer::renderFrame(RenderSate& state)
{
	vkWaitForFences(device, 1, &getCurrentFrame().renderFence, true, 1000000000);
	vkResetFences(device, 1, &getCurrentFrame().renderFence);

	getCurrentFrame().deletionQueue.deleteQueue(device, vma);
	getCurrentFrame().deletionQueue = std::move(frameDeletionQueue);

	uint32_t swapchainImageIndex;

	VkResult e = vkAcquireNextImageKHR(device,
									   swapchain,
									   1000000000,
									   getCurrentFrame().swapchainSemaphore,
									   nullptr,
									   &swapchainImageIndex);
	if(e == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapchain(screenExtent.width, screenExtent.height);
		return;
	}

	VKTRY(vkResetFences(device, 1, &getCurrentFrame().renderFence));

	auto* cmd = getCurrentFrame().mainCommandBuffer.get();

	auto* uniformGlobalData = (UniformGlobalData*)renderData.globalBuffer.getData();
	uniformGlobalData->viewMatrix = state.camera.getView();
	uniformGlobalData->projMatrix = state.camera.getProjection();
	uniformGlobalData->projViewMatrix =
		uniformGlobalData->projMatrix * uniformGlobalData->viewMatrix;

	cmd->begin();
	renderData.writeDescriptors(cmd->getCmd());

	bufferWritter.performWrites(cmd->getCmd());

	renderLogic(cmd);

	cmd->transitionImage(swapchainImages[swapchainImageIndex],
						 VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vk::copyImageToImage(cmd->getCmd(),
						 backbuffer.data.image,
						 swapchainImages[swapchainImageIndex],
						 backbuffer.extent,
						 screenExtent,
						 0,
						 0);

	cmd->transitionImage(swapchainImages[swapchainImageIndex],
						 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	renderImGUI(cmd->getCmd(), swapchainImageViews[swapchainImageIndex]);
	cmd->transitionImage(swapchainImages[swapchainImageIndex],
						 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
						 VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd->submit(graphicsQueue,
				getCurrentFrame().renderFence,
				renderSemaphores[swapchainImageIndex],
				getCurrentFrame().swapchainSemaphore);

	VkPresentInfoKHR presentInfo = vk::Init::presentInfo();

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &renderSemaphores[swapchainImageIndex];
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	vkQueuePresentKHR(graphicsQueue, &presentInfo);

	frameNumber++;
}

void Renderer::initVMA()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = chosenGPU;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	VKTRY(vmaCreateAllocator(&allocatorInfo, &vma));
}

void Renderer::initCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = vk::Init::commandPoolCreateInfo(
		graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{

		VKTRY(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool));

		frames[i].mainCommandBuffer =
			std::make_unique<CommandBuffer>(device, frames[i].commandPool);

		mainDeletionQueue.commandPools.push_back(frames[i].commandPool);
	}
}
void Renderer::initSyncStructures()
{
	VkFenceCreateInfo fenceCreateInfo = vk::Init::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vk::Init::semaphoreCreateInfo();

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{
		VKTRY(vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence));

		VKTRY(vkCreateSemaphore(
			device, &semaphoreCreateInfo, nullptr, &frames[i].swapchainSemaphore));

		mainDeletionQueue.fences.push_back(frames[i].renderFence);
		mainDeletionQueue.semaphores.push_back(frames[i].swapchainSemaphore);
	}
}
glm::mat4 Camera::getProjection()
{
	return glm::perspectiveFov(fov, aspect, 1.f, znear, zfar);
}
glm::mat4 Camera::getView()
{
	return glm::lookAt(position, position + forward, glm::vec3(0.f, 1.f, 0.f));
}
} // namespace render
} // namespace vblck