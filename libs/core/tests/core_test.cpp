// SPDX-License-Identifier: MIT
/**
 * @file core_test.cpp
 * @brief Unit tests for the core library.
 */

#include "core/core.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Verify that core::println() writes the message followed by a newline.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto* const original = std::cout.rdbuf();
    const std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    core::println("hello");

    std::cout.rdbuf(original);

    if (capture.str() != "hello\n")
    {
        // LCOV_EXCL_START
        std::cerr << R"(FAIL: expected "hello\n", got ")" << capture.str() << "\"\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: core::println()");
    return EXIT_SUCCESS;
}
