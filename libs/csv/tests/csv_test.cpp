/**
 * @file csv_test.cpp
 * @brief Unit tests for the csv library.
 */

#include "csv/csv.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <sstream>

/**
 * @brief Verify that csv::greet() writes "csv\n" to stdout.
 * @return EXIT_SUCCESS on pass, EXIT_FAILURE on fail.
 */
auto main() -> int
{
    auto* const original = std::cout.rdbuf();
    const std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());

    csv::greet();

    std::cout.rdbuf(original);

    if (capture.str() != "csv\n")
    {
        // LCOV_EXCL_START
        std::cerr << R"(FAIL: expected "csv\n", got ")" << capture.str() << "\"\n";
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }

    std::println("PASS: csv::greet()");
    return EXIT_SUCCESS;
}
