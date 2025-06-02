#include "types.hpp"
#include <vector>
#include <functional>
namespace vblck
{
    namespace render
    {
        struct RenderingPlatform
        {
        };
        struct Renderer
        {
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