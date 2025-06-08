#include "errors.hpp"
#include "logger.hpp"
#include <exception>
#include <iostream>

namespace vblck
{
void stopProgram(const char* expr, const std::source_location loc)
{
	std::cerr << "\nAssertion failed!\n"
			  << "  Expression : " << expr << "\n"
			  << "  Function   : " << loc.function_name() << "\n"
			  << "  File       : " << loc.file_name() << "\n"
			  << "  Line       : " << loc.line() << "\n";
	std::abort();
}
} // namespace vblck