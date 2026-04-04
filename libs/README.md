# libs/

Shared libraries for the scaffold project. Each library follows the same structure: `include/<name>/` for public headers, `src/` for implementation, and `tests/` for unit tests.

| Directory | Description |
|---|---|
| [core/](core/) | Foundation library providing `core::println()`. No external dependencies |
| [csv/](csv/) | Library providing `csv::greet()`. Depends on `core` |
| [json/](json/) | Library providing `json::greet()`. Depends on `core` |
| [convert/](convert/) | Library providing `convert::greet_both()`. Depends on `csv` and `json` (and transitively `core`) |
| [test_support/](test_support/) | Header-only utility library for integration tests. Provides `subprocess::run()` |
