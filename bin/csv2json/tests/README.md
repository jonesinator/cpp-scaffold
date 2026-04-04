# bin/csv2json/tests/

Integration tests for the `csv2json` binary. Tests launch the built binary as a subprocess and verify its stdout, stderr, and exit code.

| File | Description |
|---|---|
| [csv2json_test.cpp](csv2json_test.cpp) | Verifies `csv2json` prints "csv\njson\n" and exits 0 |
