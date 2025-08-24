#include "input_imp.hpp"
#include "input.hpp"
namespace vblck
{
std::pair<SDL_Scancode, InputEvents> INPUT_MAP[7] = {{SDL_SCANCODE_W, INPUT_MOVE_FORWARD},
													 {SDL_SCANCODE_S, INPUT_MOVE_BACKWARD},
													 {SDL_SCANCODE_A, INPUT_MOVE_LEFT},
													 {SDL_SCANCODE_D, INPUT_MOVE_RIGHT},
													 {SDL_SCANCODE_SPACE, INPUT_JUMP},
													 {SDL_SCANCODE_LSHIFT, INPUT_COUCH},
													 {SDL_SCANCODE_ESCAPE, INPUT_MENU}};
std::pair<uint8_t, InputEvents> MOUSE_INPUT_MAP[2] = {{1, INPUT_PRIMARY_ACTION},
													  {3, INPUT_SECONDARY_ACTION}};
InputData* inputData;

std::unordered_map<SDL_Scancode, InputEvents> inputMap;
std::unordered_map<uint8_t, InputEvents> mouseInputMap;
bool prevCaptureMouse;

void input_Init(SDL_Window* window)
{
	inputData = InputData::get();
	inputData->reset();
	for(int i = 0; i < 7; i++)
	{
		inputMap[INPUT_MAP[i].first] = INPUT_MAP[i].second;
	}
	for(int i = 0; i < 2; i++)
	{
		mouseInputMap[MOUSE_INPUT_MAP[i].first] = MOUSE_INPUT_MAP[i].second;
	}
	inputData->captureMouse = false;
	prevCaptureMouse = inputData->captureMouse;
}

void input_Update(SDL_Window* window, uint32_t w, uint32_t h)
{
	if(prevCaptureMouse != inputData->captureMouse)
	{
		prevCaptureMouse = inputData->captureMouse;
	}
	if(inputData->captureMouse)
	{
		SDL_WarpMouseInWindow(window, w / 2, h / 2);
	}
	inputData->axis = glm::vec2(0);
	for(int i = 0; i < INPUT_NUMBER; i++)
	{
		if(inputData->press[i])
		{
			inputData->pressed[i] = false;
		}
	}
}

void inputSetDown(InputEvents event)
{
	inputData->press[event] = inputData->pressed[event] = true;
	inputData->released[event] = false;
}

void inputSetUp(InputEvents event)
{
	inputData->press[event] = inputData->pressed[event] = false;
	inputData->released[event] = true;
}

void input_Event(SDL_Window* window, SDL_Event e)
{
	switch(e.type)
	{
		break;
	case SDL_EVENT_MOUSE_BUTTON_DOWN:
		if(mouseInputMap.contains(e.button.button))
		{
			auto event = mouseInputMap[e.button.button];
			if(!InputData::isDown(event))
			{
				inputSetDown(event);
			}
		}
		break;
	case SDL_EVENT_MOUSE_BUTTON_UP:
		if(mouseInputMap.contains(e.button.button))
		{
			auto event = mouseInputMap[e.button.button];
			inputSetUp(event);
		}
		break;
	case SDL_EVENT_MOUSE_WHEEL:
		if(e.wheel.y > 0)
		{
			inputSetDown(INPUT_SELECT_UP);
		}
		else if(e.wheel.y < 0)
		{
			inputSetDown(INPUT_SELECT_DOWN);
		}
		break;
	case SDL_EVENT_KEY_DOWN:
		if(inputMap.contains(e.key.scancode))
		{
			auto event = inputMap[e.key.scancode];
			if(!InputData::isDown(event))
			{
				inputSetDown(event);
			}
		}
		break;
	case SDL_EVENT_KEY_UP:
		if(inputMap.contains(e.key.scancode))
		{
			auto event = inputMap[e.key.scancode];
			inputSetUp(event);
		}
		break;
	case SDL_EVENT_MOUSE_MOTION:
		if(inputData->captureMouse)
		{
			int dx = -e.motion.xrel;
			int dy = -e.motion.yrel;
			inputData->axis.x = dx / 50.f;
			inputData->axis.y = dy / 50.f;
		}
		break;
	}
}
} // namespace vblck