# bin/csv2json/

The `csv2json` executable. Demonstrates linking against `convert`, which transitively depends on `csv`, `json`, and `core`. Included in the CMake install rules and all distribution packages.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — links against `convert`, install rule, defines integration test |
| [main.cpp](main.cpp) | Entry point — calls `convert::greet_both()` |
| [tests/](tests/) | Integration tests for the built binary |
