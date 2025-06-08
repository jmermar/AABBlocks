#pragma once

#include "utils/errors.hpp"
#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>
namespace vblck
{
namespace vk
{
struct Buffer
{
	VkBuffer buffer;
	VmaAllocation allocation;
};

struct Image
{
	VkImage image;
	VmaAllocation allocation;
};
} // namespace vk
} // namespace vblck