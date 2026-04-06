// SPDX-License-Identifier: MIT
/**
 * @file convert.cpp
 * @brief Implementation of the convert library.
 */

#include "convert/convert.hpp"

#include "csv/csv.hpp"

#include "json/json.hpp"
#include <string>
#include <string_view>

namespace convert
{

auto csv_to_json(std::string_view csv_text) -> std::string
{
    return json::write(csv::parse(csv_text));
}

auto json_to_csv(std::string_view json_text) -> std::string
{
    return csv::write(json::parse(json_text));
}

} // namespace convert
