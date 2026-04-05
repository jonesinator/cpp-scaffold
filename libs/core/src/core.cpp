// SPDX-License-Identifier: MIT
/**
 * @file core.cpp
 * @brief Implementation of the core library.
 */

#include "core/core.hpp"

#include <iostream>
#include <print>
#include <string_view>

namespace core
{

void println(std::string_view msg)
{
    std::println(std::cout, "{}", msg);
}

} // namespace core
