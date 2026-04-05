// SPDX-License-Identifier: MIT
/**
 * @file json_test.cpp
 * @brief Unit tests for the json library.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
#include "json/json.hpp"
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
    const auto t = json::parse(R"([{"a":"1","b":"2"},{"a":"3","b":"4"}])");
    return t.headers == std::vector<std::string>{"a", "b"} && t.rows.size() == 2 &&
           t.rows[0] == std::vector<std::string>{"1", "2"} && t.rows[1] == std::vector<std::string>{"3", "4"};
}

auto test_parse_whitespace() -> bool
{
    const auto t = json::parse("[\n  { \"a\" : \"1\" , \"b\":\"2\"  }\n]");
    return t.rows.size() == 1 && t.rows[0] == std::vector<std::string>{"1", "2"};
}

auto test_parse_escapes() -> bool
{
    const auto t = json::parse(R"([{"s":"a\nb\tc\"d\\e\/f\b\r\f"}])");
    return t.rows[0][0] == std::string{"a\nb\tc\"d\\e/f\b\r\f"};
}

auto test_parse_unicode_bmp() -> bool
{
    const auto t = json::parse(R"([{"s":"\u00e9"}])");
    // U+00E9 "é" — two bytes in UTF-8.
    return t.rows[0][0] == "\xc3\xa9";
}

auto test_parse_unicode_surrogate_pair() -> bool
{
    // U+1F600 GRINNING FACE — four bytes in UTF-8.
    const auto t = json::parse(R"([{"s":"\uD83D\uDE00"}])");
    return t.rows[0][0] == "\xf0\x9f\x98\x80";
}

auto test_write_pretty() -> bool
{
    const core::Table t{.headers = {"a", "b"}, .rows = {{"1", "2"}}};
    const std::string expected = "[\n"
                                 "  {\n"
                                 "    \"a\": \"1\",\n"
                                 "    \"b\": \"2\"\n"
                                 "  }\n"
                                 "]";
    return json::write(t) == expected;
}

auto test_write_escapes() -> bool
{
    const core::Table t{.headers = {"s"}, .rows = {{"a\"b\\c\nd\te"}}};
    const std::string out = json::write(t);
    return out.contains(R"("s": "a\"b\\c\nd\te")");
}

auto test_write_control_char() -> bool
{
    const core::Table t{.headers = {"s"}, .rows = {{std::string{'\x01'}}}};
    const std::string out = json::write(t);
    return out.contains(R"("s": "\u0001")");
}

auto test_round_trip() -> bool
{
    const core::Table original{.headers = {"name", "city"}, .rows = {{"Ada", "London, UK"}, {"Linus", "\"hi\""}}};
    return json::parse(json::write(original)).rows == original.rows;
}

auto test_rejects_number_value() -> bool
{
    try
    {
        (void)json::parse(R"([{"a":3}])");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_rejects_null_value() -> bool
{
    try
    {
        (void)json::parse(R"([{"a":null}])");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_rejects_duplicate_keys() -> bool
{
    try
    {
        (void)json::parse(R"([{"a":"1","a":"2"}])");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_rejects_empty_array() -> bool
{
    try
    {
        (void)json::parse("[]");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_rejects_mismatched_keys() -> bool
{
    try
    {
        (void)json::parse(R"([{"a":"1","b":"2"},{"a":"3","c":"4"}])");
        return false;
    }
    catch (const std::exception&)
    {
        return true;
    }
}

auto test_rejects_trailing_content() -> bool
{
    try
    {
        (void)json::parse(R"([{"a":"1"}]trailing)");
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
        {.name = "parse_whitespace", .fn = test_parse_whitespace},
        {.name = "parse_escapes", .fn = test_parse_escapes},
        {.name = "parse_unicode_bmp", .fn = test_parse_unicode_bmp},
        {.name = "parse_unicode_surrogate_pair", .fn = test_parse_unicode_surrogate_pair},
        {.name = "write_pretty", .fn = test_write_pretty},
        {.name = "write_escapes", .fn = test_write_escapes},
        {.name = "write_control_char", .fn = test_write_control_char},
        {.name = "round_trip", .fn = test_round_trip},
        {.name = "rejects_number_value", .fn = test_rejects_number_value},
        {.name = "rejects_null_value", .fn = test_rejects_null_value},
        {.name = "rejects_duplicate_keys", .fn = test_rejects_duplicate_keys},
        {.name = "rejects_empty_array", .fn = test_rejects_empty_array},
        {.name = "rejects_mismatched_keys", .fn = test_rejects_mismatched_keys},
        {.name = "rejects_trailing_content", .fn = test_rejects_trailing_content},
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
        std::println(std::cerr, "{} json test(s) failed", failures);
        return EXIT_FAILURE;
        // LCOV_EXCL_STOP
    }
    std::println("PASS: all json tests");
    return EXIT_SUCCESS;
}
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
