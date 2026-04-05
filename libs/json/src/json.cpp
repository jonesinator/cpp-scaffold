// SPDX-License-Identifier: MIT
/**
 * @file json.cpp
 * @brief Implementation of the json library.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,misc-non-private-member-variables-in-classes)
#include "json/json.hpp"

#include <cstddef>
#include <cstdint>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace json
{

namespace
{

// UTF-8 encoding thresholds.
constexpr std::uint32_t utf8_1byte_limit = 0x80U;
constexpr std::uint32_t utf8_2byte_limit = 0x800U;
constexpr std::uint32_t utf8_3byte_limit = 0x10000U;

// Continuation-byte mask / prefix.
constexpr std::uint32_t cont_mask = 0x3FU;
constexpr std::uint32_t cont_prefix = 0x80U;
constexpr std::uint32_t lead2_prefix = 0xC0U;
constexpr std::uint32_t lead3_prefix = 0xE0U;
constexpr std::uint32_t lead4_prefix = 0xF0U;

// UTF-8 continuation-byte shifts.
constexpr unsigned utf8_shift_1 = 6U;
constexpr unsigned utf8_shift_2 = 12U;
constexpr unsigned utf8_shift_3 = 18U;

// UTF-16 surrogate ranges.
constexpr std::uint32_t surrogate_high_min = 0xD800U;
constexpr std::uint32_t surrogate_high_max = 0xDBFFU;
constexpr std::uint32_t surrogate_low_min = 0xDC00U;
constexpr std::uint32_t surrogate_low_max = 0xDFFFU;
constexpr std::uint32_t surrogate_base = 0x10000U;
constexpr unsigned surrogate_shift = 10U;

// Hex parsing.
constexpr std::uint32_t hex_base = 16U;
constexpr int hex_digits_in_escape = 4;
constexpr std::uint32_t hex_letter_offset = 10U;

// First non-control ASCII code point; any byte below this must be escaped.
constexpr unsigned char control_char_limit = 0x20U;

/**
 * @brief Append @p cp as a UTF-8 byte sequence to @p out.
 * @param cp  Unicode code point (assumed valid, not a surrogate).
 * @param out Output buffer.
 */
void encode_utf8(std::uint32_t cp, std::string& out)
{
    if (cp < utf8_1byte_limit)
    {
        out.push_back(static_cast<char>(cp));
    }
    else if (cp < utf8_2byte_limit)
    {
        out.push_back(static_cast<char>(lead2_prefix | (cp >> utf8_shift_1)));
        out.push_back(static_cast<char>(cont_prefix | (cp & cont_mask)));
    }
    else if (cp < utf8_3byte_limit)
    {
        out.push_back(static_cast<char>(lead3_prefix | (cp >> utf8_shift_2)));
        out.push_back(static_cast<char>(cont_prefix | ((cp >> utf8_shift_1) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | (cp & cont_mask)));
    }
    else
    {
        out.push_back(static_cast<char>(lead4_prefix | (cp >> utf8_shift_3)));
        out.push_back(static_cast<char>(cont_prefix | ((cp >> utf8_shift_2) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | ((cp >> utf8_shift_1) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | (cp & cont_mask)));
    }
}

/// Parser state (value type; members are part of the parsing cursor).
struct Parser
{
    std::string_view src;
    std::size_t pos = 0;

    [[noreturn]] void fail(std::string_view msg) const
    {
        throw std::runtime_error(std::format("json: {} at offset {}", msg, pos));
    }

    void skip_ws()
    {
        while (pos < src.size())
        {
            const char c = src[pos];
            if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            {
                break;
            }
            ++pos;
        }
    }

    auto peek() -> char
    {
        skip_ws();
        if (pos >= src.size())
        {
            fail("unexpected end of input");
        }
        return src[pos];
    }

    void expect(char c)
    {
        const char got = peek();
        if (got != c)
        {
            fail(std::format("expected '{}' but got '{}'", c, got));
        }
        ++pos;
    }

    [[nodiscard]] auto hex_value(char c) const -> std::uint32_t
    {
        if (c >= '0' && c <= '9')
        {
            return static_cast<std::uint32_t>(c - '0');
        }
        if (c >= 'a' && c <= 'f')
        {
            return static_cast<std::uint32_t>(c - 'a') + hex_letter_offset;
        }
        if (c >= 'A' && c <= 'F')
        {
            return static_cast<std::uint32_t>(c - 'A') + hex_letter_offset;
        }
        fail("invalid hex digit in \\u escape");
    }

    auto parse_hex4() -> std::uint32_t
    {
        if (pos + hex_digits_in_escape > src.size())
        {
            fail("truncated \\u escape");
        }
        std::uint32_t result = 0;
        for (int i = 0; i < hex_digits_in_escape; ++i)
        {
            result = (result * hex_base) + hex_value(src[pos]);
            ++pos;
        }
        return result;
    }

    void parse_unicode_escape(std::string& out)
    {
        std::uint32_t cp = parse_hex4();
        if (cp >= surrogate_high_min && cp <= surrogate_high_max)
        {
            if (pos + 1 >= src.size() || src[pos] != '\\' || src[pos + 1] != 'u')
            {
                fail("unpaired high surrogate in \\u escape");
            }
            pos += 2;
            const std::uint32_t low = parse_hex4();
            if (low < surrogate_low_min || low > surrogate_low_max)
            {
                fail("invalid low surrogate in \\u escape");
            }
            cp = surrogate_base + ((cp - surrogate_high_min) << surrogate_shift) + (low - surrogate_low_min);
        }
        else if (cp >= surrogate_low_min && cp <= surrogate_low_max)
        {
            fail("unexpected low surrogate in \\u escape");
        }
        encode_utf8(cp, out);
    }

    void parse_escape(std::string& out)
    {
        if (pos >= src.size())
        {
            fail("unterminated escape sequence");
        }
        const char c = src[pos];
        ++pos;
        switch (c)
        {
        case '"':
            out.push_back('"');
            break;
        case '\\':
            out.push_back('\\');
            break;
        case '/':
            out.push_back('/');
            break;
        case 'b':
            out.push_back('\b');
            break;
        case 'f':
            out.push_back('\f');
            break;
        case 'n':
            out.push_back('\n');
            break;
        case 'r':
            out.push_back('\r');
            break;
        case 't':
            out.push_back('\t');
            break;
        case 'u':
            parse_unicode_escape(out);
            break;
        default:
            fail(std::format("invalid escape character '\\{}'", c));
        }
    }

    auto parse_string() -> std::string
    {
        expect('"');
        std::string out;
        while (pos < src.size())
        {
            const char c = src[pos];
            if (c == '"')
            {
                ++pos;
                return out;
            }
            if (c == '\\')
            {
                ++pos;
                parse_escape(out);
            }
            else if (static_cast<unsigned char>(c) < control_char_limit)
            {
                fail("unescaped control character in string");
            }
            else
            {
                out.push_back(c);
                ++pos;
            }
        }
        fail("unterminated string");
    }

    /// Parse one object's key/value pairs in source order.
    auto parse_object_pairs() -> std::vector<std::pair<std::string, std::string>>
    {
        expect('{');
        std::vector<std::pair<std::string, std::string>> pairs;
        std::unordered_set<std::string> seen;
        if (peek() == '}')
        {
            ++pos;
            return pairs;
        }
        while (true)
        {
            auto key = parse_string();
            if (!seen.insert(key).second)
            {
                fail(std::format("duplicate key '{}' in object", key));
            }
            expect(':');
            auto value = parse_string();
            pairs.emplace_back(std::move(key), std::move(value));
            const char next = peek();
            if (next == ',')
            {
                ++pos;
                continue;
            }
            if (next == '}')
            {
                ++pos;
                return pairs;
            }
            fail(std::format("expected ',' or '}}' but got '{}'", next));
        }
    }
};

/**
 * @brief Append each of @p pairs to @p row in @p header_index order.
 * @param pairs         Key/value pairs from one JSON object.
 * @param header_index  Map from header name to column index.
 * @param row           Destination row (resized to headers.size()).
 * @param object_index  Zero-based object index, for error messages.
 */
void row_from_object(const std::vector<std::pair<std::string, std::string>>& pairs,
                     const std::unordered_map<std::string, std::size_t>& header_index, std::vector<std::string>& row,
                     std::size_t object_index)
{
    if (pairs.size() != header_index.size())
    {
        throw std::runtime_error(
            std::format("json: object {} has {} keys, expected {}", object_index, pairs.size(), header_index.size()));
    }
    row.assign(header_index.size(), std::string{});
    for (const auto& [key, value] : pairs)
    {
        auto it = header_index.find(key);
        if (it == header_index.end())
        {
            throw std::runtime_error(std::format("json: object {} has unexpected key '{}'", object_index, key));
        }
        row[it->second] = value;
    }
}

/**
 * @brief Append the JSON escaping of @p s (including surrounding quotes).
 * @param out Output buffer.
 * @param s   Raw string.
 */
void escape_string(std::string& out, std::string_view s)
{
    out.push_back('"');
    for (const char c : s)
    {
        switch (c)
        {
        case '"':
            out.append("\\\"");
            break;
        case '\\':
            out.append("\\\\");
            break;
        case '\b':
            out.append("\\b");
            break;
        case '\f':
            out.append("\\f");
            break;
        case '\n':
            out.append("\\n");
            break;
        case '\r':
            out.append("\\r");
            break;
        case '\t':
            out.append("\\t");
            break;
        default:
            if (static_cast<unsigned char>(c) < control_char_limit)
            {
                out.append(std::format("\\u{:04x}", static_cast<unsigned>(c)));
            }
            else
            {
                out.push_back(c);
            }
            break;
        }
    }
    out.push_back('"');
}

} // namespace

auto parse(std::string_view text) -> core::Table // NOLINT(misc-use-internal-linkage)
{
    Parser p{.src = text};
    p.expect('[');
    if (p.peek() == ']')
    {
        throw std::runtime_error("json: empty top-level array");
    }

    core::Table table;
    std::unordered_map<std::string, std::size_t> header_index;
    std::size_t object_index = 0;

    while (true)
    {
        auto pairs = p.parse_object_pairs();
        if (object_index == 0)
        {
            table.headers.reserve(pairs.size());
            for (const auto& [key, _] : pairs)
            {
                header_index.emplace(key, table.headers.size());
                table.headers.push_back(key);
            }
        }
        std::vector<std::string> row;
        row_from_object(pairs, header_index, row, object_index);
        table.rows.push_back(std::move(row));
        ++object_index;

        const char next = p.peek();
        if (next == ',')
        {
            ++p.pos;
            continue;
        }
        if (next == ']')
        {
            ++p.pos;
            break;
        }
        throw std::runtime_error(std::format("json: expected ',' or ']' but got '{}' at offset {}", next, p.pos));
    }

    p.skip_ws();
    if (p.pos != p.src.size())
    {
        throw std::runtime_error(std::format("json: trailing content at offset {}", p.pos));
    }
    return table;
}

auto write(const core::Table& table) -> std::string // NOLINT(misc-use-internal-linkage)
{
    std::string out;
    out.push_back('[');
    for (std::size_t r = 0; r < table.rows.size(); ++r)
    {
        out.append(r == 0 ? "\n  {" : ",\n  {");
        for (std::size_t c = 0; c < table.headers.size(); ++c)
        {
            out.append(c == 0 ? "\n    " : ",\n    ");
            escape_string(out, table.headers[c]);
            out.append(": ");
            escape_string(out, table.rows[r][c]);
        }
        out.append("\n  }");
    }
    out.append(table.rows.empty() ? "]" : "\n]");
    return out;
}

} // namespace json
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,misc-non-private-member-variables-in-classes)
