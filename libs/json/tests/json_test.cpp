/**
 * @file json_test.cpp
 * @brief Unit tests for the json library.
 */

#include "json/json.hpp"
#include <cstdlib>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Verify that json::greet() writes "json\n" to stdout.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto* const original = std::cout.rdbuf();
    const std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    json::greet();

    std::cout.rdbuf(original);

    if (capture.str() != "json\n")
    {
        // LCOV_EXCL_START
        std::cerr << R"(FAIL: expected "json\n", got ")" << capture.str() << "\"\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: json::greet()");
    return EXIT_SUCCESS;
}
