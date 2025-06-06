#pragma once

#include "init.hpp"

namespace vblck
{
struct IMGUI_Instance;

IMGUI_Instance* initImgui(const System& system);
void finishImgui(const System& system, IMGUI_Instance* imgui);
} // namespace vblck