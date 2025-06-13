#pragma once
#include "utils/errors.hpp"
#include <span>
#include <vector>
#include <vulkan/vulkan.h>
namespace vblck
{
namespace vk
{
VkPipeline createComputePipeline(VkDevice device, VkPipelineLayout layout, VkShaderModule compute);
} // namespace vk
} // namespace vblck