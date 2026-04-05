// SPDX-License-Identifier: MIT
/**
 * @file main.cpp
 * @brief Entry point for the csv2json binary.
 *
 * Demonstrates linking against convert, which transitively depends on
 * csv, json, and core.
 */

#include "convert/convert.hpp"

/**
 * @brief Call greet_both() from convert, then exit.
 */
auto main() -> int
{
    convert::greet_both();
}
