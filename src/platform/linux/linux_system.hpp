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
class LinuxSystem
{
public:
	static void init(int w, int h);
};
} // namespace vblck