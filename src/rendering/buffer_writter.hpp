#pragma once

#include "utils/logger.hpp"
#include "vk/buffers.hpp"
#include "vk/init.hpp"
#include "vk/textures.hpp"
#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>
namespace vblck
{
namespace render
{
class BufferWritter
{
private:
	struct BufferToTexture2D
	{
		VkBuffer buffer;
		vk::Texture2D image;
	};
	std::vector<BufferToTexture2D> writesBufferToTexture2D;
	struct BufferToBuffer
	{
		VkBuffer src, dst;
		uint32_t size;
	};
	std::vector<BufferToBuffer> writesBufferToBuffer;

public:
	inline void writeBufferToImage(VkBuffer buffer, const vk::Texture2D& image)
	{
		BufferToTexture2D command;
		command.buffer = buffer;
		command.image = image;
		writesBufferToTexture2D.push_back(command);
	}

	inline void writeToSSBO(VkBuffer src, vk::SSBO* dst)
	{
		BufferToBuffer command;
		command.src = src;
		command.dst = dst->data.buffer;
		command.size = dst->size;
		writesBufferToBuffer.push_back(command);
	}

	void performWrites(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck