#pragma once
#include "linux_system.hpp"

namespace vblck
{
void input_Init(SDL_Window* window);
void input_Update(SDL_Window* window, uint32_t w, uint32_t h);
void input_Event(SDL_Window* window, SDL_Event e);
} // namespace vblck