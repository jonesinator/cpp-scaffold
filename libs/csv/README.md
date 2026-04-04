# libs/csv/

Library providing `csv::greet()`. Depends publicly on `core`. Installed as the `csv` component (`find_package(scaffold COMPONENTS csv)` → `scaffold::csv`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — static library with export header, install rules, core dependency, and unit test |
| [include/csv/csv.hpp](include/csv/csv.hpp) | Public header — declares `csv::greet()` |
| [src/csv.cpp](src/csv.cpp) | Implementation — calls `core::println("csv")` |
| [tests/csv_test.cpp](tests/csv_test.cpp) | Unit test — verifies `csv::greet()` output |
