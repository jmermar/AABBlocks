#include "one_time_commands.hpp"
#include "utils/errors.hpp"

namespace vblck
{
namespace vk
{

void oneTimeSubmission(VkDevice device,
					   VkQueue queue,
					   VkCommandPool pool,
					   std::function<void(VkDevice, VkCommandBuffer)> commands)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VKTRY(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VKTRY(vkBeginCommandBuffer(commandBuffer, &beginInfo));

	commands(device, commandBuffer);

	VKTRY(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VKTRY(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));
	VKTRY(vkQueueWaitIdle(queue));

	vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
}
} // namespace vk
} // namespace vblck