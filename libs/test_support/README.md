# libs/test_support/

Header-only INTERFACE library providing subprocess utilities for integration testing. Used by binary tests to launch executables and verify their behavior.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — INTERFACE library exposing include path |
| [include/test_support/subprocess.hpp](include/test_support/subprocess.hpp) | `subprocess::run()` — launches a child process with arbitrary args/env, optionally pipes data to its stdin, and captures stdout, stderr, and exit code |
| [include/test_support/expect.hpp](include/test_support/expect.hpp) | `expect::Suite` — minimal assertion helper with a failure counter, per-check logging, and a `finish()` that returns `EXIT_SUCCESS`/`EXIT_FAILURE` with a summary line |
| [tests/test_support_test.cpp](tests/test_support_test.cpp) | Self-tests covering both `subprocess` and `expect` headers |
