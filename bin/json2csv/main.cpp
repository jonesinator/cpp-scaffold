// SPDX-License-Identifier: MIT
/**
 * @file main.cpp
 * @brief Entry point for the json2csv binary.
 *
 * Reads a JSON array of objects from stdin, writes RFC-4180 CSV to stdout.
 * Uses the convert library, which transitively depends on csv, json, and core.
 */

#include "convert/convert.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Read stdin to EOF, convert JSON to CSV, write to stdout.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE with a stderr message on error.
 */
auto main() -> int
{
    try
    {
        std::ostringstream buf{};
        buf << std::cin.rdbuf();
        std::cout << convert::json_to_csv(buf.str());
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "json2csv: {}", e.what());
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
}
