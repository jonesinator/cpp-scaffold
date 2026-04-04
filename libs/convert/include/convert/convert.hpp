/**
 * @file convert.hpp
 * @brief Public interface for the convert library.
 */

#ifndef CONVERT_CONVERT_HPP
#define CONVERT_CONVERT_HPP

#include <convert/convert_export.hpp>

/**
 * @namespace convert
 * @brief Contains all symbols exported by the convert library.
 */
namespace convert
{

/**
 * @brief Greet from both csv and json.
 *
 * Calls csv::greet() and json::greet() to demonstrate a library that
 * depends on multiple other libraries which share a common dependency (core).
 */
CONVERT_EXPORT void greet_both();

} // namespace convert

#endif // CONVERT_CONVERT_HPP
