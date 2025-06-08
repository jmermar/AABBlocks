#pragma once
#include <cstdint>
#include <source_location>

#define TRY(expr) ((expr) ? (void)0 : vblck::stopProgram(#expr))

#define VKTRY(expr) TRY(expr == VK_SUCCESS)

namespace vblck
{
void stopProgram(const char* expr,
				 const std::source_location loc = std::source_location::current());
}