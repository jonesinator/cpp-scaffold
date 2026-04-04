# libs/convert/

Library providing `convert::greet_both()`. Depends publicly on `csv` and `json`, demonstrating a diamond dependency on `core`. Installed as the `convert` component (`find_package(scaffold COMPONENTS convert)` → `scaffold::convert`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — static library with export header, install rules, csv/json dependencies, and unit test |
| [include/convert/convert.hpp](include/convert/convert.hpp) | Public header — declares `convert::greet_both()` |
| [src/convert.cpp](src/convert.cpp) | Implementation — calls `csv::greet()` then `json::greet()` |
| [tests/convert_test.cpp](tests/convert_test.cpp) | Unit test — verifies combined "csv\njson\n" output |
