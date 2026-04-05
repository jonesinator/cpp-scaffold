// SPDX-License-Identifier: MIT
/**
 * @file csv2json_test.cpp
 * @brief Integration tests for the csv2json binary.
 */

#include "test_support/subprocess.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <string>

/// Path to the csv2json binary, injected by CMake.
#ifndef CSV2JSON_EXECUTABLE
#error "CSV2JSON_EXECUTABLE must be defined"
#endif

/**
 * @brief Pipe sample CSV into csv2json and verify its JSON output.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    const std::string csv_in = "name,city\n"
                               "Ada,\"London, UK\"\n"
                               "Grace,New York\n";
    const std::string expected = "[\n"
                                 "  {\n"
                                 "    \"name\": \"Ada\",\n"
                                 "    \"city\": \"London, UK\"\n"
                                 "  },\n"
                                 "  {\n"
                                 "    \"name\": \"Grace\",\n"
                                 "    \"city\": \"New York\"\n"
                                 "  }\n"
                                 "]\n";

    const auto result = subprocess::run(CSV2JSON_EXECUTABLE, {}, {}, csv_in);

    if (result.exit_code != 0)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "FAIL: csv2json exited with code {} (stderr: {})", result.exit_code, result.err);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    if (result.out != expected)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "FAIL: expected:\n{}\ngot:\n{}", expected, result.out);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: csv2json");
    return EXIT_SUCCESS;
}
