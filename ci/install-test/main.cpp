// SPDX-License-Identifier: MIT
#include <convert/convert.hpp>
#include <core/core.hpp>
#include <scaffold/version.hpp>

int main()
{
    core::println(SCAFFOLD_VERSION);
    core::println(convert::csv_to_json("k\nv\n"));
}
