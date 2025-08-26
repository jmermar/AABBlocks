#pragma once
#include "chunk_renderer.hpp"
#include "vk/descriptors.hpp"
#include "vk/textures.hpp"
#include "vk/types.hpp"
namespace vblck
{
namespace render
{
struct WorldRenderer
{
	ChunkRenderer chunkRenderer;

	void initDescriptorPool();
	void create();

	void destroy();

	void clearWorld()
	{
		chunkRenderer.clearData();
	}

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck