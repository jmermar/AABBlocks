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

	{

		render::Renderer renderer(system.instance,
								  system.chosenGPU,
								  system.device,
								  system.surface,
								  system.graphicsQueue,
								  system.graphicsQueueFamily,
								  640,
								  480);

		bool running = true;
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
					renderer.recreateSwapchain(w, h);
				}
				ImGui_ImplSDL3_ProcessEvent(&e);
			}

			ImGui_ImplVulkan_NewFrame();
			ImGui_ImplSDL3_NewFrame();
			ImGui::NewFrame();

			ImGui::ShowDemoWindow();

			ImGui::Render();

			renderer.renderFrame();
		}
	}

	finishImgui(system, imguiInstance);

	finishSystemLinux(system);

	return 0;
}