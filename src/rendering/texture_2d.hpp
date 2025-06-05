#pragma once
#include "command_buffer.hpp"
#include "types.hpp"
#include "utils/logger.hpp"
#include "vk/deletion.hpp"
#include "vk/types.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace vblck
{
namespace render
{
class Texture2D
{
private:
	VkDevice device;
	VmaAllocator vma;
	vk::Image data{};
	vk::DeletionQueue* deletionQueue{};

	VkImageView imageView{};
	VkExtent3D extent{};
	VkSampler sampler{};
	uint32_t mipLevels{};

	void createTexture();
	void createViewAndSampler();

public:
	Texture2D(VkDevice device,
			  VmaAllocator vma,
			  vk::DeletionQueue& deletion,
			  int w,
			  int h,
			  int mipLevels = 1)
		: device(device)
		, vma(vma)
		, deletionQueue(&deletion)
		, mipLevels(mipLevels)
	{
		extent.width = w;
		extent.height = h;
		extent.depth = 1;

		createTexture();
		createViewAndSampler();
	}

	~Texture2D()
	{
		if(deletionQueue)
		{
			deletionQueue->images.push_back(data);
			deletionQueue->imageViews.push_back(imageView);
			deletionQueue->samplers.push_back(sampler);
		}

		data = vk::Image();
		imageView = 0;
		sampler = 0;
		deletionQueue = 0;
	}

	inline VkImage getImage()
	{
		return data.image;
	}

	inline VkExtent2D getSize()
	{
		VkExtent2D size;
		size.width = extent.width;
		size.height = extent.height;
		return size;
	}

	void clear(CommandBuffer* cmd, float r, float g, float b, float a);
	void transition(CommandBuffer* cmd, VkImageLayout currentLayout, VkImageLayout newLayout);
};
} // namespace render
} // namespace vblck
