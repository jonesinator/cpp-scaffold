// SPDX-License-Identifier: MIT
/**
 * @file main.cpp
 * @brief Entry point for the json2csv binary.
 *
 * Demonstrates linking against both the csv and json libraries.
 */

#include "csv/csv.hpp"

#include "json/json.hpp"

/**
 * @brief Call greet() from both csv and json, then exit.
 */
auto main() -> int
{
    csv::greet();
    json::greet();
}
