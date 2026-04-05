// SPDX-License-Identifier: MIT
/**
 * @file csv.cpp
 * @brief Implementation of the csv library.
 */

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,misc-non-private-member-variables-in-classes)
#include "csv/csv.hpp"

#include <cstddef>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace csv
{

namespace
{

/// Parser state threaded through the helpers.
struct Cursor
{
    std::string_view text;
    std::size_t pos = 0;
    std::size_t line = 1;
};

/**
 * @brief Handle one character inside a quoted field.
 *
 * Consumes one or two characters from @p cur and appends to @p field.
 * A doubled quote ("") is converted to a single literal quote; a lone
 * quote closes the field.
 *
 * @param cur   Parser cursor.
 * @param field Accumulator for the current field's value.
 * @return true if still inside quotes, false if the closing quote was seen.
 */
auto consume_quoted_char(Cursor& cur, std::string& field) -> bool
{
    const char c = cur.text[cur.pos];
    if (c == '"')
    {
        if (cur.pos + 1 < cur.text.size() && cur.text[cur.pos + 1] == '"')
        {
            field.push_back('"');
            cur.pos += 2;
            return true;
        }
        ++cur.pos;
        return false;
    }
    if (c == '\n')
    {
        ++cur.line;
    }
    field.push_back(c);
    ++cur.pos;
    return true;
}

/**
 * @brief Parse one record (row) starting at the cursor.
 *
 * Consumes characters up to (but not including) the end-of-record terminator
 * (CR, LF, CRLF, or end of input). Advances @p cur.
 *
 * @param cur Parser cursor.
 * @return The parsed fields (always at least one element).
 * @throws std::runtime_error on an unterminated quoted field.
 */
auto parse_row(Cursor& cur) -> std::vector<std::string>
{
    std::vector<std::string> fields;
    std::string field;
    bool in_quotes = false;
    const std::size_t start_line = cur.line;

    while (cur.pos < cur.text.size())
    {
        if (in_quotes)
        {
            in_quotes = consume_quoted_char(cur, field);
            continue;
        }
        const char c = cur.text[cur.pos];
        if (c == '"' && field.empty())
        {
            in_quotes = true;
            ++cur.pos;
        }
        else if (c == ',')
        {
            fields.push_back(std::move(field));
            field.clear();
            ++cur.pos;
        }
        else if (c == '\r' || c == '\n')
        {
            break;
        }
        else
        {
            field.push_back(c);
            ++cur.pos;
        }
    }

    if (in_quotes)
    {
        throw std::runtime_error(std::format("csv: unterminated quoted field starting at line {}", start_line));
    }

    fields.push_back(std::move(field));
    return fields;
}

/**
 * @brief Skip a single LF, CR, or CRLF terminator at the cursor.
 * @param cur Parser cursor.
 */
void skip_line_terminator(Cursor& cur)
{
    if (cur.pos < cur.text.size() && cur.text[cur.pos] == '\r')
    {
        ++cur.pos;
    }
    if (cur.pos < cur.text.size() && cur.text[cur.pos] == '\n')
    {
        ++cur.pos;
        ++cur.line;
    }
}

/**
 * @brief Whether a field must be quoted on output.
 * @param s The raw field value.
 * @return true if @p s contains a comma, quote, CR, or LF.
 */
auto needs_quoting(std::string_view s) -> bool
{
    return s.find_first_of(",\"\r\n") != std::string_view::npos;
}

/**
 * @brief Append a single field to @p out, quoting and escaping if necessary.
 * @param out The output buffer.
 * @param s   The field value.
 */
void write_field(std::string& out, std::string_view s)
{
    if (!needs_quoting(s))
    {
        out.append(s);
        return;
    }
    out.push_back('"');
    for (const char c : s)
    {
        if (c == '"')
        {
            out.push_back('"');
        }
        out.push_back(c);
    }
    out.push_back('"');
}

} // namespace

auto parse(std::string_view text) -> core::Table // NOLINT(misc-use-internal-linkage)
{
    if (text.empty())
    {
        throw std::runtime_error("csv: empty input");
    }

    core::Table table;
    Cursor cur{.text = text};

    table.headers = parse_row(cur);
    skip_line_terminator(cur);

    while (cur.pos < cur.text.size())
    {
        auto row = parse_row(cur);
        if (row.size() != table.headers.size())
        {
            throw std::runtime_error(std::format("csv: row at line {} has {} fields, expected {}", cur.line, row.size(),
                                                 table.headers.size()));
        }
        table.rows.push_back(std::move(row));
        skip_line_terminator(cur);
    }

    return table;
}

auto write(const core::Table& table) -> std::string // NOLINT(misc-use-internal-linkage)
{
    std::string out;

    auto write_row = [&out](const std::vector<std::string>& row)
    {
        for (std::size_t i = 0; i < row.size(); ++i)
        {
            if (i != 0)
            {
                out.push_back(',');
            }
            write_field(out, row[i]);
        }
        out.push_back('\n');
    };

    write_row(table.headers);
    for (const auto& row : table.rows)
    {
        write_row(row);
    }
    return out;
}

} // namespace csv
// NOLINTEND(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access,misc-non-private-member-variables-in-classes)
