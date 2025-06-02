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
                }

        void Renderer::destroySwapchain()
        {
        }

        void Renderer::init(const RenderingPlatform &rp)
        {
            initVulkan();
            initSwapchain();
            initCommands();
            initSyncStructures();
        }

        void Renderer::initVulkan()
        {

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

            LOG_INFO("Vulkan finished");
        }
    }
}