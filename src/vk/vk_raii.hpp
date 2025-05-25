#include "types.hpp"
#include <vulkan/vulkan.h>
namespace vblck
{
    namespace vk
    {
        template <typename Raw, void (*Des)(VkDevice, Raw, const VkAllocationCallbacks *)>
        class RAII
        {
        public:
            RAII()
                : raw(nullptr)
            {
            }

            RAII(VkDevice device, Raw raw)
                : raw(raw), device(device)
            {
            }

            ~RAII()
            {
                Destroy();
            }

            void set(VkDevice device, Raw raw)
            {
                Destroy();
                this->device = device;
                this->raw = raw;
            }

            RAII &operator=(const RAII &other) = delete;

            RAII(const RAII &other) = delete;

            RAII &operator=(RAII &&other)
            {
                Destroy();
                raw = other.raw;
                device = other.device;
                other.raw = nullptr;
                other.device = nullptr;
                return *this;
            }

            RAII(RAII &&other)
                : raw(other.raw)
            {
                other.raw = nullptr;
            }

            operator bool() const { return raw; }
            const Raw &operator*() const { return raw; }
            Raw &operator*() { return raw; }
            const Raw *operator->() const { return &raw; }
            Raw *operator->() { return &raw; }

        private:
            void Destroy()
            {
                if (!raw)
                    return;
                Des(device, raw, nullptr);
                raw = nullptr;
            }

        private:
            Raw raw;
            VkDevice device;
        };

#define MAKE_RAII(vulkanType, delFun) using RAII_##vulkanType = RAII<vulkanType, delFun>;

        MAKE_RAII(VkBuffer, vkDestroyBuffer);
        MAKE_RAII(VkImage, vkDestroyImage);
        MAKE_RAII(VkFence, vkDestroyFence);
        MAKE_RAII(VkSemaphore, vkDestroySemaphore);
        MAKE_RAII(VkImageView, vkDestroyImageView);
        MAKE_RAII(VkPipeline, vkDestroyPipeline);
        MAKE_RAII(VkPipelineLayout, vkDestroyPipelineLayout);
        MAKE_RAII(VkSwapchainKHR, vkDestroySwapchainKHR)
#undef MAKE_RAII
    }
}