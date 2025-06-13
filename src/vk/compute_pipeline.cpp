#include "compute_pipeline.hpp"
#include "utils/errors.hpp"
namespace vblck
{
namespace vk
{
VkPipeline createComputePipeline(VkDevice device, VkPipelineLayout layout, VkShaderModule compute)
{
	VkComputePipelineCreateInfo computeCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

	computeCreateInfo.layout = layout;

	VkPipelineShaderStageCreateInfo shaderInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

	shaderInfo.module = compute;
	shaderInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	shaderInfo.pName = "main";

	computeCreateInfo.stage = shaderInfo;

	VkPipeline pipeline{};

	VKTRY(vkCreateComputePipelines(
		device, VK_NULL_HANDLE, 1, &computeCreateInfo, nullptr, &pipeline));

	return pipeline;
}
} // namespace vk
} // namespace vblck