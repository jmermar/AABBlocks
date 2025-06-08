#include "staging_buffer.hpp"
#include "utils/logger.hpp"
using namespace vblck::render;

void StagingBuffer::create()
{
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
}