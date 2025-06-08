#pragma once

#include "utils/errors.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>
namespace vblck
{
namespace render
{
class CommandBuffer
{
private:
	VkDevice device;
	VkCommandPool pool;
	VkCommandBuffer cmd;

public:
	CommandBuffer(VkDevice device, VkCommandPool pool);
	~CommandBuffer() { }

	inline VkCommandBuffer getCmd()
	{
		return cmd;
	}

	inline void begin()
	{
		VKTRY(vkResetCommandBuffer(cmd, 0));

		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;

		info.pInheritanceInfo = nullptr;
		info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VKTRY(vkBeginCommandBuffer(cmd, &info));
	}

	void submit(VkQueue queue,
				VkFence renderFence,
				VkSemaphore renderSemaphore,
				VkSemaphore swapchainSemaphore);

	void transitionImage(VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
};
} // namespace render
} // namespace vblck