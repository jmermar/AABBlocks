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

void loadWorlds()
{
	sceneState.worlds.clear();
	for(auto& f : listFilesInFolder("worlds/"))
	{
		if(f.size() >= 8)
		{
			auto ext = f.substr(f.size() - 8);
			auto name = f.substr(0, f.size() - 8);
			if(ext == std::string(".sqlite3"))
			{
				sceneState.worlds.push_back(name);
			}
		}
	}
}

bool nameRepeated(const std::string& name)
{
	for(auto& n : sceneState.worlds)
	{
		if(name == n)
			return true;
	}
	return false;
}
void sceneMainmenu_Init()
{
	sceneState.state = MAIN;
	loadWorlds();
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
		auto name = std::string(str0);

		if(nameRepeated(name))
		{
			ImGui::Text("A world with that name already exists.");
		}
		else
		{
			GameData::get()->world.name = name;
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
		}

		if(ImGui::Button("Return"))
		{
			sceneState.state = MAIN;
		}
	}
	else if(sceneState.state == LOAD_WORLD)
	{
		for(auto& name : sceneState.worlds)
		{
			std::string text = "World: \"" + name + "\"";
			if(ImGui::Button(text.c_str()))
			{
				scenes::sceneWorld_LoadWorld(name);
				return true;
			}
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