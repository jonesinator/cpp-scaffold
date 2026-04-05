// SPDX-License-Identifier: MIT
/**
 * @file table.hpp
 * @brief Shared tabular data type used by the csv and json libraries.
 */

#ifndef CORE_TABLE_HPP
#define CORE_TABLE_HPP

#include <string>
#include <vector>

namespace core
{

/**
 * @brief A row-oriented table of strings.
 *
 * Represents CSV-style tabular data as a header row plus zero or more
 * data rows. Every data row has @c row.size() equal to @c headers.size().
 * Values are opaque strings: no type inference is performed, so the
 * representation is lossless with respect to either CSV or JSON
 * (string-valued) text.
 */
struct Table
{
    std::vector<std::string> headers;           ///< Column names (first CSV row / JSON object keys).
    std::vector<std::vector<std::string>> rows; ///< Each row's field values in header order.
};

} // namespace core

#endif // CORE_TABLE_HPP
