# libs/core/

Foundation library providing shared output functionality used by other libraries. Installed as the `core` component (`find_package(scaffold COMPONENTS core)` → `scaffold::core`).

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — static library with export header, install rules, and unit test |
| [include/core/core.hpp](include/core/core.hpp) | Public header — declares `core::println()` |
| [src/core.cpp](src/core.cpp) | Implementation — wraps `std::println` via `std::cout` |
| [tests/core_test.cpp](tests/core_test.cpp) | Unit test — verifies `core::println()` output |
