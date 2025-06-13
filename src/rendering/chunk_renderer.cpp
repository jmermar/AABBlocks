#include "chunk_renderer.hpp"
#include "renderer.hpp"
#include "utils/errors.hpp"
#include "utils/files.hpp"
#include "vk/graphics_pipeline.hpp"
namespace vblck
{
namespace render
{

struct Vertex
{
	glm::vec3 position;
	float pad;
	glm::vec3 normal;
	float pad2;
	glm::vec2 uv;
	float pad3[2];
};

struct ChunkDataBuffer
{
	VkDeviceAddress chunkFaces;
	float pad[2];
	glm::vec3 position;
	float pad2[1];
};

struct Face
{
	Vertex vertices[6];
};

constexpr Face pushFace(glm::vec3 top, glm::vec3 normal, glm::vec3 right, glm::vec3 down)
{

	Face pushFace;
	pushFace.vertices[2] = {top, 0.f, normal, 0.f, glm::vec2(0, 0), {0.f, 0.f}};
	pushFace.vertices[1] = {top + down, 0.f, normal, 0.f, glm::vec2(0, 1), {0.f, 0.f}};
	pushFace.vertices[0] = {top + down + right, 0.f, normal, 0.f, glm::vec2(1, 1), {0.f, 0.f}};
	pushFace.vertices[5] = {top, 0.f, normal, 0.f, glm::vec2(0, 0), {0.f, 0.f}};
	pushFace.vertices[4] = {top + down + right, 0, normal, 0.f, glm::vec2(1, 1), {0.f, 0.f}};
	pushFace.vertices[3] = {top + right, 0.f, normal, 0.f, glm::vec2(1, 0), {0.f, 0.f}};
	return pushFace;
};

Face precomputedVerticesData[6] = {
	//Front
	pushFace({0, 1, 1}, {0, 0, -1}, {1, 0, 0}, {0, -1, 0}),
	//Back
	pushFace({1, 1, 0}, {0, 0, 1}, {-1, 0, 0}, {0, -1, 0}),
	// Right
	pushFace({1, 1, 1}, {1, 0, 0}, {0, 0, -1}, {0, -1, 0}),
	// Left
	pushFace({0, 1, 0}, {-1, 0, 0}, {0, 0, 1}, {0, -1, 0}),
	//Top
	pushFace({0, 1, 0}, {0, 1, 0}, {1, 0, 0}, {0, 0, 1}),
	//Bottom
	pushFace({0, 0, 1}, {0, -1, 0}, {1, 0, 0}, {0, 0, -1}),

};

void ChunkRenderer::createDescriptors(vk::DescriptorAllocator* allocator)
{
	auto* render = Renderer::get();

	vk::DescriptorLayoutBuilder layoutBuilder;
	layoutBuilder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	layoutBuilder.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	layoutBuilder.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	descriptorSetLayout =
		layoutBuilder.build(Renderer::get()->device, VK_SHADER_STAGE_ALL_GRAPHICS);

	descriptorSet = allocator->allocate(Renderer::get()->device, descriptorSetLayout);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = render->textureAtlas.imageView;
	imageInfo.sampler = render->textureAtlas.sampler;

	VkDescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = precomputedVertices.data.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = precomputedVertices.size;

	VkWriteDescriptorSet writeDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(render->device, 1, &writeDescriptorSet, 0, 0);
	writeDescriptorSet.dstBinding = 1;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet.pBufferInfo = &bufferInfo;
	writeDescriptorSet.pImageInfo = 0;
	vkUpdateDescriptorSets(render->device, 1, &writeDescriptorSet, 0, 0);

	bufferInfo.buffer = chunksDataBuffer.data.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = chunksDataBuffer.size;

	writeDescriptorSet.dstBinding = 2;
	writeDescriptorSet.dstArrayElement = 0;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSet.pBufferInfo = &bufferInfo;
	writeDescriptorSet.pImageInfo = 0;
	vkUpdateDescriptorSets(render->device, 1, &writeDescriptorSet, 0, 0);
}
void ChunkRenderer::createBuffers()
{
	auto* render = Renderer::get();

	// PrecomputedVertices

	precomputedVertices.create(render->device, render->vma, sizeof(Face) * 6);
	vk::StagingBuffer precomputedStaging;
	precomputedStaging.create(render->device, render->vma, sizeof(Face) * 6);
	precomputedStaging.write(std::span<Face>(precomputedVerticesData));
	render->bufferWritter.writeToSSBO(precomputedStaging.data.buffer, &precomputedVertices);
	precomputedStaging.destroy(&render->frameDeletionQueue);

	// ChunkDataBuffer

	chunksDataBuffer.create(render->device, render->vma, sizeof(ChunkDataBuffer) * 64 * 64 * 64);

	//ChunkDrawCommands
	chunkDrawCommands.create(
		render->device, render->vma, sizeof(VkDrawIndirectCommand) * 64 * 64 * 64);
}
void ChunkRenderer::createPipeline()
{
	auto* render = Renderer::get();

	// PipelineLayout
	VkPipelineLayoutCreateInfo layoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

	VkDescriptorSetLayout descriptors[2] = {Renderer::get()->renderData.globalDescriptorLayout,
											descriptorSetLayout};

	layoutInfo.setLayoutCount = 2;
	layoutInfo.pSetLayouts = descriptors;

	VKTRY(vkCreatePipelineLayout(render->device, &layoutInfo, nullptr, &pipelineLayout));

	std::vector<uint8_t> vertexShaderData = loadBinaryFile("res/shaders/test.vert.spv");
	std::vector<uint8_t> fragmentShaderData = loadBinaryFile("res/shaders/test.frag.spv");

	vk::GraphicsPipelineBuilder pipelineBuilder;

	auto vertexShader = vk::createShaderModule(render->device, vertexShaderData);
	auto fragmentShader = vk::createShaderModule(render->device, fragmentShaderData);

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout = pipelineLayout;
	//connecting the vertex and pixel shaders to the pipeline
	pipelineBuilder.set_shaders(vertexShader, fragmentShader);
	//it will draw triangles
	pipelineBuilder.set_input_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	//filled triangles
	pipelineBuilder.set_polygon_mode(VK_POLYGON_MODE_FILL);
	//no backface culling
	pipelineBuilder.set_cull_mode(VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	//no multisampling
	pipelineBuilder.set_multisampling_none();
	//no blending
	pipelineBuilder.disable_blending();
	//no depth testing
	pipelineBuilder.set_depth_format(VK_FORMAT_D32_SFLOAT);
	pipelineBuilder.enable_depthtest();

	//connect the image format we will draw into, from draw image
	pipelineBuilder.set_color_attachment_format(VK_FORMAT_R8G8B8A8_UNORM);

	//finally build the pipeline
	pipeline = pipelineBuilder.buildPipeline(render->device);

	//clean structures
	vkDestroyShaderModule(render->device, fragmentShader, nullptr);
	vkDestroyShaderModule(render->device, vertexShader, nullptr);
}

void ChunkRenderer::regenerateChunks()
{
	auto* render = Renderer::get();
	auto numChunks = chunks.size();
	std::vector<ChunkDataBuffer> data(numChunks);
	int i = 0;
	for(auto* chunk : chunks)
	{
		data[i].chunkFaces = chunk->vertexAddr;
		data[i].position = chunk->position;
		i++;
	}

	vk::StagingBuffer staging;
	staging.create(render->device, render->vma, numChunks * sizeof(ChunkDataBuffer));
	staging.write<ChunkDataBuffer>(data);
	render->bufferWritter.writeToSSBO(
		staging.data.buffer, &chunksDataBuffer, numChunks * sizeof(ChunkDataBuffer));

	std::vector<VkDrawIndirectCommand> drawCommandData(numChunks);
	i = 0;
	for(auto* chunk : chunks)
	{
		drawCommandData[i].firstInstance = i;
		drawCommandData[i].instanceCount = 1;
		drawCommandData[i].firstVertex = 0;
		drawCommandData[i].vertexCount = chunk->numVertices;
		i++;
	}

	vk::StagingBuffer drawStaging;
	drawStaging.create(render->device, render->vma, numChunks * sizeof(VkDrawIndirectCommand));
	drawStaging.write<VkDrawIndirectCommand>(drawCommandData);
	render->bufferWritter.writeToSSBO(
		drawStaging.data.buffer, &chunkDrawCommands, numChunks * sizeof(VkDrawIndirectCommand));

	staging.destroy(&render->frameDeletionQueue);
	drawStaging.destroy(&render->frameDeletionQueue);
}

void ChunkRenderer::destroy()
{
	auto* render = Renderer::get();

	vk::DeletionQueue deletion;

	vkDestroyPipeline(render->device, pipeline, nullptr);
	vkDestroyPipelineLayout(render->device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(render->device, descriptorSetLayout, nullptr);

	for(auto* chunk : chunks)
	{
		chunk->vertexData.destroy(&deletion);
		delete chunk;
	}

	precomputedVertices.destroy(&deletion);
	chunkDrawCommands.destroy(&deletion);
	chunksDataBuffer.destroy(&deletion);

	deletion.deleteQueue(render->device, render->vma);
}

ChunkData* ChunkRenderer ::loadChunk(glm::vec3 position, std::span<ChunkFaceData> data)
{
	auto* chunk = new ChunkData;
	auto* render = Renderer::get();

	chunk->position = position;
	chunk->vertexData.create(render->device, render->vma, sizeof(ChunkFaceData) * data.size());
	vk::StagingBuffer staging;
	staging.create(render->device, render->vma, sizeof(ChunkFaceData) * data.size());
	staging.write(data);
	render->bufferWritter.writeToSSBO(staging.data.buffer, &chunk->vertexData);
	staging.destroy(&render->frameDeletionQueue);
	chunk->numVertices = data.size() * 6;
	VkBufferDeviceAddressInfo addrInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
	addrInfo.buffer = chunk->vertexData.data.buffer;
	chunk->vertexAddr = vkGetBufferDeviceAddress(render->device, &addrInfo);

	chunks.insert(chunk);
	return chunk;
}

void ChunkRenderer::deleteChunk(ChunkData* chunk)
{
	if(chunks.contains(chunk))
	{
		chunk->vertexData.destroy(&Renderer::get()->frameDeletionQueue);
		chunks.erase(chunk);
		delete chunk;
	}
}

void ChunkRenderer::clearData()
{
	for(auto* chunk : chunks)
	{
		chunk->vertexData.destroy(&Renderer::get()->frameDeletionQueue);
		delete chunk;
	}
	chunks.clear();
}

void ChunkRenderer::render(VkCommandBuffer cmd)
{
	auto render = Renderer::get();
	render->textureAtlas.transition(
		cmd, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	auto* backbuffer = render->getBackbuffer();
	auto colorAttachment = vk::Init::attachementInfo(
		backbuffer->imageView, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkClearValue clearValue{};
	clearValue.depthStencil.depth = 1;
	auto depthAttachment = vk::Init::attachementInfo(
		render->depthBuffer.imageView, &clearValue, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

	VkRenderingInfo renderInfo = {.sType = VK_STRUCTURE_TYPE_RENDERING_INFO};

	renderInfo.renderArea = {{0, 0}, backbuffer->extent};
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachment;
	renderInfo.pDepthAttachment = &depthAttachment;
	renderInfo.pStencilAttachment = 0;
	renderInfo.layerCount = 1;
	vkCmdBeginRendering(cmd, &renderInfo);

	auto globalDescriptor = Renderer::get()->renderData.getGlobalDescriptor();

	vkCmdBindDescriptorSets(
		cmd, (VkPipelineBindPoint)0, pipelineLayout, 0, 1, &globalDescriptor, 0, 0);
	vkCmdBindDescriptorSets(
		cmd, (VkPipelineBindPoint)0, pipelineLayout, 1, 1, &descriptorSet, 0, 0);

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

	vkCmdDrawIndirect(
		cmd, chunkDrawCommands.data.buffer, 0, chunks.size(), sizeof(VkDrawIndirectCommand));

	vkCmdEndRendering(cmd);
}
} // namespace render
} // namespace vblck