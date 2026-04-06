// SPDX-License-Identifier: MIT
/**
 * @file csv_test.cpp
 * @brief Unit tests for the csv library.
 */

#include "csv/csv.hpp"
#include "test_support/expect.hpp"

#include <array>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace
{

auto test_parse_simple() -> bool
{
    const auto table = csv::parse("a,b\n1,2\n3,4\n");
    return table.headers == std::vector<std::string>{"a", "b"} && table.rows.size() == 2 &&
           table.rows.at(0) == std::vector<std::string>{"1", "2"} &&
           table.rows.at(1) == std::vector<std::string>{"3", "4"};
}

auto test_parse_quoted() -> bool
{
    const auto table = csv::parse("a,b\n\"he said \"\"hi\"\"\",\"x,y\"\n");
    return table.rows.size() == 1 && table.rows.at(0).at(0) == "he said \"hi\"" && table.rows.at(0).at(1) == "x,y";
}

auto test_parse_empty_field() -> bool
{
    const auto table = csv::parse("a,b,c\n1,,3\n,,\n");
    return table.rows.size() == 2 && table.rows.at(0) == std::vector<std::string>{"1", "", "3"} &&
           table.rows.at(1) == std::vector<std::string>{"", "", ""};
}

auto test_parse_crlf() -> bool
{
    const auto table = csv::parse("a,b\r\n1,2\r\n");
    return table.headers == std::vector<std::string>{"a", "b"} && table.rows.size() == 1 &&
           table.rows.at(0) == std::vector<std::string>{"1", "2"};
}

auto test_parse_no_trailing_newline() -> bool
{
    const auto table = csv::parse("a,b\n1,2");
    return table.rows.size() == 1 && table.rows.at(0) == std::vector<std::string>{"1", "2"};
}

auto test_parse_embedded_newline() -> bool
{
    const auto table = csv::parse("a,b\n\"line1\nline2\",x\n");
    return table.rows.size() == 1 && table.rows.at(0).at(0) == "line1\nline2" && table.rows.at(0).at(1) == "x";
}

auto test_write_quotes_when_needed() -> bool
{
    const core::Table table{.headers = {"a", "b"}, .rows = {{"plain", "has,comma"}, {"has\"quote", "has\nnewline"}}};
    const auto str = csv::write(table);
    return str == "a,b\nplain,\"has,comma\"\n\"has\"\"quote\",\"has\nnewline\"\n";
}

auto test_round_trip() -> bool
{
    // Input is already in the writer's canonical form: empty fields are bare,
    // and quotes are only used where required (containing , or ").
    const std::string original = R"(name,city,note
Ada,"London, UK",pioneer
Grace,New York,
Linus,Helsinki,"said ""hi"""
Anonymous,,no city
)";
    return csv::write(csv::parse(original)) == original;
}

auto test_malformed_wrong_column_count() -> bool
{
    return expect::expect_throws([]() -> void { (void)csv::parse("a,b\n1,2,3\n"); });
}

auto test_malformed_unterminated_quote() -> bool
{
    return expect::expect_throws([]() -> void { (void)csv::parse("a,b\n\"oops,x\n"); });
}

auto test_empty_input_throws() -> bool
{
    return expect::expect_throws([]() -> void { (void)csv::parse(""); });
}

struct TestCase
{
    const char* name;
    bool (*fn)();
};

} // namespace

auto main() -> int
{
    try
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

        expect::Suite suite("csv");
        for (const auto& test_case : cases)
        {
            suite.check(test_case.fn(), test_case.name);
        }
        return suite.finish();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "FATAL: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "FATAL: unknown exception\n";
        return EXIT_FAILURE;
    }
}
