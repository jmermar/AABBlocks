#include "buffer_writter.hpp"
#include "vk/textures.hpp"

void vblck::render::BufferWritter::performWrites(
	VkCommandBuffer cmd)
{
	VkMemoryBarrier preCopyBarrier{};
	preCopyBarrier.sType =
		VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	preCopyBarrier.srcAccessMask =
		VK_ACCESS_HOST_WRITE_BIT |
		VK_ACCESS_TRANSFER_WRITE_BIT;
	preCopyBarrier.dstAccessMask =
		VK_ACCESS_TRANSFER_READ_BIT;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT, // dstStage
		0,
		1,
		&preCopyBarrier,
		0,
		nullptr,
		0,
		nullptr);

	for(auto& [buffer, image] :
		writesBufferToTexture2D)
	{
		image.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image.copyFromBuffer(cmd, buffer);
	}
	writesBufferToTexture2D.clear();

	for(auto& [buffer, image] :
		writesBufferToTexture2DArray)
	{
		image.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		image.copyFromBuffer(cmd, buffer);
	}
	writesBufferToTexture2DArray.clear();

	for(auto& [src, dst, size] :
		writesBufferToBuffer)
	{
		vk::copyBufferToBuffer(
			cmd, src, dst, size, 0, 0);
	}
	writesBufferToBuffer.clear();

	VkMemoryBarrier postCopyBarrier{};
	postCopyBarrier.sType =
		VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	postCopyBarrier.srcAccessMask =
		VK_ACCESS_TRANSFER_WRITE_BIT;
	postCopyBarrier.dstAccessMask =
		VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, // dstStage
		0,
		1,
		&postCopyBarrier,
		0,
		nullptr,
		0,
		nullptr);
}