#pragma once
#include "types.hpp"
#include <span>
namespace vblck
{
namespace vk
{
struct DescriptorLayoutBuilder
{

	std::vector<VkDescriptorSetLayoutBinding>
		bindings;

	void addBinding(uint32_t binding,
					VkDescriptorType type);
	void clear();
	VkDescriptorSetLayout
	build(VkDevice device,
		  VkShaderStageFlags shaderStages,
		  void* pNext = nullptr,
		  VkDescriptorSetLayoutCreateFlags flags =
			  0);
};

struct DescriptorAllocator
{
public:
	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	void
	create(VkDevice device,
		   uint32_t initialSets,
		   std::span<PoolSizeRatio> poolRatios);
	void clear(VkDevice device);
	void destroy(VkDevice device);

	VkDescriptorSet
	allocate(VkDevice device,
			 VkDescriptorSetLayout layout,
			 void* pNext = nullptr);

	VkDescriptorPool getPool(VkDevice device);
	VkDescriptorPool createPool(
		VkDevice device,
		uint32_t setCount,
		std::span<PoolSizeRatio> poolRatios);

	std::vector<PoolSizeRatio> ratios;
	std::vector<VkDescriptorPool> fullPools;
	std::vector<VkDescriptorPool> readyPools;
	uint32_t setsPerPool;
};

struct DescriptorWriter
{
	std::vector<VkDescriptorImageInfo> imageInfos;
	std::vector<VkDescriptorBufferInfo>
		bufferInfos;
	std::vector<VkWriteDescriptorSet> writes;

	void startWrites(size_t num)
	{
		clear();
		imageInfos.reserve(num);
		bufferInfos.reserve(num);
		writes.reserve(num);
	}

	void writeImage(
		int binding,
		VkImageView image,
		VkSampler sampler,
		VkImageLayout layout =
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VkDescriptorType type =
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	void writeBuffer(
		int binding,
		VkBuffer buffer,
		size_t size,
		size_t offset,
		VkDescriptorType type =
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);

	void clear();
	void write(VkDevice device,
			   VkDescriptorSet set);
};

} // namespace vk
} // namespace vblck