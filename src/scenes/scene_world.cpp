#include "scene_world.hpp"
#include "input.hpp"
#include "world/world.hpp"
#include "world/world_generator.hpp"
#include <imgui.h>
namespace vblck
{
namespace scenes
{

struct SceneData
{
	world::Player player;
	world::WorldGenerator worldGenerator;
} scene;
void sceneWorld_Init(uint32_t worldSize, uint32_t worldHeight)
{
	scene.player.init();
	scene.player.body.position = glm::vec3(1, 0, 1) * (float)(worldSize * 0.5f * world::CHUNK_SIZE);
	scene.player.body.position.y = 50;
	scene.worldGenerator.baseAmplitude = 20;
	scene.worldGenerator.baseHeight = 16;
	scene.worldGenerator.sandLevel = 25;
	scene.worldGenerator.world_height = worldHeight;
	scene.worldGenerator.world_size = worldSize;
	scene.worldGenerator.generateWorld();
}
void sceneWorld_Finish() { }

void sceneWorld_Update(float deltaTime, render::RenderState& renderState)
{
	auto* world = world::World::get();
	if(!scene.worldGenerator.finished)
	{
		return;
	}
	world->chunkGenerateCommands = std::move(scene.worldGenerator.chunksToGenerate);
	world->update(deltaTime);
	scene.player.update(deltaTime);

	renderState.camera.position = scene.player.body.position + scene.player.eye;
	renderState.camera.forward = scene.player.forward;
	renderState.cullCamera = renderState.camera;
}
void sceneWorld_FixedUpdate()
{
	if(!scene.worldGenerator.finished)
	{
		return;
	}
	scene.player.fixedUpdate();
}
bool sceneWorld_DrawGUI()
{
	if(!scene.worldGenerator.finished)
	{
		ImGui::Text("Loading world");
		ImGui::Text("Progress: %.2f", scene.worldGenerator.progress * 100.f);
		return false;
	}
	if(InputData::isPressed(INPUT_MENU))
	{
		InputData::setCaptureMosue(!InputData::getCaptureMosue());
	}

	if(!InputData::getCaptureMosue())
	{
		if(ImGui::Button("Return to title"))
		{
			return true;
		}
	}

	return false;
}
} // namespace scenes
} // namespace vblck