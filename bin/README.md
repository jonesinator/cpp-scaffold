# bin/

Executable targets for the scaffold project. Each subdirectory produces one binary.

| Directory | Description |
|---|---|
| [csv2json/](csv2json/) | Binary that links against `convert` (transitively depends on `csv`, `json`, and `core`) |
| [json2csv/](json2csv/) | Binary that links directly against `csv` and `json` |
