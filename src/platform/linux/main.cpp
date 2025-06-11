#include "init.hpp"
#include "init_imgui.hpp"
#include "rendering/renderer.hpp"
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>

using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

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

	System system{};

	system = initSystemLinux("Vulkan App", 1920, 1080);
	auto imguiInstance = initImgui(system);

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
	renderState.camera.position = glm::vec3(-5, 5, -5);
	renderState.camera.forward = glm::vec3(5, -5, 5);
	uint64_t frameDelta = 0;
	float acc = 0;
	while(running)
	{
		SDL_Event e;
		while(SDL_PollEvent(&e))
		{
			switch(e.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;
			case SDL_EVENT_WINDOW_RESIZED:
				auto w = e.window.data1;
				auto h = e.window.data2;
				renderer->recreateSwapchain(w, h);
			}
			ImGui_ImplSDL3_ProcessEvent(&e);
		}
		acc += frameDelta / 1000.f;

		renderState.camera.position.x = glm::cos(acc) * 5;
		renderState.camera.position.z = glm::sin(acc) * 5;
		renderState.camera.position.y = 0;
		renderState.camera.forward = -renderState.camera.position;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Text("FPS: %f", 1000.f / frameDelta);

		ImGui::Render();

		renderer->renderFrame(renderState);

		frameDelta = SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();
	}
	delete renderer;

	finishImgui(system, imguiInstance);

	finishSystemLinux(system);

	return 0;
}