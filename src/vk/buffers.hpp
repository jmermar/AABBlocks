#pragma once

#include "deletion.hpp"
#include "init.hpp"
#include "types.hpp"
#include <span>
namespace vblck
{
namespace vk
{
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
	void write(const std::span<T> data)
	{
		assert(mappedData && data.buffer && data.allocation);
		assert(sizeof(T) * data.size() <= size);
		std::memcpy(mappedData, data.data(), data.size() * sizeof(T));
	}
};
} // namespace vk
} // namespace vblck
