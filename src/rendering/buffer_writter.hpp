#pragma once

#include "texture_2d.hpp"
#include "utils/logger.hpp"
#include "vk/init.hpp"
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
		Texture2D* image;
	};
	std::vector<BufferToTexture2D> writesBufferToTexture2D;

public:
	inline void writeBufferToImage(VkBuffer buffer, Texture2D* image)
	{
		BufferToTexture2D command;
		command.buffer = buffer;
		command.image = image;

		writesBufferToTexture2D.push_back(command);
	}

	void performWrites(CommandBuffer* cmd);
};
} // namespace render
} // namespace vblck