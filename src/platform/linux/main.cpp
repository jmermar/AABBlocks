#include "init.hpp"
#include "init_imgui.hpp"
#include "input.hpp"
#include "rendering/renderer.hpp"
#include "world/world.hpp"
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

std::pair<SDL_Scancode, InputEvents> INPUT_MAP[INPUT_NUMBER] = {
	{SDL_SCANCODE_W, INPUT_MOVE_FORWARD},
	{SDL_SCANCODE_S, INPUT_MOVE_BACKWARD},
	{SDL_SCANCODE_A, INPUT_MOVE_LEFT},
	{SDL_SCANCODE_D, INPUT_MOVE_RIGHT},
	{SDL_SCANCODE_SPACE, INPUT_JUMP},
	{SDL_SCANCODE_LSHIFT, INPUT_CROUCH},
	{SDL_SCANCODE_ESCAPE, INPUT_MENU}};

std::unordered_map<SDL_Scancode, InputEvents> getInputMap()
{
	std::unordered_map<SDL_Scancode, InputEvents> map;
	for(int i = 0; i < INPUT_NUMBER; i++)
	{
		map[INPUT_MAP[i].first] = INPUT_MAP[i].second;
	}

	return map;
}

namespace vblck
{

std::shared_ptr<spdlog::logger>& getLogger()
{
	return logger;
}
} // namespace vblck

int main(int argc, char** argv)
{
	logger = spdlog::stdout_color_mt("VKP");
	// Init input
	auto* inputData = InputData::get();
	inputData->reset();
	auto inputMap = getInputMap();

	// Init world
	auto* world = world::World::get();
	world->generateWorld();

	System system{};

	system = initSystemLinux("Vulkan App", 1920, 1080);
	auto imguiInstance = initImgui(system);

	SDL_SetWindowRelativeMouseMode(system.window, false);
	inputData->captureMouse = false;

	auto* renderer = new render::Renderer(system.instance,
										  system.chosenGPU,
										  system.device,
										  system.surface,
										  system.graphicsQueue,
										  system.graphicsQueueFamily,
										  1920,
										  1080);

	bool running = true;
	auto ticks = SDL_GetTicks();

	render::RenderSate renderState{};
	renderState.camera.fov = 45;
	renderState.camera.aspect = 1920.f / 1080.f;
	renderState.camera.position = glm::vec3(50, 20, 50);
	renderState.camera.forward = glm::vec3(0, 0, 1);
	uint64_t frameDelta = 0;
	float deltaTime = 0;

	world->generateWorld();

	for(auto& cmd : world->chunkGenerateCommands)
	{
		renderer->worldRenderer->chunkRenderer.loadChunk(cmd.position, cmd.data);
	}
	world->chunkGenerateCommands.clear();
	bool prevCaptureMouse = inputData->captureMouse;
	while(running)
	{
		if(prevCaptureMouse != inputData->captureMouse)
		{
			SDL_SetWindowRelativeMouseMode(system.window, inputData->captureMouse);
			prevCaptureMouse = inputData->captureMouse;
		}
		inputData->axis = glm::vec2(0);
		for(int i = 0; i < INPUT_NUMBER; i++)
		{
			if(inputData->press[i])
			{
				inputData->pressed[i] = false;
			}
		}
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED: {
				auto w = e.window.data1;
				auto h = e.window.data2;
				renderer->recreateSwapchain(w, h);
				renderState.camera.aspect = (float)w / (float)h;
			}
			break;
			case SDL_EVENT_KEY_DOWN:
				if(inputMap.contains(e.key.scancode))
				{
					auto event = inputMap[e.key.scancode];
					if(!InputData::isDown(event))
					{
						inputData->press[event] = inputData->pressed[event] = true;
						inputData->released[event] = false;
					}
				}
				break;
			case SDL_EVENT_KEY_UP:
				if(inputMap.contains(e.key.scancode))
				{
					auto event = inputMap[e.key.scancode];
					inputData->press[event] = inputData->pressed[event] = false;
					inputData->released[event] = true;
				}
				break;
			case SDL_EVENT_MOUSE_MOTION:
				if(inputData->captureMouse)
				{
					int dx = e.motion.xrel;
					int dy = e.motion.yrel;
					inputData->axis.x = dx / 50.f;
					inputData->axis.y = dy / 50.f;
				}
				break;
			}
			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Text("FPS: %f", 1000.f / frameDelta);

		ImGui::Render();

		world->update(deltaTime);

		renderState.camera.position = world->player.position;
		renderState.camera.forward = world->player.forward;

		renderer->renderFrame(renderState);

		frameDelta = SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();
		deltaTime = frameDelta / 1000.f;
	}
	delete renderer;

	finishImgui(system, imguiInstance);

	finishSystemLinux(system);

	return 0;
}