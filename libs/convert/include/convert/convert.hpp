// SPDX-License-Identifier: MIT
/**
 * @file convert.hpp
 * @brief Public interface for the convert library.
 */

#ifndef CONVERT_CONVERT_HPP
#define CONVERT_CONVERT_HPP

#include <convert/convert_export.hpp>
#include <string>
#include <string_view>

/**
 * @namespace convert
 * @brief Contains all symbols exported by the convert library.
 */
namespace convert
{

/**
 * @brief Convert CSV text to a pretty-printed JSON array of objects.
 *
 * The first CSV line is the header row; each subsequent row becomes one
 * JSON object whose keys are the headers and whose values are the row's
 * fields (all emitted as JSON strings).
 *
 * @param csv_text The CSV input.
 * @return JSON text.
 * @throws std::runtime_error on malformed CSV.
 */
CONVERT_EXPORT auto csv_to_json(std::string_view csv_text) -> std::string;

/**
 * @brief Convert a JSON array of objects to CSV text.
 *
 * The first object's key order defines the CSV header; every object must
 * have the same string-valued keys.
 *
 * @param json_text The JSON input.
 * @return CSV text.
 * @throws std::runtime_error on malformed JSON or non-string values.
 */
CONVERT_EXPORT auto json_to_csv(std::string_view json_text) -> std::string;

} // namespace convert

#endif // CONVERT_CONVERT_HPP
