# bin/

Executable targets for the scaffold project. Each subdirectory produces one binary. Both binaries are Unix-pipeline tools: they read from stdin, write to stdout, and print errors to stderr with a non-zero exit code.

| Directory | Description |
|---|---|
| [csv2json/](csv2json/) | Reads CSV from stdin, writes pretty-printed JSON to stdout. Links against `convert` |
| [json2csv/](json2csv/) | Reads JSON from stdin, writes CSV to stdout. Links against `convert` |
