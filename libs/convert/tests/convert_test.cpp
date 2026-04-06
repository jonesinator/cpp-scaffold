// SPDX-License-Identifier: MIT
/**
 * @file convert_test.cpp
 * @brief Unit tests for the convert library.
 */

#include "convert/convert.hpp"
#include "test_support/expect.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>

namespace
{

auto test_csv_round_trip_via_json() -> bool
{
    const std::string original = R"(name,city,note
Ada,"London, UK",pioneer
Grace,New York,
Linus,Helsinki,"said ""hi"""
Anonymous,,no city
)";
    return convert::json_to_csv(convert::csv_to_json(original)) == original;
}

auto test_json_round_trip_via_csv() -> bool
{
    const std::string original = R"([
  {
    "name": "Ada",
    "city": "London, UK"
  },
  {
    "name": "Linus",
    "city": "Helsinki"
  }
])";
    return convert::csv_to_json(convert::json_to_csv(original)) == original;
}

} // namespace

auto main() -> int
{
    try
    {
        expect::Suite suite("convert");
        suite.check(test_csv_round_trip_via_json(), "csv_round_trip_via_json");
        suite.check(test_json_round_trip_via_csv(), "json_round_trip_via_csv");
        return suite.finish();
    }
    // LCOV_EXCL_START
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    // LCOV_EXCL_STOP
}
