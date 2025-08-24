#pragma once
#include "world.hpp"

namespace vblck
{
namespace world
{
namespace persistence
{
void saveWorld(const std::string& worldName);
void loadWorld(const std::string& worldName);

bool exists(const ::std::string& worldName);
} // namespace persistence
} // namespace world
} // namespace vblck