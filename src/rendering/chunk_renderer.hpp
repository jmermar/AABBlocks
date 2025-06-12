#pragma once
#include "types.hpp"
#include "vk/buffers.hpp"
#include "vk/descriptors.hpp"
#include "vk/textures.hpp"
#include "vk/types.hpp"
#include <glm/glm.hpp>
#include <unordered_set>
namespace vblck
{
namespace render
{

struct ChunkData
{
	vk::SSBO vertexData;
	VkDeviceAddress vertexAddr;
	glm::vec3 position;
	uint32_t numVertices;
};
struct ChunkRenderer
{
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline{};
	VkDescriptorSetLayout descriptorSetLayout{};
	VkDescriptorSet descriptorSet{};

	vk::SSBO precomputedVertices{};

	std::unordered_set<ChunkData*> chunks;

	void createDescriptors(vk::DescriptorAllocator* allocator);
	void createBuffers();
	void createPipeline();

	void destroy();

	ChunkData* loadChunk(glm::vec3 position, std::span<ChunkFaceData> data);
	void deleteChunk(ChunkData* chunk);

	void clearData();

	void create(vk::DescriptorAllocator* allocator)
	{
		createBuffers();
		createDescriptors(allocator);
		createPipeline();
	}

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck