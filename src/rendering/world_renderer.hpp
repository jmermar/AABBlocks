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
	VkDevice device{};
	VmaAllocator vma{};
	vk::DescriptorAllocator descriptorAllocator;
	ChunkRenderer chunkRenderer;

	void initDescriptorPool();
	void create();

	WorldRenderer(VkDevice device, VmaAllocator vma)
		: device(device)
		, vma(vma)
	{
		create();
	}
	~WorldRenderer()
	{
		chunkRenderer.destroy();
		vkDestroyDescriptorPool(device, descriptorAllocator.pool, nullptr);
	}

	void clearWorld()
	{
		chunkRenderer.clearData();
	}

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck