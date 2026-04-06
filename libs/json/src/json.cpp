// SPDX-License-Identifier: MIT
/**
 * @file json.cpp
 * @brief Implementation of the json library.
 */

#include "json/json.hpp"

#include "core/table.hpp"

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
 * @brief Append @p codepoint as a UTF-8 byte sequence to @p out.
 * @param codepoint  Unicode code point (assumed valid, not a surrogate).
 * @param out Output buffer.
 */
void encode_utf8(std::uint32_t codepoint, std::string& out)
{
    if (codepoint < utf8_1byte_limit)
    {
        out.push_back(static_cast<char>(codepoint));
    }
    else if (codepoint < utf8_2byte_limit)
    {
        out.push_back(static_cast<char>(lead2_prefix | (codepoint >> utf8_shift_1)));
        out.push_back(static_cast<char>(cont_prefix | (codepoint & cont_mask)));
    }
    else if (codepoint < utf8_3byte_limit)
    {
        out.push_back(static_cast<char>(lead3_prefix | (codepoint >> utf8_shift_2)));
        out.push_back(static_cast<char>(cont_prefix | ((codepoint >> utf8_shift_1) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | (codepoint & cont_mask)));
    }
    else
    {
        out.push_back(static_cast<char>(lead4_prefix | (codepoint >> utf8_shift_3)));
        out.push_back(static_cast<char>(cont_prefix | ((codepoint >> utf8_shift_2) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | ((codepoint >> utf8_shift_1) & cont_mask)));
        out.push_back(static_cast<char>(cont_prefix | (codepoint & cont_mask)));
    }
}

/// Parser state (value type; members are part of the parsing cursor).
class Parser
{
  public:
    explicit Parser(std::string_view text) : src_(text) {}

    /// Advance past the current character (for top-level comma/bracket skipping).
    void advance()
    {
        ++pos_;
    }

    /// Current byte offset into the source (for error messages).
    [[nodiscard]] auto offset() const -> std::size_t
    {
        return pos_;
    }

    /// Whether the entire input has been consumed.
    [[nodiscard]] auto at_end() -> bool
    {
        skip_ws();
        return pos_ >= src_.size();
    }

    [[noreturn]] void fail(std::string_view msg) const
    {
        throw std::runtime_error(std::format("json: {} at offset {}", msg, pos_));
    }

    void skip_ws()
    {
        while (pos_ < src_.size())
        {
            const char chr = src_.at(pos_);
            if (chr != ' ' && chr != '\t' && chr != '\n' && chr != '\r')
            {
                break;
            }
            ++pos_;
        }
    }

    auto peek() -> char
    {
        skip_ws();
        if (pos_ >= src_.size())
        {
            fail("unexpected end of input");
        }
        return src_.at(pos_);
    }

    void expect(char chr)
    {
        const char got = peek();
        if (got != chr)
        {
            fail(std::format("expected '{}' but got '{}'", chr, got));
        }
        ++pos_;
    }

    [[nodiscard]] auto hex_value(char chr) const -> std::uint32_t
    {
        if (chr >= '0' && chr <= '9')
        {
            return static_cast<std::uint32_t>(chr - '0');
        }
        if (chr >= 'a' && chr <= 'f')
        {
            return static_cast<std::uint32_t>(chr - 'a') + hex_letter_offset;
        }
        if (chr >= 'A' && chr <= 'F')
        {
            return static_cast<std::uint32_t>(chr - 'A') + hex_letter_offset;
        }
        fail("invalid hex digit in \\u escape");
    }

    auto parse_hex4() -> std::uint32_t
    {
        if (pos_ + hex_digits_in_escape > src_.size())
        {
            fail("truncated \\u escape");
        }
        std::uint32_t result = 0;
        for (int i = 0; i < hex_digits_in_escape; ++i)
        {
            result = (result * hex_base) + hex_value(src_.at(pos_));
            ++pos_;
        }
        return result;
    }

    void parse_unicode_escape(std::string& out)
    {
        std::uint32_t codepoint = parse_hex4();
        if (codepoint >= surrogate_high_min && codepoint <= surrogate_high_max)
        {
            if (pos_ + 1 >= src_.size() || src_.at(pos_) != '\\' || src_.at(pos_ + 1) != 'u')
            {
                fail("unpaired high surrogate in \\u escape");
            }
            pos_ += 2;
            const std::uint32_t low = parse_hex4();
            if (low < surrogate_low_min || low > surrogate_low_max)
            {
                fail("invalid low surrogate in \\u escape");
            }
            codepoint =
                surrogate_base + ((codepoint - surrogate_high_min) << surrogate_shift) + (low - surrogate_low_min);
        }
        else if (codepoint >= surrogate_low_min && codepoint <= surrogate_low_max)
        {
            fail("unexpected low surrogate in \\u escape");
        }
        encode_utf8(codepoint, out);
    }

    void parse_escape(std::string& out)
    {
        if (pos_ >= src_.size())
        {
            fail("unterminated escape sequence");
        }
        const char chr = src_.at(pos_);
        ++pos_;
        switch (chr)
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
            fail(std::format("invalid escape character '\\{}'", chr));
        }
    }

    auto parse_string() -> std::string
    {
        expect('"');
        std::string out;
        while (pos_ < src_.size())
        {
            const char chr = src_.at(pos_);
            if (chr == '"')
            {
                ++pos_;
                return out;
            }
            if (chr == '\\')
            {
                ++pos_;
                parse_escape(out);
            }
            else if (static_cast<unsigned char>(chr) < control_char_limit)
            {
                fail("unescaped control character in string");
            }
            else
            {
                out.push_back(chr);
                ++pos_;
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
            ++pos_;
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
                ++pos_;
                continue;
            }
            if (next == '}')
            {
                ++pos_;
                return pairs;
            }
            fail(std::format("expected ',' or '}}' but got '{}'", next));
        }
    }

  private:
    std::string_view src_;
    std::size_t pos_ = 0;
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
        auto iter = header_index.find(key);
        if (iter == header_index.end())
        {
            throw std::runtime_error(std::format("json: object {} has unexpected key '{}'", object_index, key));
        }
        row.at(iter->second) = value;
    }
}

/**
 * @brief Append the JSON escaping of @p str (including surrounding quotes).
 * @param out Output buffer.
 * @param str Raw string.
 */
void escape_string(std::string& out, std::string_view str)
{
    out.push_back('"');
    for (const char chr : str)
    {
        switch (chr)
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
            if (static_cast<unsigned char>(chr) < control_char_limit)
            {
                out.append(std::format("\\u{:04x}", static_cast<unsigned>(chr)));
            }
            else
            {
                out.push_back(chr);
            }
            break;
        }
    }
    out.push_back('"');
}

} // namespace

auto parse(std::string_view text) -> core::Table
{
    Parser parser(text);
    parser.expect('[');
    if (parser.peek() == ']')
    {
        throw std::runtime_error("json: empty top-level array");
    }

    core::Table table;
    std::unordered_map<std::string, std::size_t> header_index;
    std::size_t object_index = 0;

    while (true)
    {
        auto pairs = parser.parse_object_pairs();
        if (object_index == 0)
        {
            table.headers.reserve(pairs.size());
            for (const auto& [key, unused] : pairs)
            {
                header_index.emplace(key, table.headers.size());
                table.headers.push_back(key);
            }
        }
        std::vector<std::string> row;
        row_from_object(pairs, header_index, row, object_index);
        table.rows.push_back(std::move(row));
        ++object_index;

        const char next = parser.peek();
        if (next == ',')
        {
            parser.advance();
            continue;
        }
        if (next == ']')
        {
            parser.advance();
            break;
        }
        throw std::runtime_error(
            std::format("json: expected ',' or ']' but got '{}' at offset {}", next, parser.offset()));
    }

    if (!parser.at_end())
    {
        throw std::runtime_error(std::format("json: trailing content at offset {}", parser.offset()));
    }
    return table;
}

auto write(const core::Table& table) -> std::string
{
    std::string out;
    out.push_back('[');
    for (std::size_t row = 0; row < table.rows.size(); ++row)
    {
        out.append(row == 0 ? "\n  {" : ",\n  {");
        for (std::size_t col = 0; col < table.headers.size(); ++col)
        {
            out.append(col == 0 ? "\n    " : ",\n    ");
            escape_string(out, table.headers.at(col));
            out.append(": ");
            escape_string(out, table.rows.at(row).at(col));
        }
        out.append("\n  }");
    }
    out.append(table.rows.empty() ? "]" : "\n]");
    return out;
}

} // namespace json
