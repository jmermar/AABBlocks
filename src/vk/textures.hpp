#pragma once

#include "deletion.hpp"
#include "init.hpp"
#include "types.hpp"
namespace vblck
{
namespace vk
{

inline void copyImageToImage(VkCommandBuffer cmd,
							 VkImage source,
							 VkImage destination,
							 VkExtent2D srcSize,
							 VkExtent2D dstSize,
							 uint32_t srcMipLevel,
							 uint32_t dstMipLevel)
{
	VkImageBlit2 blitRegion{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr};

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel =
		srcMipLevel;

	blitRegion.dstSubresource.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel =
		dstMipLevel;

	VkBlitImageInfo2 blitInfo{
		.sType =
			VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr};
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout =
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

inline void
copyImageArrayToImageArray(VkCommandBuffer cmd,
						   VkImage source,
						   VkImage destination,
						   VkExtent2D srcSize,
						   VkExtent2D dstSize,
						   uint32_t srcMipLevel,
						   uint32_t dstMipLevel,
						   uint32_t layers)
{
	VkImageBlit2 blitRegion{
		.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2,
		.pNext = nullptr};

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = layers;
	blitRegion.srcSubresource.mipLevel =
		srcMipLevel;

	blitRegion.dstSubresource.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = layers;
	blitRegion.dstSubresource.mipLevel =
		dstMipLevel;

	VkBlitImageInfo2 blitInfo{
		.sType =
			VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2,
		.pNext = nullptr};
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout =
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout =
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

inline void regenerateMipmaps(VkCommandBuffer cmd,
							  VkImage image,
							  VkExtent2D extent,
							  uint32_t mipLevels,
							  uint32_t layers = 1)
{
	assert(mipLevels > 1);

	VkImageMemoryBarrier barrier{};
	barrier.sType =
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout =
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask =
		VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask =
		VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.subresourceRange.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layers;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		nullptr,
		0,
		nullptr,
		1,
		&barrier);

	uint32_t mipWidth = extent.width;
	uint32_t mipHeight = extent.height;
	for(uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel =
			i - 1;
		barrier.oldLayout =
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout =
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask =
			VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask =
			VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		VkExtent2D srcOffset = {mipWidth,
								mipHeight};

		VkExtent2D dstOffset = {
			mipWidth > 1 ? mipWidth / 2 : 1,
			mipHeight > 1 ? mipHeight / 2 : 1};

		// Transición del destino (mip i)
		barrier.subresourceRange.baseMipLevel = i;
		barrier.oldLayout =
			VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout =
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask =
			VK_ACCESS_TRANSFER_WRITE_BIT;

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);
		if(layers == 1)
		{

			vk::copyImageToImage(cmd,
								 image,
								 image,
								 srcOffset,
								 dstOffset,
								 i - 1,
								 i);
		}
		else
		{
			vk::copyImageArrayToImageArray(
				cmd,
				image,
				image,
				srcOffset,
				dstOffset,
				i - 1,
				i,
				layers);
		}
		barrier.subresourceRange.baseMipLevel =
			i - 1;
		barrier.oldLayout =
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout =
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask =
			VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask =
			VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&barrier);

		mipWidth =
			std::max(mipWidth / 2, (uint32_t)1);
		mipHeight =
			std::max(mipHeight / 2, (uint32_t)1);
	}
}

inline void
copyBufferToImage(VkCommandBuffer cmd,
				  VkBuffer buffer,
				  VkImage image,
				  VkExtent2D dstSize,
				  uint32_t dstMipLevel,
				  uint32_t layers = 1)
{
	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;

	copyRegion.imageSubresource.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel =
		dstMipLevel;
	copyRegion.imageSubresource.baseArrayLayer =
		0;
	copyRegion.imageSubresource.layerCount =
		layers;

	copyRegion.imageOffset = {0, 0, 0};
	copyRegion.imageExtent = {
		dstSize.width, dstSize.height, 1};

	vkCmdCopyBufferToImage(
		cmd,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion);
}

struct Texture2D
{
	vk::Image data{};
	VkImageView imageView{};
	VkExtent2D extent{};
	uint32_t layers{};
	VkSampler sampler{};
	uint32_t mipLevels{};
	VkImageLayout layout{
		VK_IMAGE_LAYOUT_UNDEFINED};
	VkFormat format{};
	VkImageUsageFlags usage{};
	VkImageAspectFlags aspect{};
	VkImageType imageType{VK_IMAGE_TYPE_2D};
	VkImageViewType viewType{
		VK_IMAGE_VIEW_TYPE_2D};

	void createRGBATexture(VkDevice device,
						   VmaAllocator vma,
						   VkExtent2D size,
						   uint32_t layers = 1,
						   uint32_t mipLevels = 1)
	{
		this->extent = size;
		this->layers = layers;
		this->mipLevels = mipLevels;
		this->format = VK_FORMAT_R8G8B8A8_UNORM;
		this->usage =
			VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		this->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		this->imageType = VK_IMAGE_TYPE_2D;
		this->viewType =
			(layers > 1)
				? VK_IMAGE_VIEW_TYPE_2D_ARRAY
				: VK_IMAGE_VIEW_TYPE_2D;
		__create(device, vma);
	}
	void
	createRGBA16Texture(VkDevice device,
						VmaAllocator vma,
						VkExtent2D size,
						uint32_t layers = 1,
						uint32_t mipLevels = 1)
	{
		this->extent = size;
		this->layers = layers;
		this->mipLevels = mipLevels;
		this->format =
			VK_FORMAT_R16G16B16A16_SFLOAT;
		this->usage =
			VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		this->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		this->imageType = VK_IMAGE_TYPE_2D;
		this->viewType =
			(layers > 1)
				? VK_IMAGE_VIEW_TYPE_2D_ARRAY
				: VK_IMAGE_VIEW_TYPE_2D;
		__create(device, vma);
	}
	void
	createDepthTexture(VkDevice device,
					   VmaAllocator vma,
					   VkExtent2D size,
					   uint32_t layers = 1,
					   uint32_t mipLevels = 1)
	{
		this->extent = size;
		this->layers = layers;
		this->mipLevels = mipLevels;
		this->format = VK_FORMAT_D32_SFLOAT;
		this->usage =
			VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		this->aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		this->imageType = VK_IMAGE_TYPE_2D;
		this->viewType =
			(layers > 1)
				? VK_IMAGE_VIEW_TYPE_2D_ARRAY
				: VK_IMAGE_VIEW_TYPE_2D;
		__create(device, vma);
	}
	void createCubeMap(VkDevice device,
					   VmaAllocator vma,
					   VkExtent2D size,
					   uint32_t mipLevels = 1)
	{
		this->extent = size;
		this->layers = 6;
		this->mipLevels = mipLevels;
		this->format = VK_FORMAT_R8G8B8A8_UNORM;
		this->usage =
			VK_IMAGE_USAGE_SAMPLED_BIT |
			VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		this->aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		this->imageType = VK_IMAGE_TYPE_2D;
		this->viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		__create(device, vma);
	}
	inline void
	destroy(vk::DeletionQueue* deletionQueue)
	{
		assert(data.image && mipLevels > 0 &&
			   sampler && imageView);
		assert(deletionQueue);
		deletionQueue->images.push_back(data);
		deletionQueue->imageViews.push_back(
			imageView);
		deletionQueue->samplers.push_back(
			sampler);
	}

	inline void
	copyFromBuffer(VkCommandBuffer cmd,
				   VkBuffer buffer)
	{
		assert(buffer && data.image &&
			   mipLevels > 0);
		VkExtent2D size = {extent.width,
						   extent.height};
		copyBufferToImage(cmd,
						  buffer,
						  data.image,
						  size,
						  0,
						  layers);
		if(mipLevels > 1)
		{
			regenerateMipmaps(cmd,
							  data.image,
							  size,
							  mipLevels,
							  layers);
			layout =
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		}
	}

	inline void
	transition(VkCommandBuffer cmd,
			   VkImageLayout newLayout)
	{
		VkImageMemoryBarrier2 imageBarrier{
			.sType =
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
		imageBarrier.pNext = nullptr;

		imageBarrier.srcStageMask =
			VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.srcAccessMask =
			VK_ACCESS_2_MEMORY_WRITE_BIT |
			VK_ACCESS_2_TRANSFER_WRITE_BIT;
		imageBarrier.dstStageMask =
			VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
		imageBarrier.dstAccessMask =
			VK_ACCESS_2_MEMORY_WRITE_BIT |
			VK_ACCESS_2_MEMORY_READ_BIT |
			VK_ACCESS_2_TRANSFER_WRITE_BIT |
			VK_ACCESS_2_TRANSFER_READ_BIT;

		imageBarrier.oldLayout = layout;
		imageBarrier.newLayout = newLayout;

		VkImageAspectFlags aspectMask =
			(usage &
			 VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
				? VK_IMAGE_ASPECT_DEPTH_BIT
				: VK_IMAGE_ASPECT_COLOR_BIT;

		imageBarrier.subresourceRange =
			vk::Init::imageSubresourceRange(
				aspectMask);
		imageBarrier.image = data.image;

		VkDependencyInfo depInfo{};
		depInfo.sType =
			VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
		depInfo.pNext = nullptr;

		depInfo.imageMemoryBarrierCount = 1;
		depInfo.pImageMemoryBarriers =
			&imageBarrier;

		vkCmdPipelineBarrier2(cmd, &depInfo);

		layout = newLayout;
	}

	void clearColor(VkCommandBuffer cmd,
					float r,
					float g,
					float b,
					float a);

	void __create(VkDevice device,
				  VmaAllocator vma);
};

} // namespace vk
} // namespace vblck