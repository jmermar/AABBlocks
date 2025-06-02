#include "renderer.hpp"
#include "render_command_buffer.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
#include "vk/util.hpp"
#include <SDL3/SDL_vulkan.h>
#include <VkBootstrap.h>
constexpr bool bUseValidationLayers = true;
namespace vblck
{
namespace render
{

void Renderer::destroySwapchain()
{
	if(swapchain != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(device, swapchain, nullptr);
	}
	swapchain = 0;

	for(size_t i = 0; i < swapchainImageViews.size(); i++)
	{

		vkDestroyImageView(device, swapchainImageViews[i], nullptr);
	}
	swapchainImageViews.clear();
	swapchainImages.clear();
}

void Renderer::cleanup()
{
	vkDeviceWaitIdle(device);
	destroySwapchain();
	mainDeletionQueue.deleteQueue(device, vma);
}

void Renderer::recreateSwapchain(int w, int h)
{
	if(swapchain != VK_NULL_HANDLE)
	{
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
}

void Renderer::renderFrame()
{
	vkWaitForFences(device, 1, &getCurrentFrame().renderFence, true, 1000000000);
	vkResetFences(device, 1, &getCurrentFrame().renderFence);

	getCurrentFrame().deletionQueue.deleteQueue(device, vma);

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

	vkResetFences(device, 1, &getCurrentFrame().renderFence);

	RenderCommandBuffer cmd(device,
							graphicsQueue,
							getCurrentFrame().mainCommandBuffer,
							getCurrentFrame().swapchainSemaphore,
							getCurrentFrame().renderSemaphore,
							getCurrentFrame().renderFence);

	cmd.begin();

	cmd.transitionImage(
		swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	//make a clear-color from frame number. This will flash with a 120 frame period.
	VkClearColorValue clearValue;
	float flash = std::abs(std::sin(frameNumber / 120.f));
	clearValue = {{0.0f, 0.0f, flash, 1.0f}};

	VkImageSubresourceRange clearRange = vk::Init::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);

	//clear image
	vkCmdClearColorImage(cmd.getCmd(),
						 swapchainImages[swapchainImageIndex],
						 VK_IMAGE_LAYOUT_GENERAL,
						 &clearValue,
						 1,
						 &clearRange);

	//make the swapchain image into presentable mode
	cmd.transitionImage(swapchainImages[swapchainImageIndex],
						VK_IMAGE_LAYOUT_GENERAL,
						VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	cmd.submit();

	VkPresentInfoKHR presentInfo = vk::Init::presentInfo();

	presentInfo.pSwapchains = &swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &getCurrentFrame().renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	vkQueuePresentKHR(graphicsQueue, &presentInfo);

	frameNumber++;
}

void Renderer::initCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = vk::Init::commandPoolCreateInfo(
		graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{

		vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frames[i].commandPool);

		VkCommandBufferAllocateInfo cmdAllocInfo =
			vk::Init::commandBufferCreateInfo(frames[i].commandPool);

		vkAllocateCommandBuffers(device, &cmdAllocInfo, &frames[i].mainCommandBuffer);

		mainDeletionQueue.commandPools.push_back(frames[i].commandPool);
	}
}
void Renderer::initSyncStructures()
{
	VkFenceCreateInfo fenceCreateInfo = vk::Init::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vk::Init::semaphoreCreateInfo();

	for(size_t i = 0; i < FRAME_OVERLAP; i++)
	{
		vkCreateFence(device, &fenceCreateInfo, nullptr, &frames[i].renderFence);

		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].swapchainSemaphore);

		vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &frames[i].renderSemaphore);
		mainDeletionQueue.fences.push_back(frames[i].renderFence);
		mainDeletionQueue.semaphores.push_back(frames[i].renderSemaphore);
		mainDeletionQueue.semaphores.push_back(frames[i].swapchainSemaphore);
	}
}
} // namespace render
} // namespace vblck