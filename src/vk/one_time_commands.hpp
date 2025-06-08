#pragma once
#include "vulkan/vulkan.h"
#include <functional>
namespace vblck
{
namespace vk
{
void oneTimeSubmission(VkDevice device,
					   VkQueue queue,
					   VkCommandPool pool,
					   std::function<void(VkDevice, VkCommandBuffer)> commands);
}
} // namespace vblck