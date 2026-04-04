# bin/json2csv/

The `json2csv` executable. Demonstrates linking directly against `csv` and `json`. Included in the CMake install rules and all distribution packages.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — links against `csv` and `json`, install rule, defines integration test |
| [main.cpp](main.cpp) | Entry point — calls `csv::greet()` and `json::greet()` |
| [tests/](tests/) | Integration tests for the built binary |
