// SPDX-License-Identifier: MIT
/**
 * @file json2csv_test.cpp
 * @brief Integration tests for the json2csv binary.
 */

#include "test_support/expect.hpp"
#include "test_support/subprocess.hpp"

#include <string>

/// Path to the json2csv binary, injected by CMake.
#ifndef JSON2CSV_EXECUTABLE
#error "JSON2CSV_EXECUTABLE must be defined"
#endif

auto main() -> int
{
    expect::Suite suite("json2csv");

    // Happy path.
    {
        const std::string json_in = R"([
  {
    "name": "Ada",
    "city": "London, UK"
  },
  {
    "name": "Grace",
    "city": "New York"
  }
])";
        const std::string expected = R"(name,city
Ada,"London, UK"
Grace,New York
)";
        const auto r = subprocess::run(JSON2CSV_EXECUTABLE, {}, {}, json_in);
        suite.check(r.exit_code == 0, "ok/exits_zero");
        suite.check(r.out == expected, "ok/output_matches");
        suite.check(r.err.empty(), "ok/stderr_empty");
    }

    // Malformed input: library throws, main catches, binary prints
    // "json2csv: <message>" to stderr and exits non-zero.
    {
        const std::string bad_json = "not valid json at all";
        const auto r = subprocess::run(JSON2CSV_EXECUTABLE, {}, {}, bad_json);
        suite.check(r.exit_code != 0, "bad_input/exits_nonzero");
        suite.check(r.err.contains("json2csv:"), "bad_input/stderr_prefixed");
    }

    return suite.finish();
}
