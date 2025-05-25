#include "renderer.hpp"
#include <VkBootstrap.h>
#include <SDL3/SDL_vulkan.h>
#include "utils/logger.hpp"
constexpr bool bUseValidationLayers = true;
namespace vblck
{
    namespace render
    {
        void Renderer::createSwapchain()
        {
            vkb::SwapchainBuilder swapchainBuilder{chosenGPU, device, surface};

            swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

            vkb::Swapchain vkbSwapchain = swapchainBuilder
                                              //.use_default_format_selection()
                                              .set_desired_format(VkSurfaceFormatKHR{.format = swapchainImageFormat, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
                                              // use vsync present mode
                                              .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
                                              .set_desired_extent(640, 480)
                                              .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
                                              .build()
                                              .value();

            swapchainExtent = vkbSwapchain.extent;
            // store swapchain and its related images
            swapchain.set(device, vkbSwapchain.swapchain);
            swapchainImages = vkbSwapchain.get_images().value();

            auto rawImageViews = vkbSwapchain.get_image_views().value();
            swapchainImageViews.resize(rawImageViews.size());
            for (size_t i = 0; i < swapchainImages.size(); i++)
            {
                swapchainImageViews[i].set(device, rawImageViews[i]);
            }

            LOG_INFO("Swapchain created");
        }

        void Renderer::destroySwapchain()
        {
            swapchain = {};
            swapchainImageViews.clear();
            swapchainImages.clear();
            LOG_INFO("Swapchain destroyed");
        }

        void Renderer::init(const RenderingPlatform &rp)
        {
            platform = rp;
            initVulkan();
            initSwapchain();
            initCommands();
            initSyncStructures();
        }

        void Renderer::initVulkan()
        {
            vkb::InstanceBuilder builder;

            // make the vulkan instance, with basic debug features
            auto inst_ret = builder.set_app_name("Example Vulkan Application")
                                .request_validation_layers(bUseValidationLayers)
                                .use_default_debug_messenger()
                                .require_api_version(1, 3, 0)
                                .build();

            vkb::Instance vkb_inst = inst_ret.value();

            // grab the instance
            instance = vkb_inst.instance;
            debugMessenger = vkb_inst.debug_messenger;

            surface = platform.createSurface(instance);

            // vulkan 1.3 features
            VkPhysicalDeviceVulkan13Features features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
            features.dynamicRendering = true;
            features.synchronization2 = true;

            // vulkan 1.2 features
            VkPhysicalDeviceVulkan12Features features12{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
            features12.bufferDeviceAddress = true;
            features12.descriptorIndexing = true;

            // use vkbootstrap to select a gpu.
            // We want a gpu that can write to the SDL surface and supports vulkan 1.3 with the correct features
            vkb::PhysicalDeviceSelector selector{vkb_inst};
            vkb::PhysicalDevice physicalDevice = selector
                                                     .set_minimum_version(1, 3)
                                                     .set_required_features_13(features)
                                                     .set_required_features_12(features12)
                                                     .set_surface(surface)
                                                     .select()
                                                     .value();

            // create the final vulkan device
            vkb::DeviceBuilder deviceBuilder{physicalDevice};

            vkb::Device vkbDevice = deviceBuilder.build().value();

            // Get the VkDevice handle used in the rest of a vulkan application
            device = vkbDevice.device;
            chosenGPU = physicalDevice.physical_device;

            LOG_INFO("Vulkan initialized");
        }
        void Renderer::initSwapchain()
        {
            createSwapchain();
        }
        void Renderer::initCommands()
        {
        }
        void Renderer::initSyncStructures()
        {
        }
        void Renderer::finish()
        {
            destroySwapchain();

            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(instance, surface, nullptr);

            vkb::destroy_debug_utils_messenger(instance, debugMessenger);
            vkDestroyInstance(instance, nullptr);

            LOG_INFO("Vulkan finished");
        }
    }
}