#include "buffers.hpp"
#include "utils/errors.hpp"

namespace vblck
{
namespace vk
{
void StagingBuffer::create(VkDevice device, VmaAllocator vma, size_t size)
{
	assert(device && vma && size > 0);
	assert(!data.buffer && !data.allocation);
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo allocationInfo{};
	VKTRY(vmaCreateBuffer(
		vma, &bufferInfo, &allocInfo, &data.buffer, &data.allocation, &allocationInfo));
	mappedData = allocationInfo.pMappedData;
	this->size = size;
}

void SSBO::create(VkDevice device, VmaAllocator vma, size_t size)
{
	assert(device && vma && size > 0);
	assert(!data.buffer && !data.allocation);
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
					   VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	this->size = size;

	VmaAllocationInfo allocationInfo{};
	VKTRY(vmaCreateBuffer(
		vma, &bufferInfo, &allocInfo, &data.buffer, &data.allocation, &allocationInfo));
}

void copyBufferToBuffer(VkCommandBuffer cmd,
						VkBuffer src,
						VkBuffer dst,
						uint32_t size,
						uint32_t srcOffset,
						uint32_t dstOffset)
{
	VkBufferCopy2 region = {.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2};
	region.dstOffset = dstOffset;
	region.srcOffset = srcOffset;
	region.size = size;

	VkCopyBufferInfo2 info{.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2};
	info.srcBuffer = src;
	info.dstBuffer = dst;
	info.regionCount = 1;
	info.pRegions = &region;

	vkCmdCopyBuffer2(cmd, &info);
}
} // namespace vk
} // namespace vblck