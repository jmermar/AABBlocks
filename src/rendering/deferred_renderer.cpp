#include "deferred_renderer.hpp"
#include "renderer.hpp"
#include "utils/files.hpp"
#include "vk/graphics_pipeline.hpp"
namespace vblck
{
namespace render
{

glm::vec4 verticesData[6] = {
	{0, 0, 0, 0},
	{1, 0, 0, 0},
	{1, 1, 0, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{0, 1, 0, 0},
};
void DeferredRenderer::destroy()
{
	auto* render = Renderer::get();

	vk::DeletionQueue deletion;
	vertices.destroy(&deletion);
	deletion.deleteQueue(render->device,
						 render->vma);

	vkDestroyPipeline(
		render->device, pipeline, nullptr);
	vkDestroyPipelineLayout(
		render->device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(
		render->device,
		descriptorSetLayout,
		nullptr);
}
void DeferredRenderer::_createDescriptors()
{
	auto* render = Renderer::get();
	auto* deferredBuffers =
		&render->deferredBuffers;

	vk::DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(
		0,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(
		1,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(
		2,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(
		3,
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(
		4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	descriptorSetLayout = layoutBuilder.build(
		Renderer::get()->device,
		VK_SHADER_STAGE_ALL_GRAPHICS);

	descriptorSet = render->allocateDescriptor(
		descriptorSetLayout);

	vk::DescriptorWriter writer;
	writer.startWrites(5);
	writer.writeImage(
		0,
		deferredBuffers->albedo.imageView,
		deferredBuffers->albedo.sampler);
	writer.writeImage(
		1,
		deferredBuffers->normal.imageView,
		deferredBuffers->normal.sampler);
	writer.writeImage(
		2,
		deferredBuffers->pos.imageView,
		deferredBuffers->pos.sampler);
	writer.writeImage(
		3,
		deferredBuffers->material.imageView,
		deferredBuffers->material.sampler);
	writer.writeBuffer(4,
					   vertices.data.buffer,
					   vertices.size,
					   0);

	writer.write(render->device, descriptorSet);
}

void DeferredRenderer::render(VkCommandBuffer cmd)
{
	auto render = Renderer::get();
	VkRenderingAttachmentInfo colorAttachment;
	colorAttachment = vk::Init::attachementInfo(
		render->backbuffer.imageView,
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = {
		.sType =
			VK_STRUCTURE_TYPE_RENDERING_INFO};

	renderInfo.renderArea = {
		{0, 0}, render->screenExtent};
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments =
		&colorAttachment;
	renderInfo.pDepthAttachment = 0;
	renderInfo.pStencilAttachment = 0;
	renderInfo.layerCount = 1;
	vkCmdBeginRendering(cmd, &renderInfo);

	auto globalDescriptor =
		Renderer::get()
			->renderData.getGlobalDescriptor();

	vkCmdBindDescriptorSets(
		cmd,
		(VkPipelineBindPoint)0,
		pipelineLayout,
		0,
		1,
		&globalDescriptor,
		0,
		0);
	vkCmdBindDescriptorSets(
		cmd,
		(VkPipelineBindPoint)0,
		pipelineLayout,
		1,
		1,
		&descriptorSet,
		0,
		0);

	vkCmdBindPipeline(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline);

	VkViewport viewport = {};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width =
		render->backbuffer.extent.width;
	viewport.height =
		render->backbuffer.extent.height;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	vkCmdSetViewport(cmd, 0, 1, &viewport);

	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width =
		render->backbuffer.extent.width;
	scissor.extent.height =
		render->backbuffer.extent.height;

	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkCmdDraw(cmd, 6, 1, 0, 0);
	vkCmdEndRendering(cmd);
}
void DeferredRenderer::_createPipeline()
{
	auto* render = Renderer::get();

	// PipelineLayout
	VkPipelineLayoutCreateInfo layoutInfo{
		.sType =
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	VkDescriptorSetLayout descriptors[2] = {
		Renderer::get()
			->renderData.globalDescriptorLayout,
		descriptorSetLayout};

	layoutInfo.setLayoutCount = 2;
	layoutInfo.pSetLayouts = descriptors;

	VKTRY(
		vkCreatePipelineLayout(render->device,
							   &layoutInfo,
							   nullptr,
							   &pipelineLayout));

	std::vector<uint8_t> vertexShaderData =
		loadBinaryFile(
			"res/shaders/deferred.vert.spv");
	std::vector<uint8_t> fragmentShaderData =
		loadBinaryFile(
			"res/shaders/deferred.frag.spv");

	vk::GraphicsPipelineBuilder pipelineBuilder;

	auto vertexShader = vk::createShaderModule(
		render->device, vertexShaderData);
	auto fragmentShader = vk::createShaderModule(
		render->device, fragmentShaderData);

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout =
		pipelineLayout;
	//connecting the vertex and pixel shaders to the pipeline
	pipelineBuilder.set_shaders(vertexShader,
								fragmentShader);
	//it will draw triangles
	pipelineBuilder.set_input_topology(
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//filled triangles
	pipelineBuilder.set_polygon_mode(
		VK_POLYGON_MODE_FILL);
	//no backface culling
	pipelineBuilder.set_cull_mode(
		VK_CULL_MODE_NONE,
		VK_FRONT_FACE_CLOCKWISE);
	//no multisampling
	pipelineBuilder.set_multisampling_none();
	//no blending
	pipelineBuilder.disable_blending();
	//no depth testing
	pipelineBuilder.set_depth_format(
		VK_FORMAT_D32_SFLOAT);
	pipelineBuilder.enable_depthtest();

	VkFormat colorFormats[1] = {
		VK_FORMAT_R8G8B8A8_UNORM};

	//connect the image format we will draw into, from draw image
	pipelineBuilder.set_color_attachment_format(
		std::span(colorFormats));

	//finally build the pipeline
	pipeline = pipelineBuilder.buildPipeline(
		render->device);

	//clean structures
	vkDestroyShaderModule(
		render->device, fragmentShader, nullptr);
	vkDestroyShaderModule(
		render->device, vertexShader, nullptr);
}

void DeferredRenderer::_createBuffers()
{
	auto* render = Renderer::get();

	// vertices

	vertices.create(render->device,
					render->vma,
					sizeof(float) * 6 * 4);
	vk::StagingBuffer precomputedStaging;
	precomputedStaging.create(render->device,
							  render->vma,
							  sizeof(float) * 6 *
								  4);
	precomputedStaging.write(
		std::span<glm::vec4>(verticesData));
	render->bufferWritter.writeToSSBO(
		precomputedStaging.data.buffer,
		&vertices);
	precomputedStaging.destroy(
		&render->frameDeletionQueue);
}

} // namespace render
} // namespace vblck