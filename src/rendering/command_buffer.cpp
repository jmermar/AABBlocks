
#include "command_buffer.hpp"

namespace vblck
{
namespace render
{
CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool pool)
	: device(device)
	, pool(pool)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = 1;
	info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	vkAllocateCommandBuffers(device, &info, &cmd);
}
void CommandBuffer::submit(VkQueue queue,
						   VkFence renderFence,
						   VkSemaphore renderSemaphore,
						   VkSemaphore swapchainSemaphore)
{
	vkEndCommandBuffer(cmd);

	VkCommandBufferSubmitInfo cmdinfo = vk::Init::commandBufferSubmitInfo(cmd);

	VkSemaphoreSubmitInfo signalInfo =
		vk::Init::semaphoreSubmitInfo(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, renderSemaphore);

	VkSemaphoreSubmitInfo waitInfo = vk::Init::semaphoreSubmitInfo(
		VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, swapchainSemaphore);

	VkSubmitInfo2 submit = vk::Init::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

	vkQueueSubmit2(queue, 1, &submit, renderFence);
}
void CommandBuffer::transitionImage(VkImage image,
									VkImageLayout currentLayout,
									VkImageLayout newLayout)
{
	VkImageMemoryBarrier2 imageBarrier{.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
	imageBarrier.pNext = nullptr;

	imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
	imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

	imageBarrier.oldLayout = currentLayout;
	imageBarrier.newLayout = newLayout;

	VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
										? VK_IMAGE_ASPECT_DEPTH_BIT
										: VK_IMAGE_ASPECT_COLOR_BIT;

	imageBarrier.subresourceRange = vk::Init::imageSubresourceRange(aspectMask);
	imageBarrier.image = image;

	VkDependencyInfo depInfo{};
	depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	depInfo.pNext = nullptr;

	depInfo.imageMemoryBarrierCount = 1;
	depInfo.pImageMemoryBarriers = &imageBarrier;

	vkCmdPipelineBarrier2(cmd, &depInfo);
};
} // namespace render
} // namespace vblck