// SPDX-License-Identifier: MIT
/**
 * @file convert.cpp
 * @brief Implementation of the convert library.
 */

#include "convert/convert.hpp"

#include "csv/csv.hpp"

#include "json/json.hpp"

namespace convert
{

void greet_both()
{
    csv::greet();
    json::greet();
}

} // namespace convert
