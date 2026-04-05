// SPDX-License-Identifier: MIT
/**
 * @file json.hpp
 * @brief Public interface for the json library.
 */

#ifndef JSON_JSON_HPP
#define JSON_JSON_HPP

#include <core/table.hpp>
#include <json/json_export.hpp>
#include <string>
#include <string_view>

/**
 * @namespace json
 * @brief Contains all symbols exported by the json library.
 */
namespace json
{

/**
 * @brief Parse a JSON array-of-objects into a core::Table.
 *
 * Expects text of the form @c [{...},{...}] where every value is a JSON
 * string. The first object's key order defines the resulting table's
 * header order; every subsequent object must have the same key set.
 * Whitespace is tolerated between tokens. Supports the standard JSON
 * escapes (\\", \\\\, \\/, \\b, \\f, \\n, \\r, \\t) and \\uXXXX including
 * UTF-16 surrogate pairs for supplementary-plane code points.
 *
 * @param text The JSON text to parse.
 * @return A Table with one row per object.
 * @throws std::runtime_error on malformed JSON, a non-string value, an
 *         empty top-level array, a duplicate key, or inconsistent key sets.
 */
JSON_EXPORT auto parse(std::string_view text) -> core::Table;

/**
 * @brief Serialize a core::Table as a pretty-printed JSON array of objects.
 *
 * Output uses two-space indentation and LF line endings, with no trailing
 * newline. Keys are emitted in @c table.headers order; values are escaped
 * per RFC 8259.
 *
 * @param table The table to serialize.
 * @return The JSON text.
 */
JSON_EXPORT auto write(const core::Table& table) -> std::string;

} // namespace json

#endif // JSON_JSON_HPP
