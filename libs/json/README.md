# libs/json/

Library that parses and writes string-valued JSON arrays of objects, converting between text and `core::Table`. Depends publicly on `core`. Installed as the `json` component (`find_package(scaffold COMPONENTS json)` → `scaffold::json`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — both shared (`libscaffold-json.so.0`) and static (`libscaffold-json.a`) library targets with SOVERSION, export header, per-component install rules, core dependency, and unit test |
| [include/json/json.hpp](include/json/json.hpp) | Public header — declares `json::parse()` and `json::write()` |
| [src/json.cpp](src/json.cpp) | Implementation — hand-rolled recursive-descent parser with full `\uXXXX` + surrogate-pair support; writer emits 2-space-indented array of objects |
| [tests/json_test.cpp](tests/json_test.cpp) | Unit tests — parsing, whitespace, escapes, BMP + supplementary-plane Unicode, round-trip, strict rejection of non-string values / duplicate keys / mismatched schemas |
