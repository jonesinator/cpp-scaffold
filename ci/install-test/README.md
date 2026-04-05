# ci/install-test/

Minimal consumer project used by CI install-test jobs to verify that the installed scaffold package works. Builds a small executable that requests all scaffold components via `find_package` and exercises them at runtime.

| File | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Calls `find_package(scaffold REQUIRED COMPONENTS core csv json convert)`, links `scaffold::convert` (pulls in `csv`, `json`, `core` transitively) |
| [main.cpp](main.cpp) | Prints `SCAFFOLD_VERSION` then `convert::csv_to_json("k\nv\n")` — exercises `core::println` and the full `convert → csv + json → core` chain |
