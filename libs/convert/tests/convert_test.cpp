/**
 * @file convert_test.cpp
 * @brief Unit tests for the convert library.
 */

#include "convert/convert.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Verify that convert::greet_both() writes "csv\njson\n" to stdout.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto* const original = std::cout.rdbuf();
    const std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    convert::greet_both();

    std::cout.rdbuf(original);

    if (capture.str() != "csv\njson\n")
    {
        // LCOV_EXCL_START
        std::cerr << R"(FAIL: expected "csv\njson\n", got ")" << capture.str() << "\"\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: convert::greet_both()");
    return EXIT_SUCCESS;
}
