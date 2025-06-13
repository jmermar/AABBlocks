#pragma once

#include "deletion.hpp"
#include "init.hpp"
#include "types.hpp"
#include <span>
namespace vblck
{
namespace vk
{
void copyBufferToBuffer(VkCommandBuffer cmd,
						VkBuffer src,
						VkBuffer dst,
						uint32_t size,
						uint32_t srcOffset,
						uint32_t dstOffset);

inline VkDeviceSize align(VkDeviceSize value, VkDeviceSize alignment)
{
	return (value + alignment - 1) & ~(alignment - 1);
}

struct StagingBuffer
{
	Buffer data{};
	void* mappedData{};
	size_t size{};
	void create(VkDevice device, VmaAllocator vma, size_t size);
	inline void destroy(DeletionQueue* deletion)
	{
		assert(data.allocation && data.buffer && size > 0);
		assert(deletion);
		deletion->buffers.push_back(data);
		data.buffer = 0;
		data.allocation = 0;
		size = 0;
		mappedData = 0;
	}
	template <typename T>
	void write(const std::span<T> writeData)
	{
		assert(mappedData && data.buffer && data.allocation);
		assert(sizeof(T) * writeData.size() <= size);
		std::memcpy(mappedData, writeData.data(), writeData.size() * sizeof(T));
	}
};

struct SSBO
{
	Buffer data{};
	size_t size{};
	void create(VkDevice device, VmaAllocator vma, size_t size);

	inline void destroy(DeletionQueue* deletion)
	{
		assert(data.allocation && data.buffer && size > 0);
		assert(deletion);
		deletion->buffers.push_back(data);
		data.buffer = 0;
		data.allocation = 0;
		size = 0;
	}

	inline void barrier(VkCommandBuffer cmd,
						VkPipelineStageFlags srcStage,
						VkPipelineStageFlags dstStage,
						VkAccessFlags srcAccess,
						VkAccessFlags dstAccess)
	{
		VkBufferMemoryBarrier bufferBarrier = {.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
											   .pNext = nullptr,
											   .srcAccessMask = srcAccess,
											   .dstAccessMask = dstAccess,
											   .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
											   .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
											   .buffer = data.buffer,
											   .offset = 0,
											   .size = VK_WHOLE_SIZE};
		vkCmdPipelineBarrier(cmd,
							 srcStage,
							 dstStage,
							 0,
							 0,
							 nullptr,
							 1,
							 &bufferBarrier,
							 0,
							 nullptr // image barriers
		);
	}
};

} // namespace vk
} // namespace vblck
