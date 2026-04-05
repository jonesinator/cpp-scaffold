// SPDX-License-Identifier: MIT
/**
 * @file csv_test.cpp
 * @brief Unit tests for the csv library.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
#include "csv/csv.hpp"

#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <print>
#include <string>
#include <vector>

namespace
{

auto test_parse_simple() -> bool
{
    const auto t = csv::parse("a,b\n1,2\n3,4\n");
    return t.headers == std::vector<std::string>{"a", "b"} && t.rows.size() == 2 &&
           t.rows[0] == std::vector<std::string>{"1", "2"} && t.rows[1] == std::vector<std::string>{"3", "4"};
}

auto test_parse_quoted() -> bool
{
    const auto t = csv::parse("a,b\n\"he said \"\"hi\"\"\",\"x,y\"\n");
    return t.rows.size() == 1 && t.rows[0][0] == "he said \"hi\"" && t.rows[0][1] == "x,y";
}

auto test_parse_empty_field() -> bool
{
    const auto t = csv::parse("a,b,c\n1,,3\n,,\n");
    return t.rows.size() == 2 && t.rows[0] == std::vector<std::string>{"1", "", "3"} &&
           t.rows[1] == std::vector<std::string>{"", "", ""};
}

auto test_parse_crlf() -> bool
{
    const auto t = csv::parse("a,b\r\n1,2\r\n");
    return t.headers == std::vector<std::string>{"a", "b"} && t.rows.size() == 1 &&
           t.rows[0] == std::vector<std::string>{"1", "2"};
}

auto test_parse_no_trailing_newline() -> bool
{
    const auto t = csv::parse("a,b\n1,2");
    return t.rows.size() == 1 && t.rows[0] == std::vector<std::string>{"1", "2"};
}

auto test_parse_embedded_newline() -> bool
{
    const auto t = csv::parse("a,b\n\"line1\nline2\",x\n");
    return t.rows.size() == 1 && t.rows[0][0] == "line1\nline2" && t.rows[0][1] == "x";
}

auto test_write_quotes_when_needed() -> bool
{
    const core::Table t{.headers = {"a", "b"}, .rows = {{"plain", "has,comma"}, {"has\"quote", "has\nnewline"}}};
    const auto s = csv::write(t);
    return s == "a,b\nplain,\"has,comma\"\n\"has\"\"quote\",\"has\nnewline\"\n";
}

auto test_round_trip() -> bool
{
    // Input is already in the writer's canonical form: empty fields are bare,
    // and quotes are only used where required (containing , or ").
    const std::string original = "name,city,note\n"
                                 "Ada,\"London, UK\",pioneer\n"
                                 "Grace,New York,\n"
                                 "Linus,Helsinki,\"said \"\"hi\"\"\"\n"
                                 "Anonymous,,no city\n";
    return csv::write(csv::parse(original)) == original;
}

auto test_malformed_wrong_column_count() -> bool
{
    try
    {
        (void)csv::parse("a,b\n1,2,3\n");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_malformed_unterminated_quote() -> bool
{
    try
    {
        (void)csv::parse("a,b\n\"oops,x\n");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_empty_input_throws() -> bool
{
    try
    {
        (void)csv::parse("");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

struct TestCase
{
    const char* name;
    bool (*fn)();
};

} // namespace

auto main() -> int
{
    const std::array cases = std::to_array<TestCase>({
        {.name = "parse_simple", .fn = test_parse_simple},
        {.name = "parse_quoted", .fn = test_parse_quoted},
        {.name = "parse_empty_field", .fn = test_parse_empty_field},
        {.name = "parse_crlf", .fn = test_parse_crlf},
        {.name = "parse_no_trailing_newline", .fn = test_parse_no_trailing_newline},
        {.name = "parse_embedded_newline", .fn = test_parse_embedded_newline},
        {.name = "write_quotes_when_needed", .fn = test_write_quotes_when_needed},
        {.name = "round_trip", .fn = test_round_trip},
        {.name = "malformed_wrong_column_count", .fn = test_malformed_wrong_column_count},
        {.name = "malformed_unterminated_quote", .fn = test_malformed_unterminated_quote},
        {.name = "empty_input_throws", .fn = test_empty_input_throws},
    });

    int failures = 0;
    for (const auto& tc : cases)
    {
        if (!tc.fn())
        {
            // LCOV_EXCL_START
            std::println(std::cerr, "FAIL: {}", tc.name);
            ++failures;
            // LCOV_EXCL_STOP
        }
    }

    if (failures != 0)
    {
        // LCOV_EXCL_START
        std::println(std::cerr, "{} csv test(s) failed", failures);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    std::println("PASS: all csv tests");
    return EXIT_SUCCESS;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
