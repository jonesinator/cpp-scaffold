# libs/

Shared libraries for the scaffold project. Each library follows the same structure: `include/<name>/` for public headers, `src/` for implementation, and `tests/` for unit tests.

| Directory | Description |
|---|---|
| [core/](core/) | Foundation library providing `core::println()` and the `core::Table` data type. No external dependencies |
| [csv/](csv/) | RFC-4180 CSV parser/writer over `core::Table`. Depends on `core` |
| [json/](json/) | JSON array-of-objects parser/writer (string values only) over `core::Table`. Depends on `core` |
| [convert/](convert/) | CSV↔JSON conversion as a thin composition. Depends on `csv` and `json` (and transitively `core`) |
| [test_support/](test_support/) | Header-only utility library for integration tests. Provides `subprocess::run()` |
