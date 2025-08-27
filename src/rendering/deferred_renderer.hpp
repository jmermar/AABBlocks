#pragma once
#include "vk/buffers.hpp"
#include "vk/descriptors.hpp"
#include "vk/textures.hpp"
#include "vk/types.hpp"
namespace vblck
{
namespace render
{
struct DeferredRenderer
{
	VkPipelineLayout pipelineLayout{};
	VkPipeline pipeline{};
	VkDescriptorSetLayout descriptorSetLayout{};
	VkDescriptorSet descriptorSet{};
	vk::SSBO vertices{};

	void create()
	{
		_createBuffers();
		_createDescriptors();
		_createPipeline();
	}
	void destroy();

	void render(VkCommandBuffer cmd);

	void writeDescriptorSets();

	void _createDescriptors();
	void _createPipeline();
	void _createBuffers();
};
} // namespace render
} // namespace vblck