#include "descriptors.hpp"
#include "utils/errors.hpp"

namespace vblck
{
namespace vk
{
void DescriptorLayoutBuilder::addBinding(
	uint32_t binding, VkDescriptorType type)
{
	VkDescriptorSetLayoutBinding newbind{};
	newbind.binding = binding;
	newbind.descriptorCount = 1;
	newbind.descriptorType = type;

	bindings.push_back(newbind);
}

void DescriptorLayoutBuilder::clear()
{
	bindings.clear();
}

VkDescriptorSetLayout
DescriptorLayoutBuilder::build(
	VkDevice device,
	VkShaderStageFlags shaderStages,
	void* pNext,
	VkDescriptorSetLayoutCreateFlags flags)
{
	for(auto& b : bindings)
	{
		b.stageFlags |= shaderStages;
	}

	VkDescriptorSetLayoutCreateInfo info = {
		.sType =
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	info.pNext = pNext;

	info.pBindings = bindings.data();
	info.bindingCount = (uint32_t)bindings.size();
	info.flags = flags;

	VkDescriptorSetLayout set;
	VKTRY(vkCreateDescriptorSetLayout(
		device, &info, nullptr, &set));

	return set;
}

void DescriptorAllocator::create(
	VkDevice device,
	uint32_t maxSets,
	std::span<PoolSizeRatio> poolRatios)
{
	ratios.clear();

	for(auto r : poolRatios)
	{
		ratios.push_back(r);
	}

	VkDescriptorPool newPool =
		createPool(device, maxSets, poolRatios);

	setsPerPool =
		maxSets * 1.5; //grow it next allocation

	readyPools.push_back(newPool);
}

void DescriptorAllocator::clear(VkDevice device)
{
	for(auto p : readyPools)
	{
		vkResetDescriptorPool(device, p, 0);
	}
	for(auto p : fullPools)
	{
		vkResetDescriptorPool(device, p, 0);
		readyPools.push_back(p);
	}
	fullPools.clear();
}

void DescriptorAllocator::destroy(VkDevice device)
{
	for(auto p : readyPools)
	{
		vkDestroyDescriptorPool(
			device, p, nullptr);
	}
	readyPools.clear();
	for(auto p : fullPools)
	{
		vkDestroyDescriptorPool(
			device, p, nullptr);
	}
	fullPools.clear();
}

VkDescriptorPool
DescriptorAllocator::getPool(VkDevice device)
{
	VkDescriptorPool newPool;
	if(readyPools.size() != 0)
	{
		newPool = readyPools.back();
		readyPools.pop_back();
	}
	else
	{
		//need to create a new pool
		newPool = createPool(
			device, setsPerPool, ratios);

		setsPerPool = setsPerPool * 1.5;
		if(setsPerPool > 4092)
		{
			setsPerPool = 4092;
		}
	}

	return newPool;
}

VkDescriptorSet DescriptorAllocator::allocate(
	VkDevice device,
	VkDescriptorSetLayout layout,
	void* pNext)
{
	//get or create a pool to allocate from
	VkDescriptorPool poolToUse = getPool(device);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = pNext;
	allocInfo.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = poolToUse;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkDescriptorSet ds;
	VkResult result = vkAllocateDescriptorSets(
		device, &allocInfo, &ds);

	//allocation failed. Try again
	if(result == VK_ERROR_OUT_OF_POOL_MEMORY ||
	   result == VK_ERROR_FRAGMENTED_POOL)
	{

		fullPools.push_back(poolToUse);

		poolToUse = getPool(device);
		allocInfo.descriptorPool = poolToUse;

		VKTRY(vkAllocateDescriptorSets(
			device, &allocInfo, &ds));
	}

	readyPools.push_back(poolToUse);
	return ds;
}

VkDescriptorPool DescriptorAllocator::createPool(
	VkDevice device,
	uint32_t setCount,
	std::span<PoolSizeRatio> poolRatios)
{
	std::vector<VkDescriptorPoolSize> poolSizes;
	for(PoolSizeRatio ratio : poolRatios)
	{
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = ratio.type,
			.descriptorCount = uint32_t(
				ratio.ratio * setCount)});
	}

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType =
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = 0;
	pool_info.maxSets = setCount;
	pool_info.poolSizeCount =
		(uint32_t)poolSizes.size();
	pool_info.pPoolSizes = poolSizes.data();

	VkDescriptorPool newPool;
	vkCreateDescriptorPool(
		device, &pool_info, nullptr, &newPool);
	return newPool;
}

void DescriptorWriter::writeBuffer(
	int binding,
	VkBuffer buffer,
	size_t size,
	size_t offset,
	VkDescriptorType type)
{
	bufferInfos.push_back({.buffer = buffer,
						   .offset = offset,
						   .range = size});

	VkWriteDescriptorSet write = {
		.sType =
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

	write.dstBinding = binding;
	write.dstSet =
		VK_NULL_HANDLE; //left empty for now until we need to write it
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pBufferInfo = &bufferInfos.back();

	writes.push_back(write);
}

void DescriptorWriter::writeImage(
	int binding,
	VkImageView image,
	VkSampler sampler,
	VkImageLayout layout,
	VkDescriptorType type)
{
	imageInfos.push_back({.sampler = sampler,
						  .imageView = image,
						  .imageLayout = layout});

	VkWriteDescriptorSet write = {
		.sType =
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};

	write.dstBinding = binding;
	write.dstSet =
		VK_NULL_HANDLE; //left empty for now until we need to write it
	write.descriptorCount = 1;
	write.descriptorType = type;
	write.pImageInfo = &imageInfos.back();

	writes.push_back(write);
}

void DescriptorWriter::clear()
{
	imageInfos.clear();
	writes.clear();
	bufferInfos.clear();
}

void DescriptorWriter::write(VkDevice device,
							 VkDescriptorSet set)
{
	for(VkWriteDescriptorSet& write : writes)
	{
		write.dstSet = set;
	}

	vkUpdateDescriptorSets(
		device,
		(uint32_t)writes.size(),
		writes.data(),
		0,
		nullptr);
}

} // namespace vk
} // namespace vblck