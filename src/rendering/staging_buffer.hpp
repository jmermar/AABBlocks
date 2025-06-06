#pragma once
#include "vk/deletion.hpp"
#include "vk/types.hpp"
#include <cstring>
#include <span>
#include <stdexcept>
namespace vblck
{
namespace render
{
class StagingBuffer
{
private:
	vk::Buffer data;
	VkDevice device;
	VmaAllocator vma;
	vk::DeletionQueue* deletionQueue{};

	void* mappedData{};

	size_t size;

	void create();

public:
	StagingBuffer(VkDevice device, VmaAllocator vma, vk::DeletionQueue* deletionQueue, size_t size)
		: device(device)
		, vma(vma)
		, deletionQueue(deletionQueue)
		, size(size)
	{
		create();
	}

	~StagingBuffer()
	{
		if(deletionQueue)
		{
			deletionQueue->buffers.push_back(data);
		}
		deletionQueue = 0;
		data = vk::Buffer();
		size = 0;
		mappedData = 0;
	}

	inline VkBuffer get()
	{
		return data.buffer;
	}

	template <typename T>
	void write(const std::span<T> data)
	{
		assert(sizeof(T) * data.size() <= size);
		std::memcpy(mappedData, data.data(), data.size() * sizeof(T));
	}
};
} // namespace render
} // namespace vblck