#pragma once
#include "types.hpp"
#include <span>
namespace vblck
{
namespace vk
{
struct DescriptorLayoutBuilder
{

	std::vector<VkDescriptorSetLayoutBinding> bindings;

	void addBinding(uint32_t binding, VkDescriptorType type);
	void clear();
	VkDescriptorSetLayout build(VkDevice device,
								VkShaderStageFlags shaderStages,
								void* pNext = nullptr,
								VkDescriptorSetLayoutCreateFlags flags = 0);
};

struct DescriptorAllocator
{

	struct PoolSizeRatio
	{
		VkDescriptorType type;
		float ratio;
	};

	VkDescriptorPool pool;

	void initPool(VkDevice device, uint32_t maxSets, std::span<PoolSizeRatio> poolRatios);
	void clearDescriptors(VkDevice device);

	inline void freeDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet)
	{
		vkFreeDescriptorSets(device, pool, 1, &descriptorSet);
	}

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetLayout layout);
};

struct DescriptorWritter
{
	static inline void writeBuffer(VkDevice device,
								   VkDescriptorSet descriptorSet,
								   uint32_t bindPoint,
								   VkBuffer buffer,
								   size_t size)
	{
		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = buffer;
		bufferInfo.offset = 0;
		bufferInfo.range = size;

		VkWriteDescriptorSet writeDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.dstBinding = bindPoint;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		writeDescriptorSet.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, 0);
	}
};

} // namespace vk
} // namespace vblck