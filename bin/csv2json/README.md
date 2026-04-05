# bin/csv2json/

The `csv2json` executable. Reads CSV from stdin, writes a pretty-printed JSON array of objects to stdout. Links against `convert`, which transitively depends on `csv`, `json`, and `core`. Included in the CMake install rules and all distribution packages.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — links against `convert`, install rule, defines integration test |
| [main.cpp](main.cpp) | Entry point — reads stdin, calls `convert::csv_to_json()`, writes to stdout |
| [tests/](tests/) | Integration tests for the built binary |
