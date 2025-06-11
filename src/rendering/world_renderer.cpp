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

void WorldRenderer::initPipelineLayout()
{
	VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	auto globalDescriptorSetLayout = Renderer::get()->renderData.globalDescriptorLayout;

	layoutInfo.setLayoutCount = 1;
	layoutInfo.pSetLayouts = &globalDescriptorSetLayout;
	layoutInfo.pushConstantRangeCount = 0;

	VKTRY(vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout));
}
void WorldRenderer::initPipeline()
{

	std::vector<uint8_t> vertexShaderData = loadBinaryFile("res/shaders/test.vert.spv");
	std::vector<uint8_t> fragmentShaderData = loadBinaryFile("res/shaders/test.frag.spv");

	vk::GraphicsPipelineBuilder pipelineBuilder;

	auto vertexShader = vk::createShaderModule(device, vertexShaderData);
	auto fragmentShader = vk::createShaderModule(device, fragmentShaderData);

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout = pipelineLayout;
	//connecting the vertex and pixel shaders to the pipeline
	pipelineBuilder.set_shaders(vertexShader, fragmentShader);
	//it will draw triangles
	pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//filled triangles
	pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	//no backface culling
	pipelineBuilder.set_cull_mode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
	//no multisampling
	pipelineBuilder.set_multisampling_none();
	//no blending
	pipelineBuilder.disable_blending();
	//no depth testing
	pipelineBuilder.disable_depthtest();

	//connect the image format we will draw into, from draw image
	pipelineBuilder.set_color_attachment_format(VK_FORMAT_R8G8B8A8_UNORM);
	pipelineBuilder.set_depth_format(VK_FORMAT_UNDEFINED);

	//finally build the pipeline
	pipeline = pipelineBuilder.buildPipeline(device);

	//clean structures
	vkDestroyShaderModule(device, fragmentShader, nullptr);
	vkDestroyShaderModule(device, vertexShader, nullptr);
}

void WorldRenderer::destroy()
{
	vkDestroyPipeline(device, pipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
}
void WorldRenderer::render(VkCommandBuffer cmd)
{
	auto* backbuffer = Renderer::get()->getBackbuffer();
	auto colorAttachment = vk::Init::attachementInfo(
		backbuffer->imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO};

	renderInfo.renderArea = {{0, 0}, backbuffer->extent};
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachment;
	renderInfo.pDepthAttachment = 0;
	renderInfo.pStencilAttachment = 0;
	renderInfo.layerCount = 1;
	vkCmdBeginRendering(cmd, &renderInfo);

	vkCmdBindDescriptorSets(cmd,
							(VkPipelineBindPoint)0,
							pipelineLayout,
							0,
							1,
							&Renderer::get()->renderData.globalDescriptor,
							0,
							0);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = backbuffer->extent.width;
	viewport.height = backbuffer->extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = backbuffer->extent.width;
	scissor.extent.height = backbuffer->extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkCmdDraw(cmd, 3, 1, 0, 0);

	vkCmdEndRendering(cmd);
}
} // namespace render
} // namespace vblck