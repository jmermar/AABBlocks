#include "debug.hpp"
#include "imgui.h"

namespace vblck
{
render::RenderState::DebugRenderBuffer debugGetRenderBuffer()
{
	static int selectedIndex = 0;
	const char* options[6] = {"nothing", "albedo", "normal", "position", "depth", "material"};
	render::RenderState::DebugRenderBuffer returns[6] = {render::RenderState::NOTHING,
														 render::RenderState::ALBEDO,
														 render::RenderState::NORMAL,
														 render::RenderState::POSITION,
														 render::RenderState::DEPTH,
														 render::RenderState::MATERIAL};
	if(ImGui::BeginCombo("Display buffer", options[selectedIndex], ImGuiComboFlags_HeightLarge))
	{

		for(int i = 0; i < 6; ++i)
		{
			const bool isSelected = (selectedIndex == i);

			if(isSelected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
			}

			if(ImGui::Selectable(options[i], isSelected))
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
} // namespace vblck