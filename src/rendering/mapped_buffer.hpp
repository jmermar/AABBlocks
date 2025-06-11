#pragma once
#include "vk/types.hpp"

namespace vblck
{
namespace render
{
struct MappedBuffer
{
	vk::Buffer buffer{};
	size_t size{};
	size_t alignedSize{};
	size_t totalSize{};
	void* data{};

	void create(size_t size);
	void destroy();

	void* getData();
	VkDeviceSize getBaseAddr(uint32_t frameNumber);
};
} // namespace render
} // namespace vblck