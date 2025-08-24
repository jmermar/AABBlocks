#include "scene_mainmenu.hpp"
#include "imgui.h"
#include "scene_world.hpp"
namespace vblck
{
namespace scenes
{
enum SceneState
{
	MAIN,
	NEW_WORLD,
	LOAD_WORLD
};
struct State
{
	SceneState state;

	std::vector<std::string> worlds;
} sceneState;

void loadWorlds() { }
void sceneMainmenu_Init()
{
	sceneState.state = MAIN;
}
void sceneMainmenu_Finish() { }
bool sceneMainmenu_DrawUI()
{
	if(sceneState.state == MAIN)
	{
		if(ImGui::Button("Create new world"))
		{
			sceneState.state = NEW_WORLD;
		}
		if(ImGui::Button("Load world"))
		{
			sceneState.state = LOAD_WORLD;
		}
	}
	else if(sceneState.state == NEW_WORLD)
	{
		static char str0[128] = "Enter world name";
		ImGui::InputText("World name", str0, 128);
		GameData::get()->world.name = std::string(str0);
		if(ImGui::Button("Gen 8x8 world"))
		{
			scenes::sceneWorld_InitNewWorld(8, 16);
			return true;
		}

		if(ImGui::Button("Gen 16x16 world"))
		{
			scenes::sceneWorld_InitNewWorld(16, 16);
			return true;
		}

		if(ImGui::Button("Gen 32x32 world"))
		{
			scenes::sceneWorld_InitNewWorld(32, 16);
			return true;
		}

		if(ImGui::Button("Gen 64x64 world"))
		{
			scenes::sceneWorld_InitNewWorld(64, 16);
			return true;
		}
		if(ImGui::Button("Gen 128x128 world"))
		{
			scenes::sceneWorld_InitNewWorld(128, 16);
			return true;
		}

		if(ImGui::Button("Return"))
		{
			sceneState.state = MAIN;
		}
	}
	else if(sceneState.state == LOAD_WORLD)
	{
		static char str0[128] = "Enter world name";
		ImGui::InputText("World name", str0, 128);

		if(ImGui::Button("Load"))
		{
			scenes::sceneWorld_LoadWorld(std::string(str0));
			return true;
		}

		if(ImGui::Button("Return"))
		{
			sceneState.state = MAIN;
		}
	}
	return false;
}
} // namespace scenes
} // namespace vblck