#include "init.hpp"
using namespace vblck;

std::shared_ptr<spdlog::logger> logger;

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

    System system{};

    system = initSystem("Vulkan App", 640, 480);

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

    finishSystem(system);

    return 0;
}