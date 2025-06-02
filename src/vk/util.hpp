#pragma once

#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>
namespace vblck
{
namespace vk
{
/* This odd ugly code is for a validation error generated in the first frame
    * for waiting to newly created semaphores, as they are unsignaled. This solution is ugly as
    it requires vkWaitDeviceIddle and a queue submit per semaphore, but it works for now without
    adding much complexity to the code*/
class Util
{
public:
};
} // namespace vk
} // namespace vblck