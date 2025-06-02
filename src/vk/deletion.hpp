#pragma once

#include "types.hpp"
namespace vblck
{
    namespace vk
    {
        struct DeletionQueue
        {
            std::vector<Buffer> buffers;
            std::vector<Image> images;
            std::vector<VkImageView> imageViews;
            std::vector<VkSemaphore> semaphores;
            std::vector<VkFence> fences;

            void deleteQueue(VkDevice device, VmaAllocator vma);
        };
    }
}