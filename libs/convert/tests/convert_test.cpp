// SPDX-License-Identifier: MIT
/**
 * @file convert_test.cpp
 * @brief Unit tests for the convert library.
 */

#include "convert/convert.hpp"

#include <cstdlib>
#include <iostream>
#include <print>
#include <string>

namespace
{

auto test_csv_round_trip_via_json() -> bool
{
    const std::string original = "name,city,note\n"
                                 "Ada,\"London, UK\",pioneer\n"
                                 "Grace,New York,\n"
                                 "Linus,Helsinki,\"said \"\"hi\"\"\"\n"
                                 "Anonymous,,no city\n";
    return convert::json_to_csv(convert::csv_to_json(original)) == original;
}

auto test_json_round_trip_via_csv() -> bool
{
    const std::string original = "[\n"
                                 "  {\n"
                                 "    \"name\": \"Ada\",\n"
                                 "    \"city\": \"London, UK\"\n"
                                 "  },\n"
                                 "  {\n"
                                 "    \"name\": \"Linus\",\n"
                                 "    \"city\": \"Helsinki\"\n"
                                 "  }\n"
                                 "]";
    return convert::csv_to_json(convert::json_to_csv(original)) == original;
}

} // namespace

auto main() -> int
{
    int failures = 0;
    if (!test_csv_round_trip_via_json())
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "FAIL: csv_round_trip_via_json");
        ++failures;
        // LCOV_EXCL_STOP
    }
    if (!test_json_round_trip_via_csv())
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "FAIL: json_round_trip_via_csv");
        ++failures;
        // LCOV_EXCL_STOP
    }
    if (failures != 0)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "{} convert test(s) failed", failures);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    std::println("PASS: all convert tests");
    return EXIT_SUCCESS;
}
