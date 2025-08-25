#include "debug.hpp"
#include "imgui.h"
#include "renderer.hpp"
namespace vblck
{
namespace render
{
DebugRenderer::RenderBuffer selectRenderBuffer()
{
	static int selectedIndex = 0;
	const char* options[6] = {"nothing",
							  "albedo",
							  "normal",
							  "position",
							  "depth",
							  "material"};
	DebugRenderer::RenderBuffer returns[6] = {
		DebugRenderer::RenderBuffer::NOTHING,
		DebugRenderer::RenderBuffer::ALBEDO,
		DebugRenderer::RenderBuffer::NORMAL,
		DebugRenderer::RenderBuffer::POSITION,
		DebugRenderer::RenderBuffer::DEPTH,
		DebugRenderer::RenderBuffer::MATERIAL};
	if(ImGui::BeginCombo(
		   "Display buffer",
		   options[selectedIndex],
		   ImGuiComboFlags_HeightLarge))
	{

		for(int i = 0; i < 6; ++i)
		{
			const bool isSelected =
				(selectedIndex == i);

			if(isSelected)
			{
				ImGui::PushStyleColor(
					ImGuiCol_Text,
					ImVec4(
						1.0f, 0.8f, 0.0f, 1.0f));
			}

			if(ImGui::Selectable(options[i],
								 isSelected))
			{
				if(selectedIndex != i)
				{
					selectedIndex = i;
				}
				ImGui::CloseCurrentPopup();
			}

			if(isSelected)
			{
				ImGui::PopStyleColor();
				if(ImGui::IsWindowAppearing())
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}
	return returns[selectedIndex];
}
void DebugRenderer::drawDebugUI()
{
	ImGui::Begin("Debug");
	renderBuffer = selectRenderBuffer();
	ImGui::End();
}
void DebugRenderer::render(VkCommandBuffer cmd)
{
	auto* render = Renderer::get();
	auto* deferredBuffers =
		&render->deferredBuffers;

	if(renderBuffer == POSITION)
	{
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		deferredBuffers->pos.transition(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::copyImageToImage(
			cmd,
			deferredBuffers->pos.data.image,
			render->backbuffer.data.image,
			render->screenExtent,
			render->screenExtent,
			0,
			0);
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	else if(renderBuffer == ALBEDO)
	{
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		deferredBuffers->albedo.transition(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::copyImageToImage(
			cmd,
			deferredBuffers->albedo.data.image,
			render->backbuffer.data.image,
			render->screenExtent,
			render->screenExtent,
			0,
			0);
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	else if(renderBuffer == NORMAL)
	{
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		deferredBuffers->normal.transition(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::copyImageToImage(
			cmd,
			deferredBuffers->normal.data.image,
			render->backbuffer.data.image,
			render->screenExtent,
			render->screenExtent,
			0,
			0);
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	else if(renderBuffer == MATERIAL)
	{
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		deferredBuffers->material.transition(
			cmd,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		vk::copyImageToImage(
			cmd,
			deferredBuffers->material.data.image,
			render->backbuffer.data.image,
			render->screenExtent,
			render->screenExtent,
			0,
			0);
		render->backbuffer.transition(
			cmd,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	}
	else if(renderBuffer == DEPTH)
	{
	}
}
} // namespace render
} // namespace vblck