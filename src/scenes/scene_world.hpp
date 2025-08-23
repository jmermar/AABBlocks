#pragma once
#include "rendering/renderer.hpp"
#include "types.hpp"

namespace vblck
{
namespace scenes
{
void sceneWorld_Init(uint32_t worldSize, uint32_t worldHeight);
void sceneWorld_Finish();
void sceneWorld_Update(float deltaTime, render::RenderState& rs);
void sceneWorld_FixedUpdate();
bool sceneWorld_DrawGUI();
} // namespace scenes
} // namespace vblck