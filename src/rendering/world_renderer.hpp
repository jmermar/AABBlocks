#pragma once
#include "vk/types.hpp"

namespace vblck
{
namespace render
{
class WorldRenderer
{
private:
	VkDevice device{};
	VmaAllocator vma{};
	VkPipeline pipeline{};
	VkPipelineLayout pipelineLayout{};

	void initPipelineLayout();
	void initPipeline();
	void destroy();

public:
	WorldRenderer(VkDevice device, VmaAllocator vma)
		: device(device)
		, vma(vma)
	{
		initPipelineLayout();
		initPipeline();
	}
	~WorldRenderer()
	{
		destroy();
	}

	void render(VkCommandBuffer cmd);
};
} // namespace render
} // namespace vblck