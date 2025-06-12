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

	chunkRenderer.create(&descriptorAllocator);
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