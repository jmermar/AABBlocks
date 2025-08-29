#include "textures.hpp"

using namespace vblck::vk;

void vblck::vk::Texture2D::clearColor(
	VkCommandBuffer cmd,
	float r,
	float g,
	float b,
	float a)
{
	assert(data.image && mipLevels > 0 &&
		   (format == VK_FORMAT_R8G8B8A8_UNORM ||
			format ==
				VK_FORMAT_R16G16B16A16_SFLOAT ||
			format ==
				VK_FORMAT_R32G32B32A32_SFLOAT));
	VkClearColorValue clearColor = {};
	clearColor.float32[0] = r;
	clearColor.float32[1] = g;
	clearColor.float32[2] = b;
	clearColor.float32[3] = a;
	VkImageSubresourceRange range =
		vk::Init::imageSubresourceRange(aspect);
	range.levelCount = mipLevels;
	range.layerCount = layers;

	vkCmdClearColorImage(cmd,
						 data.image,
						 layout,
						 &clearColor,
						 1,
						 &range);
}

void Texture2D::__create(VkDevice device,
						 VmaAllocator vma)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType =
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.extent = {
		extent.width, extent.height, 1};
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = layers;
	imageInfo.format = format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout =
		VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode =
		VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.requiredFlags =
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkImage image{};
	VmaAllocation alloc{};
	VKTRY(vmaCreateImage(vma,
						 &imageInfo,
						 &allocInfo,
						 &image,
						 &alloc,
						 nullptr));

	data.allocation = alloc;
	data.image = image;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType =
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = data.image;
	viewInfo.viewType = viewType;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask =
		usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
			? VK_IMAGE_ASPECT_DEPTH_BIT
			: VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount =
		mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layers;

	VKTRY(vkCreateImageView(
		device, &viewInfo, nullptr, &imageView));

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType =
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU =
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV =
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW =
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor =
		VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates =
		VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.mipmapMode =
		VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod =
		static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	VKTRY(vkCreateSampler(
		device, &samplerInfo, nullptr, &sampler));
}