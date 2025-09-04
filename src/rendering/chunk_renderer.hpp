#pragma once
#include "mapped_buffer.hpp"
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

struct ChunkDrawCommandsDispatcher
{
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet descriptorSet;

	vk::SSBO dispatchBuffer{};

	void createBuffers();
	void createDescriptors();
	void createPipeline();

	void destroy();

	void create()
	{
		createBuffers();
		createDescriptors();
		createPipeline();
	}

	void dispatch(VkCommandBuffer cmd,
				  uint32_t nChunks);
};

struct ChunkRenderer
{
	VkPipelineLayout pipelineLayout{};
	VkPipeline pipeline{};
	VkDescriptorSetLayout descriptorSetLayout{};
	VkDescriptorSet descriptorSet{};

	vk::SSBO precomputedVertices{};
	vk::SSBO chunksDataBuffer{};
	vk::SSBO chunkDrawCommands{};

	std::unordered_set<ChunkData*> chunks;

	ChunkDrawCommandsDispatcher dispatcher;

	void createDescriptors();
	void createBuffers();
	void createPipeline();

	void regenerateChunks();

	void destroy();

	ChunkData*
	loadChunk(glm::vec3 position,
			  std::span<ChunkFaceData> data);
	void deleteChunk(ChunkData* chunk);

	void clearData();

	void create()
	{
		createBuffers();
		createDescriptors();
		createPipeline();
		dispatcher.create();
	}

	void render(VkCommandBuffer cmd);
	void renderShadowMaps(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck