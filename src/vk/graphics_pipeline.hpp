#pragma once
#include "utils/errors.hpp"
#include <span>
#include <vector>
#include <vulkan/vulkan.h>
namespace vblck
{
namespace vk
{
inline VkShaderModule createShaderModule(VkDevice device, std::span<uint8_t> data)
{
	VkShaderModuleCreateInfo info = {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	info.codeSize = data.size();
	info.pCode = (uint32_t*)data.data();

	VkShaderModule shaderModule;

	VKTRY(vkCreateShaderModule(device, &info, nullptr, &shaderModule));

	return shaderModule;
}

class GraphicsPipelineBuilder
{
public:
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;

	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
	VkPipelineRenderingCreateInfo _renderInfo;
	std::vector<VkFormat> _colorAttachmentformats;

	GraphicsPipelineBuilder()
	{
		clear();
	}

	void clear();

	VkPipeline buildPipeline(VkDevice device);

	void set_shaders(VkShaderModule vertexShader, VkShaderModule fragmentShader)
	{
		_shaderStages.clear();

		VkPipelineShaderStageCreateInfo info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};

		info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		info.module = vertexShader;
		info.pName = "main";

		_shaderStages.push_back(info);

		info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		info.module = fragmentShader;
		info.pName = "main";

		_shaderStages.push_back(info);
	}

	void set_input_topology(VkPrimitiveTopology topology)
	{
		_inputAssembly.topology = topology;
		// we are not going to use primitive restart on the entire tutorial so leave
		// it on false
		_inputAssembly.primitiveRestartEnable = VK_FALSE;
	}

	void set_polygon_mode(VkPolygonMode mode)
	{
		_rasterizer.polygonMode = mode;
		_rasterizer.lineWidth = 1.f;
	}

	void set_cull_mode(VkCullModeFlags cullMode, VkFrontFace frontFace)
	{
		_rasterizer.cullMode = cullMode;
		_rasterizer.frontFace = frontFace;
	}

	void set_multisampling_none()
	{
		_multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling defaulted to no multisampling (1 sample per pixel)
		_multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		_multisampling.minSampleShading = 1.0f;
		_multisampling.pSampleMask = nullptr;
		// no alpha to coverage either
		_multisampling.alphaToCoverageEnable = VK_FALSE;
		_multisampling.alphaToOneEnable = VK_FALSE;
	}

	void disable_blending()
	{
		// default write mask
		_colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
											   VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		// no blending
		_colorBlendAttachment.blendEnable = VK_FALSE;
	}

	void set_color_attachment_format(VkFormat format)
	{
		_colorAttachmentformats.resize(1);
		_colorAttachmentformats[0] = format;
		// connect the format to the renderInfo  structure
		_renderInfo.colorAttachmentCount = 1;
		_renderInfo.pColorAttachmentFormats = _colorAttachmentformats.data();
	}

	void set_color_attachment_format(std::span<VkFormat> formats)
	{
		_colorAttachmentformats.resize(formats.size());
		for(size_t i = 0; i < formats.size(); i++)
		{
			_colorAttachmentformats[i] = formats[i];
		}
		// connect the format to the renderInfo  structure
		_renderInfo.colorAttachmentCount = _colorAttachmentformats.size();
		_renderInfo.pColorAttachmentFormats = _colorAttachmentformats.data();
	}

	void set_depth_format(VkFormat format)
	{
		_renderInfo.depthAttachmentFormat = format;
	}

	void disable_depthtest()
	{
		_depthStencil.depthTestEnable = VK_FALSE;
		_depthStencil.depthWriteEnable = VK_FALSE;
		_depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
		_depthStencil.depthBoundsTestEnable = VK_FALSE;
		_depthStencil.stencilTestEnable = VK_FALSE;
		_depthStencil.front = {};
		_depthStencil.back = {};
		_depthStencil.minDepthBounds = 0.f;
		_depthStencil.maxDepthBounds = 1.f;
	}

	void enable_depthtest()
	{
		_depthStencil.depthTestEnable = VK_TRUE;
		_depthStencil.depthWriteEnable = VK_TRUE;
		_depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		_depthStencil.depthBoundsTestEnable = VK_FALSE;
		_depthStencil.stencilTestEnable = VK_FALSE;
		_depthStencil.front = {};
		_depthStencil.back = {};
		_depthStencil.minDepthBounds = 0.f;
		_depthStencil.maxDepthBounds = 1.f;
	}
};

} // namespace vk
} // namespace vblck