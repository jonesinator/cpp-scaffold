// SPDX-License-Identifier: MIT
/**
 * @file json2csv_test.cpp
 * @brief Integration tests for the json2csv binary.
 */

#include "test_support/subprocess.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <string>

/// Path to the json2csv binary, injected by CMake.
#ifndef JSON2CSV_EXECUTABLE
#error "JSON2CSV_EXECUTABLE must be defined"
#endif

/**
 * @brief Pipe sample JSON into json2csv and verify its CSV output.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    const std::string json_in = "[\n"
                                "  {\n"
                                "    \"name\": \"Ada\",\n"
                                "    \"city\": \"London, UK\"\n"
                                "  },\n"
                                "  {\n"
                                "    \"name\": \"Grace\",\n"
                                "    \"city\": \"New York\"\n"
                                "  }\n"
                                "]";
    const std::string expected = "name,city\n"
                                 "Ada,\"London, UK\"\n"
                                 "Grace,New York\n";

    const auto result = subprocess::run(JSON2CSV_EXECUTABLE, {}, {}, json_in);

    if (result.exit_code != 0)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "FAIL: json2csv exited with code {} (stderr: {})", result.exit_code, result.err);
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

    std::println("PASS: json2csv");
    return EXIT_SUCCESS;
}
