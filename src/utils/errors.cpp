#include "errors.hpp"
#include <exception>
#include "logger.hpp"
#include <iostream>

namespace vblck {
    void stopProgram(const char *expr, const std::source_location loc)
    {
        std:: cerr << "\nAssertion failed!\n"
              << "  Expression : " << expr << "\n"
              << "  Function   : " << loc.function_name() << "\n"
              << "  File       : " << loc.file_name() << "\n"
              << "  Line       : " << loc.line() << "\n";
        exit(-1);
    }
}