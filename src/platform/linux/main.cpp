#include "init.hpp"
#include "init_imgui.hpp"
#include "rendering/renderer.hpp"
#include "world/world.hpp"
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_vulkan.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/matrix.hpp>

using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

namespace vblck
{

std::shared_ptr<spdlog::logger>& getLogger()
{
	return logger;
}
} // namespace vblck

void rotateY(render::Camera& cam, float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle), glm::vec3(0, 1, 0) * glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	cam.forward = rotation * cam.forward * rotationC;
}

void rotateX(render::Camera& cam, float degrees)
{
	auto angle = glm::radians(degrees / 2.f);
	glm::quat rotation(glm::cos(angle),
					   glm::normalize(glm::cross(cam.forward, glm::vec3(0, 1, 0))) *
						   glm::sin(angle));
	glm::quat rotationC = glm::conjugate(rotation);

	cam.forward = rotation * cam.forward * rotationC;
}

int main(int argc, char** argv)
{
	logger = spdlog::stdout_color_mt("VKP");

	System system{};

	world::World* world = new world::World;
	world->generateWorld();

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
	renderState.camera.fov = 45;
	renderState.camera.aspect = 1920.f / 1080.f;
	renderState.camera.position = glm::vec3(50, 20, 50);
	renderState.camera.forward = glm::vec3(0, 0, 1);
	uint64_t frameDelta = 0;
	float deltaTime = 0;

	for(auto& ys : world->chunks)
	{
		for(auto& xs : ys)
		{
			for(auto& chunk : xs)
			{
				auto data = chunk.generateChunkData();
				if(data.size() > 0)
				{
					renderer->worldRenderer->chunkRenderer.loadChunk(chunk.position, data);
				}
			}
		}
	}
	float moveSpeed = 5;
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
				renderState.camera.aspect = (float)w / (float)h;
			}
			ImGui_ImplSDL3_ProcessEvent(&e);
		}

		auto* ks = SDL_GetKeyboardState(NULL);

		glm::vec3 input(0);
		if(ks[SDL_SCANCODE_W])
			input.z += 1;
		if(ks[SDL_SCANCODE_S])
			input.z -= 1;
		if(ks[SDL_SCANCODE_A])
			input.x += 1;
		if(ks[SDL_SCANCODE_D])
			input.x -= 1;
		if(ks[SDL_SCANCODE_SPACE])
			input.y += 1;
		if(ks[SDL_SCANCODE_LSHIFT])
			input.y -= 1;

		if(ks[SDL_SCANCODE_LEFT])
			rotateY(renderState.camera, -45 * deltaTime);
		if(ks[SDL_SCANCODE_RIGHT])
			rotateY(renderState.camera, 45 * deltaTime);

		if(ks[SDL_SCANCODE_UP])
			rotateX(renderState.camera, 45 * deltaTime);
		if(ks[SDL_SCANCODE_DOWN])
			rotateX(renderState.camera, -45 * deltaTime);

		renderState.camera.position += renderState.camera.forward * input.z * deltaTime * moveSpeed;
		renderState.camera.position += glm::cross(renderState.camera.forward, glm::vec3(0, 1, 0)) *
									   input.x * deltaTime * moveSpeed;
		renderState.camera.position.y += input.y * deltaTime * moveSpeed;

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Text("FPS: %f", 1000.f / frameDelta);

		ImGui::Render();

		renderer->renderFrame(renderState);

		frameDelta = SDL_GetTicks() - ticks;
		ticks = SDL_GetTicks();
		deltaTime = frameDelta / 1000.f;
	}
	delete renderer;

	finishImgui(system, imguiInstance);

	finishSystemLinux(system);

	delete world;

	return 0;
}