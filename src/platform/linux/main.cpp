#include "init.hpp"
#include "init_imgui.hpp"
#include "input.hpp"
#include "input_imp.hpp"
#include "rendering/renderer.hpp"
#include "scenes/scene_mainmenu.hpp"
#include "scenes/scene_world.hpp"
#include "world/world.hpp"
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>
#include <thread>
using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

namespace vblck
{
GameData gameData;

GameData* GameData::get()
{
	return &gameData;
}
std::shared_ptr<spdlog::logger>& getLogger()
{
	return logger;
}
} // namespace vblck

enum SceneState
{
	SCENE_STATE_MAINMENU,
	SCENE_STATE_WORLD
};

std::atomic<SceneState> sceneState;

std::atomic<bool> running;

constexpr uint32_t TICKS_PER_SECOND = 50;
constexpr float PHYSIQS_DELTA =
	1.f / TICKS_PER_SECOND;

void fixedLoop()
{
	auto delta = SDL_GetTicks();
	while(running)
	{

		if(sceneState == SCENE_STATE_WORLD)
		{
			scenes::sceneWorld_FixedUpdate();
		}

		auto elapsed = SDL_GetTicks() - delta;
		if(elapsed < 1000 / 50)
		{
			SDL_Delay((1000 / 50) - elapsed);
		}
		delta = SDL_GetTicks();
	}
}

int main(int argc, char** argv)
{
	logger = spdlog::stdout_color_mt("VKP");

	System system{};

	gameData.screen.width = 1920;
	gameData.screen.height = 1080;

	system =
		initSystemLinux("Vulkan App",
						gameData.screen.width,
						gameData.screen.height);
	auto imguiInstance = initImgui(system);

	auto* renderer = new render::Renderer(
		system.instance,
		system.chosenGPU,
		system.device,
		system.surface,
		system.graphicsQueue,
		system.graphicsQueueFamily,
		gameData.screen.width,
		gameData.screen.height);

	running = true;
	auto ticks = SDL_GetTicks();

	input_Init(system.window);

	render::RenderState renderState{};
	renderState.camera.fov = 45;
	renderState.camera.aspect = 1920.f / 1080.f;
	renderState.camera.position =
		glm::vec3(50, 20, 50);
	renderState.camera.forward =
		glm::vec3(0, 0, 1);
	renderState.cullCamera = renderState.camera;
	uint64_t frameDelta = 0;
	float deltaTime = 0;

	sceneState = SCENE_STATE_MAINMENU;

	std::thread pthread(fixedLoop);

	scenes::sceneMainmenu_Init();

	while(running)
	{
		input_Update(system.window,
					 gameData.screen.width,
					 gameData.screen.height);
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED: {
				auto iw = e.window.data1;
				auto ih = e.window.data2;
				renderer->recreateSwapchain(
					gameData.screen.width,
					gameData.screen.height);
				gameData.screen.width =
					(uint32_t)iw;
				gameData.screen.height =
					(uint32_t)ih;
				renderState.camera.aspect =
					(float)gameData.screen.width /
					(float)gameData.screen.height;
			}
			break;
			}
			input_Event(system.window, e);
			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		renderState.drawDebug = true;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();

		ImGui::NewFrame();
		ImGui::Text("FPS: %f",
					1000.f / frameDelta);
		ImGui::InputFloat("Ambient",
						  &renderState.ambient,
						  0.01f,
						  0.1f,
						  "%.3f");
		ImGui::InputFloat3(
			"Light Direction",
			&renderState.lightDirection[0],
			"%.3f");
		ImGui::InputFloat(
			"Light Intensity",
			&renderState.lightIntensity,
			0.01f,
			0.1f,
			"%.3f");

		renderer->imGUIDefaultRender();

		if(sceneState == SCENE_STATE_MAINMENU)
		{
			if(scenes::sceneMainmenu_DrawUI())
			{
				scenes::sceneMainmenu_Finish();
				sceneState = SCENE_STATE_WORLD;
			}
		}
		else if(sceneState == SCENE_STATE_WORLD)
		{
			if(scenes::sceneWorld_DrawGUI())
			{
				sceneState = SCENE_STATE_MAINMENU;
				scenes::sceneWorld_Finish();
				renderer->worldRenderer
					.clearWorld();
				scenes::sceneMainmenu_Init();
			}
		}

		ImGui::Render();

		if(sceneState == SCENE_STATE_WORLD)
		{
			scenes::sceneWorld_Update(
				deltaTime, renderState);
		}

		renderer->renderFrame(renderState);

		frameDelta = SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();
		deltaTime = frameDelta / 1000.f;
	}
	pthread.join();

	delete renderer;

	finishImgui(system, imguiInstance);

	finishSystemLinux(system);

	return 0;
}