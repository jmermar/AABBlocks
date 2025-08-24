#pragma once
#include "rendering/renderer.hpp"
#include "types.hpp"
#include "world/world.hpp"
#include "world/world_generator.hpp"

namespace vblck
{
namespace scenes
{
struct UIState
{
	struct
	{
		uint16_t currentSelect;
	} blockSelect;
};

struct SceneWorldData
{
	UIState ui;
	world::Player player;
	world::WorldGenerator worldGenerator;
};

void sceneWorld_Init(uint32_t worldSize, uint32_t worldHeight);
void sceneWorld_Finish();
void sceneWorld_Update(float deltaTime, render::RenderState& rs);
void sceneWorld_FixedUpdate();
bool sceneWorld_DrawGUI();

SceneWorldData* sceneWorld_getData();
} // namespace scenes
} // namespace vblck