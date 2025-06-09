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
}
} // namespace vk
} // namespace vblck