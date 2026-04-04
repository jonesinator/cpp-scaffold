# ci/install-test/

Minimal consumer project used by CI install-test jobs to verify that the installed scaffold package works. Builds a small executable that requests all scaffold components via `find_package` and exercises them at runtime.

| File | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Calls `find_package(scaffold REQUIRED COMPONENTS core csv json convert)`, links `scaffold::convert` (pulls in `csv`, `json`, `core` transitively) |
| [main.cpp](main.cpp) | Prints `SCAFFOLD_VERSION` then calls `convert::greet_both()` (output: `0.1.0\ncsv\njson`) |
