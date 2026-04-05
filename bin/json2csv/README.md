# bin/json2csv/

The `json2csv` executable. Reads a JSON array of objects from stdin, writes RFC-4180 CSV to stdout. Links against `convert`, which transitively depends on `csv`, `json`, and `core`. Included in the CMake install rules and all distribution packages.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — links against `convert`, install rule, defines integration test |
| [main.cpp](main.cpp) | Entry point — reads stdin, calls `convert::json_to_csv()`, writes to stdout |
| [tests/](tests/) | Integration tests for the built binary |
