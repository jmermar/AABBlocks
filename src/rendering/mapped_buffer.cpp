#include "mapped_buffer.hpp"
#include "renderer.hpp"
namespace vblck
{
namespace render
{
void MappedBuffer::create(size_t size)
{
	auto* render = Renderer::get();
	this->alignedSize = (size + render->props.limits.minUniformBufferOffsetAlignment - 1) &
						~(render->props.limits.minUniformBufferOffsetAlignment - 1);
	this->size = size;
	totalSize = alignedSize * FRAME_OVERLAP;
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = totalSize;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

	VmaAllocationInfo vmaAllocInfo{};
	vmaCreateBuffer(
		render->vma, &bufferInfo, &allocInfo, &buffer.buffer, &buffer.allocation, &vmaAllocInfo);

	data = vmaAllocInfo.pMappedData;
}

void MappedBuffer::destroy()
{
	Renderer::get()->frameDeletionQueue.buffers.push_back(buffer);
}
void* MappedBuffer::getData()
{
	return ((unsigned char*)data + alignedSize * Renderer::get()->getFrameIndex());
}
VkDeviceSize MappedBuffer::getBaseAddr(uint32_t frameNumber)
{
	return alignedSize * frameNumber;
}
} // namespace render
} // namespace vblck