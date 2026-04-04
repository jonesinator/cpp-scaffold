/**
 * @file json2csv_test.cpp
 * @brief Integration tests for the json2csv binary.
 */

#include "test_support/subprocess.hpp"

#include <cstdlib>
#include <iostream>
#include <print>

/// Path to the json2csv binary, injected by CMake.
#ifndef JSON2CSV_EXECUTABLE
#error "JSON2CSV_EXECUTABLE must be defined"
#endif

/**
 * @brief Verify that json2csv prints "csv\njson\n" and exits 0.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto result = subprocess::run(JSON2CSV_EXECUTABLE);

    if (result.exit_code != 0)
    {
        // LCOV_EXCL_START
        std::cerr << "FAIL: json2csv exited with code " << result.exit_code << "\n";
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

    std::println("PASS: json2csv");
    return EXIT_SUCCESS;
}
