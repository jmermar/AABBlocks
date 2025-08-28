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
	auto* deferredBuffers =
		&Renderer::get()->deferredBuffers;
	deferredBuffers->depthBuffer.transition(
		cmd,
		VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);
	deferredBuffers->albedo.transition(
		cmd,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	deferredBuffers->normal.transition(
		cmd,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	deferredBuffers->material.transition(
		cmd,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	deferredBuffers->pos.transition(
		cmd,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	chunkRenderer.render(cmd);
}

void WorldRenderer::create()
{
	initDescriptorPool();

	chunkRenderer.create();
}

void WorldRenderer::destroy()
{
	chunkRenderer.destroy();
}

void WorldRenderer::initDescriptorPool() { }
} // namespace render
} // namespace vblck