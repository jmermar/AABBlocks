#include "world_renderer.hpp"
#include "renderer.hpp"
#include "utils/files.hpp"
#include "vk/descriptors.hpp"
#include "vk/graphics_pipeline.hpp"
#include <glm/glm.hpp>
namespace vblck
{
namespace render
{

struct VertexData
{
	glm::vec2 pos;
	glm::vec2 color;
};

void WorldRenderer::render(VkCommandBuffer cmd)
{
	chunkRenderer.render(cmd);
}

void WorldRenderer::create()
{
	initDescriptorPool();

	std::vector<ChunkFaceData> chunkData;

	chunkData.resize(6);
	chunkData[0].face = 0;
	chunkData[0].position = glm::vec3(0);
	chunkData[0].textureId = 49;

	chunkData[1].face = 1;
	chunkData[1].position = glm::vec3(0);
	chunkData[1].textureId = 49;

	chunkData[2].face = 2;
	chunkData[2].position = glm::vec3(0);
	chunkData[2].textureId = 49;

	chunkData[3].face = 3;
	chunkData[3].position = glm::vec3(0);
	chunkData[3].textureId = 49;

	chunkData[4].face = 4;
	chunkData[4].position = glm::vec3(0);
	chunkData[4].textureId = 48;

	chunkData[5].face = 5;
	chunkData[5].position = glm::vec3(0);
	chunkData[5].textureId = 0;

	chunkRenderer.create(&descriptorAllocator);
	chunkRenderer.loadChunk(glm::vec3(0), chunkData);
}

void WorldRenderer::initDescriptorPool()
{
	auto* render = Renderer::get();
	std::vector<vk::DescriptorAllocator::PoolSizeRatio> ratios;
	ratios.resize(3);
	ratios[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	ratios[0].ratio = 1;
	ratios[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	ratios[1].ratio = 1;
	ratios[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	ratios[2].ratio = 1;

	descriptorAllocator.initPool(render->device, 8, ratios);
}
} // namespace render
} // namespace vblck