#include <stdlib.h>
#include <SDL3/SDL.h>
#include "utils/logger.hpp"
#include "utils/errors.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "types.hpp"
#include <glm/glm.hpp>
#include "vk/vk_raii.hpp"
using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

SDL_Window *window = 0;

namespace vblck
{

    std::shared_ptr<spdlog::logger> &getLogger()
    {
        return logger;
    }
}

int main(int argc, char **argv)
{
    logger = spdlog::stdout_color_mt("VKP");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    LOG_INFO("SDL Initialized");

    TRY(window = SDL_CreateWindow("AABBLOCKS", 1280, 720, SDL_WINDOW_VULKAN));

    LOG_INFO("Window created");

    bool running = true;

    while (running)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    LOG_INFO("App finished");

    return 0;
}