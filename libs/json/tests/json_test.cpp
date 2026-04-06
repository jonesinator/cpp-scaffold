// SPDX-License-Identifier: MIT
/**
 * @file json_test.cpp
 * @brief Unit tests for the json library.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
#include "test_support/expect.hpp"

#include "json/json.hpp"
#include <array>
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
    const std::string expected = R"([
  {
    "a": "1",
    "b": "2"
  }
])";
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

auto test_write_empty_rows() -> bool
{
    const core::Table t{.headers = {"a"}, .rows = {}};
    return json::write(t) == "[]";
}

auto test_rejects_number_value() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":3}])"); });
}

auto test_rejects_null_value() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":null}])"); });
}

auto test_rejects_duplicate_keys() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1","a":"2"}])"); });
}

auto test_rejects_empty_array() -> bool
{
    return expect::expect_throws([] { (void)json::parse("[]"); });
}

auto test_rejects_mismatched_keys() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1","b":"2"},{"a":"3","c":"4"}])"); });
}

auto test_rejects_trailing_content() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1"}]trailing)"); });
}

// --- encode_utf8 range coverage ---

auto test_parse_unicode_1byte() -> bool
{
    // U+0041 'A' — single-byte UTF-8, exercises encode_utf8's cp < 0x80 branch.
    const auto t = json::parse(R"([{"s":"\u0041"}])");
    return t.rows[0][0] == "A";
}

auto test_parse_unicode_3byte() -> bool
{
    // U+2603 ☃ — three-byte UTF-8 (0xE2 0x98 0x83).
    const auto t = json::parse(R"([{"s":"\u2603"}])");
    return t.rows[0][0] == "\xe2\x98\x83";
}

// --- write escape coverage ---

auto test_write_backspace_formfeed_return() -> bool
{
    // Exercises the \b, \f, \r cases in escape_string's switch.
    const core::Table t{.headers = {"s"}, .rows = {{std::string{'\b', '\f', '\r'}}}};
    const std::string out = json::write(t);
    return out.contains("\\b") && out.contains("\\f") && out.contains("\\r");
}

// --- parse edge cases ---

auto test_parse_empty_object() -> bool
{
    // [{}] — exercises the early-return for empty objects in parse_object_pairs.
    const auto t = json::parse("[{}]");
    return t.headers.empty() && t.rows.size() == 1 && t.rows[0].empty();
}

// --- parse rejection coverage ---

auto test_rejects_empty_input() -> bool
{
    return expect::expect_throws([] { (void)json::parse(""); });
}

auto test_rejects_invalid_hex_escape() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\u00GZ"}])"); });
}

auto test_rejects_truncated_hex_escape() -> bool
{
    // Input ends with only 2 hex digits after \u (need 4).
    // Must NOT have closing "}] — those would be parsed as hex chars,
    // triggering "invalid hex digit" instead of "truncated \u escape".
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\u12)"); });
}

auto test_rejects_unpaired_high_surrogate() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\uD800"}])"); });
}

auto test_rejects_high_with_bad_low_surrogate() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\uD800\u0041"}])"); });
}

auto test_rejects_isolated_low_surrogate() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\uDC00"}])"); });
}

auto test_rejects_unterminated_escape() -> bool
{
    return expect::expect_throws([] { (void)json::parse("[{\"a\":\"\\"); });
}

auto test_rejects_invalid_escape_char() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"\z"}])"); });
}

auto test_rejects_control_char_in_string() -> bool
{
    return expect::expect_throws(
        []
        {
            std::string input = R"([{"a":")";
            input += '\x01';
            input += R"("}])";
            (void)json::parse(input);
        });
}

auto test_rejects_unterminated_string() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"hello)"); });
}

auto test_rejects_bad_separator_in_object() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1" "b":"2"}])"); });
}

auto test_rejects_wrong_key_count() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1","b":"2"},{"a":"3"}])"); });
}

auto test_rejects_bad_separator_in_array() -> bool
{
    return expect::expect_throws([] { (void)json::parse(R"([{"a":"1"} x])"); });
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
        {.name = "parse_unicode_1byte", .fn = test_parse_unicode_1byte},
        {.name = "parse_unicode_3byte", .fn = test_parse_unicode_3byte},
        {.name = "write_backspace_formfeed_return", .fn = test_write_backspace_formfeed_return},
        {.name = "write_empty_rows", .fn = test_write_empty_rows},
        {.name = "parse_empty_object", .fn = test_parse_empty_object},
        {.name = "rejects_empty_input", .fn = test_rejects_empty_input},
        {.name = "rejects_invalid_hex_escape", .fn = test_rejects_invalid_hex_escape},
        {.name = "rejects_truncated_hex_escape", .fn = test_rejects_truncated_hex_escape},
        {.name = "rejects_unpaired_high_surrogate", .fn = test_rejects_unpaired_high_surrogate},
        {.name = "rejects_high_with_bad_low_surrogate", .fn = test_rejects_high_with_bad_low_surrogate},
        {.name = "rejects_isolated_low_surrogate", .fn = test_rejects_isolated_low_surrogate},
        {.name = "rejects_unterminated_escape", .fn = test_rejects_unterminated_escape},
        {.name = "rejects_invalid_escape_char", .fn = test_rejects_invalid_escape_char},
        {.name = "rejects_control_char_in_string", .fn = test_rejects_control_char_in_string},
        {.name = "rejects_unterminated_string", .fn = test_rejects_unterminated_string},
        {.name = "rejects_bad_separator_in_object", .fn = test_rejects_bad_separator_in_object},
        {.name = "rejects_wrong_key_count", .fn = test_rejects_wrong_key_count},
        {.name = "rejects_bad_separator_in_array", .fn = test_rejects_bad_separator_in_array},
    });

    expect::Suite suite("json");
    for (const auto& tc : cases)
    {
        suite.check(tc.fn(), tc.name);
    }
    return suite.finish();
}
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
