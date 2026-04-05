# cmake/

Reusable CMake modules included by the root `CMakeLists.txt`.

| File | Description |
|---|---|
| [Ccache.cmake](Ccache.cmake) | Auto-detects ccache and sets `CMAKE_CXX_COMPILER_LAUNCHER` |
| [Coverage.cmake](Coverage.cmake) | Adds `coverage-html` and `coverage-report` custom targets using lcov/genhtml |
| [Sanitizers.cmake](Sanitizers.cmake) | Adds ASan+UBSan (`ENABLE_ASAN`) and TSan (`ENABLE_TSAN`) compile/link flags with mutual exclusion |
| [Hardening.cmake](Hardening.cmake) | Applies OpenSSF-recommended hardening flags (`_FORTIFY_SOURCE=3`, stack-protector-strong, stack-clash, CET, full RELRO, BIND_NOW, noexecstack, separate-code, PIE) scoped to Release builds (`ENABLE_HARDENING`, default ON) |
| [Version.cmake](Version.cmake) | Runs `configure_file()` to generate `scaffold/version.hpp` from the template |
| [version.hpp.in](version.hpp.in) | Template header defining `SCAFFOLD_VERSION`, `SCAFFOLD_VERSION_MAJOR/MINOR/PATCH` |
| [Install.cmake](Install.cmake) | Installs version header, generates CMake package config via `CMakePackageConfigHelpers`, exports the `scaffoldTargets` export set, and normalizes staged directory permissions to 0755 via `install(CODE ... ALL_COMPONENTS)` for umask-independent packaging |
| [scaffold-config.cmake.in](scaffold-config.cmake.in) | Package config template enabling `find_package(scaffold COMPONENTS core)` with component validation |
| [Packaging.cmake](Packaging.cmake) | CPack configuration for TGZ, DEB, and RPM generators with distribution-specific metadata |
| [SortTarballs.cmake](SortTarballs.cmake) | CPack post-build script that re-archives each `.tar.gz` with `tar --sort=name --owner=0 --group=0 \| gzip -n` so TGZ entry order is invariant to filesystem readdir order and the packaging user |
