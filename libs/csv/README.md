# libs/csv/

Library that parses and writes RFC-4180 CSV, converting between text and `core::Table`. Depends publicly on `core`. Installed as the `csv` component (`find_package(scaffold COMPONENTS csv)` → `scaffold::csv`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — both shared (`libscaffold-csv.so.0`) and static (`libscaffold-csv.a`) library targets with SOVERSION, export header, per-component install rules, core dependency, and unit test |
| [include/csv/csv.hpp](include/csv/csv.hpp) | Public header — declares `csv::parse()` and `csv::write()` |
| [src/csv.cpp](src/csv.cpp) | Implementation — state-machine parser with RFC-4180 quoting; writer quotes only fields containing `,`, `"`, CR, or LF |
| [tests/csv_test.cpp](tests/csv_test.cpp) | Unit tests — parsing, quoting, escaping, empty fields, CRLF, round-trip, malformed input |
