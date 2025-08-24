#include "scene_world.hpp"
#include "input.hpp"
#include "world/world_persistence.hpp"
#include <imgui.h>
namespace vblck
{
namespace scenes
{
SceneWorldData scene;

void sceneWorld_InitNewWorld(uint32_t worldSize, uint32_t worldHeight)
{
	scene.worldGenerator.baseAmplitude = 20;
	scene.worldGenerator.baseHeight = 16;
	scene.worldGenerator.sandLevel = 25;
	scene.worldGenerator.world_height = worldHeight;
	scene.worldGenerator.world_size = worldSize;
	scene.worldGenerator.generateNewWorld();
}

void sceneWorld_LoadWorld(const std::string& name)
{
	GameData::get()->world.name = name;

	scene.worldGenerator.loadWorld();
}

void onLoad()
{
	auto* world = world::World::get();
	scene.player.init();
	scene.player.body.position =
		glm::vec3(1, 0, 1) * (float)(world->worldSize * 0.5f * world::CHUNK_SIZE);
	scene.player.body.position.y = 50;
	scene.ui.blockSelect.currentSelect = 1;
	InputData::setCaptureMosue(true);
}
void sceneWorld_Finish()
{
	scene.loaded = false;
}

void sceneWorld_Update(float deltaTime, render::RenderState& renderState)
{
	auto* world = world::World::get();
	if(scene.worldGenerator.finished & !scene.loaded)
	{
		scene.loaded = true;
		onLoad();
	}
	if(!scene.loaded)
	{
		return;
	}
	world->chunkGenerateCommands = std::move(scene.worldGenerator.chunksToGenerate);
	world->update(deltaTime);

	if(!InputData::getCaptureMosue())
	{
		return;
	}

	if(InputData::isPressed(INPUT_SELECT_UP))
	{
		auto id = scene.ui.blockSelect.currentSelect - 1;
		scene.ui.blockSelect.currentSelect =
			1 + (world->blockDatabase.blocks.size() + id - 1) % world->blockDatabase.blocks.size();
	}

	if(InputData::isPressed(INPUT_SELECT_DOWN))
	{
		auto id = scene.ui.blockSelect.currentSelect - 1;
		scene.ui.blockSelect.currentSelect = 1 + (id + 1) % world->blockDatabase.blocks.size();
	}

	scene.player.update(deltaTime);

	renderState.camera.position = scene.player.body.position + scene.player.eye;
	renderState.camera.forward = scene.player.forward;
	renderState.cullCamera = renderState.camera;
}
void sceneWorld_FixedUpdate()
{
	if(!scene.loaded)
	{
		return;
	}
	if(!InputData::getCaptureMosue())
	{
		return;
	}
	scene.player.fixedUpdate();
}
bool sceneWorld_DrawGUI()
{
	auto* world = world::World::get();
	if(!scene.loaded)
	{
		ImGui::Text("Loading world");
		ImGui::Text("Progress: %.2f", scene.worldGenerator.progress * 100.f);
		return false;
	}
	if(InputData::isPressed(INPUT_MENU))
	{
		InputData::setCaptureMosue(!InputData::getCaptureMosue());
	}
	ImGui::Spacing();
	ImGui::Text("Select Block");
	for(uint32_t i = 1; i <= world->blockDatabase.blocks.size(); i++)
	{
		bool select = i == scene.ui.blockSelect.currentSelect;
		auto* block = world->blockDatabase.getBlockFromId(i);
		ImGui::MenuItem(block->name.c_str(), 0, &select);
	}

	if(!InputData::getCaptureMosue())
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(250, 120));

		ImGui::Begin("Pause Menu", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if(ImGui::Button("Save world"))
		{
			world::persistence::saveWorld(GameData::get()->world.name);
		}
		if(ImGui::Button("Return to title"))
		{
			ImGui::End();
			return true;
		}
		ImGui::End();
	}

	return false;
}
SceneWorldData* sceneWorld_getData()
{
	return &scene;
}
} // namespace scenes
} // namespace vblck