# libs/test_support/

Header-only INTERFACE library providing subprocess utilities for integration testing. Used by binary tests to launch executables and verify their behavior.

| Path | Description |
|---|---|
| [CMakeLists.txt](CMakeLists.txt) | Build definition — INTERFACE library exposing include path |
| [include/test_support/subprocess.hpp](include/test_support/subprocess.hpp) | `subprocess::run()` — launches a child process with arbitrary args/env, captures stdout, stderr, and exit code |
