/**
 * @file core.hpp
 * @brief Public interface for the core library.
 */

#ifndef CORE_CORE_HPP
#define CORE_CORE_HPP

#include <core/core_export.hpp>
#include <string_view>

/**
 * @namespace core
 * @brief Contains all symbols exported by the core library.
 */
namespace core
{

/**
 * @brief Print a message to stdout followed by a newline.
 * @param msg The message to print.
 */
CORE_EXPORT void println(std::string_view msg);

} // namespace core

#endif // CORE_CORE_HPP
