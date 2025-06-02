#pragma once

#include <stdlib.h>
#include <SDL3/SDL.h>
#include "utils/logger.hpp"
#include "utils/errors.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "types.hpp"
#include <glm/glm.hpp>
#include "vk/types.hpp"
#include <SDL3/SDL_vulkan.h>

namespace vblck
{
    struct System
    {
        SDL_Window *window{};
        VkInstance instance{};
        VkDebugUtilsMessengerEXT debugMessenger{};
        VkPhysicalDevice chosenGPU{};
        VkDevice device{};
        VkSurfaceKHR surface{};
    };
    System initSystem(const char *win_name, int W, int H);

    void finishSystem(const System &system);
}