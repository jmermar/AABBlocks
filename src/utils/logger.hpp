#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#define LOG_INFO(...) vblck::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...) vblck::getLogger()->warn(__VA_ARGS__)
#define LOG_ERR(...) vblck::getLogger()->error(__VA_ARGS__)

namespace vblck
{
    std::shared_ptr<spdlog::logger> &getLogger();
}
