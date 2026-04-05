// SPDX-License-Identifier: MIT
/**
 * @file json.hpp
 * @brief Public interface for the json library.
 */

#ifndef JSON_JSON_HPP
#define JSON_JSON_HPP

#include <json/json_export.hpp>

/**
 * @namespace json
 * @brief Contains all symbols exported by the json library.
 */
namespace json
{

/**
 * @brief Print the library name ("json") to stdout.
 *
 * This is a simple identity function used to verify that the json library
 * is linked and callable.
 */
JSON_EXPORT void greet();

} // namespace json

#endif // JSON_JSON_HPP
