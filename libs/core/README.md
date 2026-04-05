# libs/core/

Foundation library providing shared utilities used by other libraries, including the `core::Table` data type that the csv and json libraries both operate on. Installed as the `core` component (`find_package(scaffold COMPONENTS core)` → `scaffold::core`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — both shared (`libscaffold-core.so.0`) and static (`libscaffold-core.a`) library targets with SOVERSION, export header, per-component install rules, and unit test |
| [include/core/core.hpp](include/core/core.hpp) | Public header — declares `core::println()` |
| [include/core/table.hpp](include/core/table.hpp) | Public header — defines `core::Table`, the row-oriented string table shared by csv and json |
| [src/core.cpp](src/core.cpp) | Implementation — wraps `std::println` via `std::cout` |
| [tests/core_test.cpp](tests/core_test.cpp) | Unit test — verifies `core::println()` output |
