/**
 * @file csv.hpp
 * @brief Public interface for the csv library.
 */

#ifndef CSV_CSV_HPP
#define CSV_CSV_HPP

#include <csv/csv_export.hpp>

/**
 * @namespace csv
 * @brief Contains all symbols exported by the csv library.
 */
namespace csv
{

/**
 * @brief Print the library name ("csv") to stdout.
 *
 * This is a simple identity function used to verify that the csv library
 * is linked and callable.
 */
CSV_EXPORT void greet();

} // namespace csv

#endif // CSV_CSV_HPP
