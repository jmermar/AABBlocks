#pragma once

#include "types.hpp"
#include "utils/errors.hpp"
#include "utils/logger.hpp"
#include "vk/types.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <glm/glm.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdlib.h>

namespace vblck
{
struct System
{
	SDL_Window* window{};
	VkInstance instance{};
	VkDebugUtilsMessengerEXT debugMessenger{};
	VkPhysicalDevice chosenGPU{};
	VkDevice device{};
	VkSurfaceKHR surface{};
	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;
};
System initSystemLinux(const char* win_name, int W, int H);

void finishSystemLinux(const System& system);
} // namespace vblck