#include "types.hpp"
#include "vk/vk_raii.hpp"
#include <vector>
#include <functional>
namespace vblck
{
    namespace render
    {
        struct RenderingPlatform
        {
            std::function<VkSurfaceKHR(VkInstance)> createSurface;
        };
        struct Renderer
        {
            RenderingPlatform platform;
            VkInstance instance{};
            VkDebugUtilsMessengerEXT debugMessenger{};
            VkPhysicalDevice chosenGPU{};
            VkDevice device{};
            VkSurfaceKHR surface{};

            vk::RAII_VkSwapchainKHR swapchain;
            VkFormat swapchainImageFormat;

            std::vector<VkImage> swapchainImages;
            std::vector<vk::RAII_VkImageView> swapchainImageViews;
            VkExtent2D swapchainExtent;

            void createSwapchain();
            void destroySwapchain();

            void init(const RenderingPlatform &rp);

            void initVulkan();
            void initSwapchain();
            void initCommands();
            void initSyncStructures();

            void finish();
        };
    }
}