// SPDX-License-Identifier: MIT
/**
 * @file csv.hpp
 * @brief Public interface for the csv library.
 */

#ifndef CSV_CSV_HPP
#define CSV_CSV_HPP

#include <core/table.hpp>
#include <csv/csv_export.hpp>
#include <string>
#include <string_view>

/**
 * @namespace csv
 * @brief Contains all symbols exported by the csv library.
 */
namespace csv
{

/**
 * @brief Parse RFC-4180-style CSV text into a core::Table.
 *
 * The first line is the header row; subsequent lines are data rows. Fields
 * may be unquoted or double-quoted. Inside a quoted field, a doubled quote
 * (""""") represents a literal quote character, and embedded commas / CR / LF
 * are preserved verbatim. Both LF and CRLF line terminators are accepted on
 * input; a trailing terminator after the last row is permitted.
 *
 * @param text The CSV text to parse.
 * @return A Table whose row count equals the number of data lines.
 * @throws std::runtime_error on empty input, an unterminated quoted field,
 *         or a data row whose field count does not match the header.
 */
CSV_EXPORT auto parse(std::string_view text) -> core::Table;

/**
 * @brief Serialize a core::Table to RFC-4180-style CSV text.
 *
 * Fields are wrapped in double-quotes only when they contain a comma, a
 * double-quote, CR, or LF. Embedded double-quotes are doubled. The line
 * terminator is LF ("\n"); every row (including the last) is terminated.
 *
 * @param table The table to serialize.
 * @return The CSV text.
 */
CSV_EXPORT auto write(const core::Table& table) -> std::string;

} // namespace csv

#endif // CSV_CSV_HPP
