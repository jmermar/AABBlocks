#include "init.hpp"
#include "rendering/renderer.hpp"
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

	system = initSystemLinux("Vulkan App", 640, 480);

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
			}

			renderer.renderFrame();
		}
	}

	finishSystemLinux(system);

	return 0;
}