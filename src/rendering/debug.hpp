#pragma once

#include "utils/errors.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
#include "vulkan/vulkan.hpp"
#include <glm/glm.hpp>
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

	float ambient = 0.5f;
	glm::vec3 lightDir = {1.f, 1.f, 3.f};
	float lightIntensity = 0.8f;

	float exposure = 0.6f;

	void drawDebugUI();

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck