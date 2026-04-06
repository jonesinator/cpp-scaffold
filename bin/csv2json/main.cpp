// SPDX-License-Identifier: MIT
/**
 * @file main.cpp
 * @brief Entry point for the csv2json binary.
 *
 * Reads CSV from stdin, writes a pretty-printed JSON array of objects to
 * stdout. Uses the convert library, which transitively depends on csv,
 * json, and core.
 */

#include "convert/convert.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Read stdin to EOF, convert CSV to JSON, write to stdout.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE with a stderr message on error.
 */
auto main() -> int
{
    try
    {
        std::ostringstream buf{};
        buf << std::cin.rdbuf();
        std::cout << convert::csv_to_json(buf.str()) << '\n';
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        // NOLINTNEXTLINE(misc-include-cleaner) clang-tidy 19 can't map std::println to <print>
        std::println(std::cerr, "csv2json: {}", e.what());
        return EXIT_FAILURE;
    }
}
