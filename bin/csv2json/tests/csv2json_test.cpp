/**
 * @file csv2json_test.cpp
 * @brief Integration tests for the csv2json binary.
 */

#include "test_support/subprocess.hpp"

#include <cstdlib>
#include <iostream>
#include <print>

/// Path to the csv2json binary, injected by CMake.
#ifndef CSV2JSON_EXECUTABLE
#error "CSV2JSON_EXECUTABLE must be defined"
#endif

/**
 * @brief Verify that csv2json prints "csv\njson\n" and exits 0.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto result = subprocess::run(CSV2JSON_EXECUTABLE);

    if (result.exit_code != 0)
    {
        // LCOV_EXCL_START
        std::cerr << "FAIL: csv2json exited with code " << result.exit_code << "\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    if (result.out != "csv\njson\n")
    {
        // LCOV_EXCL_START
        std::cerr << R"(FAIL: expected "csv\njson\n", got ")" << result.out << "\"\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: csv2json");
    return EXIT_SUCCESS;
}
