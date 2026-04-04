# libs/json/

Library providing `json::greet()`. Depends publicly on `core`. Installed as the `json` component (`find_package(scaffold COMPONENTS json)` → `scaffold::json`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — static library with export header, install rules, core dependency, and unit test |
| [include/json/json.hpp](include/json/json.hpp) | Public header — declares `json::greet()` |
| [src/json.cpp](src/json.cpp) | Implementation — calls `core::println("json")` |
| [tests/json_test.cpp](tests/json_test.cpp) | Unit test — verifies `json::greet()` output |
