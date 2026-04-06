// SPDX-License-Identifier: MIT
/**
 * @file csv2json_test.cpp
 * @brief Integration tests for the csv2json binary.
 */

#include "test_support/expect.hpp"
#include "test_support/subprocess.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

/// Path to the csv2json binary, injected by CMake.
#ifndef CSV2JSON_EXECUTABLE
#error "CSV2JSON_EXECUTABLE must be defined"
#endif

auto main() -> int
{
    try
    {
        expect::Suite suite("csv2json");

        // Happy path.
        {
            const std::string csv_in = R"(name,city
Ada,"London, UK"
Grace,New York
)";
            const std::string expected = R"([
  {
    "name": "Ada",
    "city": "London, UK"
  },
  {
    "name": "Grace",
    "city": "New York"
  }
]
)";
            const auto result = subprocess::run(CSV2JSON_EXECUTABLE, {}, {}, csv_in);
            suite.check(result.exit_code == 0, "ok/exits_zero");
            suite.check(result.out == expected, "ok/output_matches");
            suite.check(result.err.empty(), "ok/stderr_empty");
        }

        // Malformed input: unterminated quote → library throws, main catches,
        // binary prints "csv2json: <message>" to stderr and exits non-zero.
        {
            const std::string bad_csv = R"(name,city
Ada,"London
)";
            const auto result = subprocess::run(CSV2JSON_EXECUTABLE, {}, {}, bad_csv);
            suite.check(result.exit_code != 0, "bad_input/exits_nonzero");
            suite.check(result.err.contains("csv2json:"), "bad_input/stderr_prefixed");
        }

        return suite.finish();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
}
