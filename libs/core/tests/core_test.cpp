// SPDX-License-Identifier: MIT
/**
 * @file core_test.cpp
 * @brief Unit tests for the core library.
 */

#include "core/core.hpp"
#include "test_support/expect.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>

/**
 * @brief Verify that core::println() writes the message followed by a newline.
 */
auto main() -> int
{
    try
    {
        auto* const original = std::cout.rdbuf();
        const std::ostringstream capture;
        std::cout.rdbuf(capture.rdbuf());

        core::println("hello");

        std::cout.rdbuf(original);

        expect::Suite suite("core");
        suite.check(capture.str() == "hello\n", "println/appends_newline");
        return suite.finish();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
