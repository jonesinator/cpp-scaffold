# bin/json2csv/tests/

Integration tests for the `json2csv` binary. Tests launch the built binary as a subprocess and verify its stdout, stderr, and exit code.

| File | Description |
|---|---|
| [json2csv_test.cpp](json2csv_test.cpp) | Verifies `json2csv` prints "csv\njson\n" and exits 0 |
