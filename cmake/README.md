# cmake/

Reusable CMake modules included by the root `CMakeLists.txt`.

| File | Description |
|---|---|
| [Ccache.cmake](Ccache.cmake) | Auto-detects ccache and sets `CMAKE_CXX_COMPILER_LAUNCHER` |
| [Coverage.cmake](Coverage.cmake) | Adds `coverage-html` and `coverage-report` custom targets using lcov/genhtml |
| [Sanitizers.cmake](Sanitizers.cmake) | Adds ASan+UBSan (`ENABLE_ASAN`) and TSan (`ENABLE_TSAN`) compile/link flags with mutual exclusion |
| [Version.cmake](Version.cmake) | Runs `configure_file()` to generate `scaffold/version.hpp` from the template |
| [version.hpp.in](version.hpp.in) | Template header defining `SCAFFOLD_VERSION`, `SCAFFOLD_VERSION_MAJOR/MINOR/PATCH` |
| [Install.cmake](Install.cmake) | Installs version header, generates CMake package config via `CMakePackageConfigHelpers`, and exports the `scaffoldTargets` export set |
| [scaffold-config.cmake.in](scaffold-config.cmake.in) | Package config template enabling `find_package(scaffold COMPONENTS core)` with component validation |
| [Packaging.cmake](Packaging.cmake) | CPack configuration for TGZ, DEB, and RPM generators with distribution-specific metadata |
