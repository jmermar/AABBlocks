#pragma once
#include <cstdint>
#include <glm/glm.hpp>
namespace vblck
{
enum InputEvents
{
	INPUT_MOVE_FORWARD = 0,
	INPUT_MOVE_BACKWARD,
	INPUT_MOVE_LEFT,
	INPUT_MOVE_RIGHT,
	INPUT_JUMP,
	INPUT_COUCH,

	INPUT_MENU,

	INPUT_PRIMARY_ACTION,
	INPUT_SECONDARY_ACTION,

	INPUT_NUMBER
};
struct InputData
{
	bool press[INPUT_NUMBER];
	bool released[INPUT_NUMBER];
	bool pressed[INPUT_NUMBER];
	bool captureMouse = 0;

	glm::vec2 axis;

	static inline InputData* get()
	{
		static InputData data;
		return &data;
	}

	static inline bool isDown(InputEvents event)
	{
		return get()->press[event];
	}

	static inline bool isPressed(InputEvents event)
	{
		return get()->pressed[event];
	}

	static glm::vec2 getAxis()
	{
		return get()->axis;
	}

	void reset()
	{
		for(int i = 0; i < INPUT_NUMBER; i++)
		{
			press[i] = pressed[i] = 0;
			released[i] = 1;
		}
	};

	static inline void setCaptureMosue(bool capture)
	{
		get()->captureMouse = capture;
	}

	static inline bool getCaptureMosue()
	{
		return get()->captureMouse;
	}
};
} // namespace vblck