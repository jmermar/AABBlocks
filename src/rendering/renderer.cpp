#include "renderer.hpp"
#include "utils/files.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
#include "vk/textures.hpp"
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui.h>
constexpr bool bUseValidationLayers = true;
namespace vblck
{
namespace render
{

Renderer* Renderer::renderInstance = 0;
void Renderer::initTextures()
{
	auto screenshotImg = readImageFromFile("res/textures/screenshot.png");
	auto staging = std::make_unique<StagingBuffer>(
		device, vma, &frameDeletionQueue, screenshotImg.data.size());
	std::span<uint8_t> data = screenshotImg.data;
	staging->write(data);
	screenshot = std::make_unique<Texture2D>(
		device, vma, frameDeletionQueue, screenshotImg.w, screenshotImg.h, 4);

	bufferWritter.writeBufferToImage(staging->get(), screenshot.get());
}
void Renderer::destroySwapchain()
{
	backbuffer = 0;
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
	screenshot = 0;
	vkDeviceWaitIdle(device);
	destroySwapchain();
	deletePendingObjects();
	mainDeletionQueue.deleteQueue(device, vma);
	vmaDestroyAllocator(vma);
}

void Renderer::renderLogic(CommandBuffer* cmd)
{
	screenshot->transition(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	backbuffer->transition(cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	backbuffer->copyFromImage(cmd, screenshot.get(), 3);

	backbuffer->transition(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);
	backbuffer->transition(cmd, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
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

void Renderer::recreateSwapchain(int w, int h)
{
	if(swapchain != VK_NULL_HANDLE)
	{
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

	backbuffer = std::make_unique<Texture2D>(device, vma, frameDeletionQueue, w, h, 1);
}

void Renderer::renderFrame()
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

	cmd->begin();

	bufferWritter.performWrites(cmd);

	renderLogic(cmd);

	cmd->transitionImage(swapchainImages[swapchainImageIndex],
						 VK_IMAGE_LAYOUT_UNDEFINED,
						 VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	vk::copyImageToImage(cmd->getCmd(),
						 backbuffer->getImage(),
						 swapchainImages[swapchainImageIndex],
						 backbuffer->getSize(),
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
} // namespace render
} // namespace vblck