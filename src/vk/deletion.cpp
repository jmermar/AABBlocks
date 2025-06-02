#include "deletion.hpp"
namespace vblck
{
    namespace vk
    {
        void DeletionQueue::deleteQueue(VkDevice device, VmaAllocator vma)
        {
            for (auto fence : fences)
            {
                vkDestroyFence(device, fence, nullptr);
            }
            fences.clear();

            for (auto semaphore : semaphores)
            {
                vkDestroySemaphore(device, semaphore, nullptr);
            }
            semaphores.clear();

            for (auto imageView : imageViews)
            {
                vkDestroyImageView(device, imageView, nullptr);
            }
            imageViews.clear();

            for (auto [image, allocation] : images)
            {
                vmaDestroyImage(vma, image, allocation);
            }
            images.clear();

            for (auto [buffer, allocation] : buffers)
            {
                vmaDestroyBuffer(vma, buffer, allocation);
            }
            buffers.clear();
        }
    }
}