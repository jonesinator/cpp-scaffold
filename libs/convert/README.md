# libs/convert/

Library providing `convert::csv_to_json()` and `convert::json_to_csv()` — thin compositions over the csv and json libraries' parse/write functions. Depends publicly on `csv` and `json`, demonstrating a diamond dependency on `core`. Installed as the `convert` component (`find_package(scaffold COMPONENTS convert)` → `scaffold::convert`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — both shared (`libscaffold-convert.so.0`) and static (`libscaffold-convert.a`) library targets with SOVERSION, export header, per-component install rules, csv/json dependencies, and unit test |
| [include/convert/convert.hpp](include/convert/convert.hpp) | Public header — declares `convert::csv_to_json()` and `convert::json_to_csv()` |
| [src/convert.cpp](src/convert.cpp) | Implementation — composes `json::write(csv::parse(...))` and `csv::write(json::parse(...))` |
| [tests/convert_test.cpp](tests/convert_test.cpp) | Unit test — verifies byte-identical CSV↔JSON round trips |
