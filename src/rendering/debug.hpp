#pragma once

#include "utils/errors.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>
namespace vblck
{
namespace render
{
struct DebugRenderer
{
	enum RenderBuffer
	{
		NOTHING,
		DEPTH,
		ALBEDO,
		NORMAL,
		MATERIAL,
		POSITION
	};
	RenderBuffer renderBuffer;

	void drawDebugUI();

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck