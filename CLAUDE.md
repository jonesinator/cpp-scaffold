# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Quick Start

```bash
just setup              # install pre-commit hook
just test               # build debug + run all tests
just check              # full validation (all profiles, lint, coverage, docs)
```

## Development Environments

Four equivalent environments are available. All provide GCC 15, CMake, Ninja, mold, ccache, clang-format, clang-tidy, lcov, doxygen, graphviz, texlive, just, and gdb.

| Environment | Entry command |
|---|---|
| Native (Arch) | Direct — tools installed locally |
| Nix flake | `nix develop` |
| Guix manifest | `guix shell -m manifest.scm` |
| Dev container | Open in VS Code with Dev Containers extension |

## Build Profiles

All builds use CMake presets (`CMakePresets.json`), Ninja generator, and mold linker. Run `just` with no arguments to see all recipes and profiles.

```bash
just build              # debug (default)
just build release      # release (stripped binaries)
just build asan         # AddressSanitizer + UBSan
just build tsan         # ThreadSanitizer
```

Profiles: `debug`, `release`, `coverage`, `asan`, `tsan`.

## Testing

Tests are plain C++ executables (no framework) run via ctest. Library tests validate internal behavior. Binary tests use `libs/test_support/subprocess.hpp` to launch the actual binary and verify stdout/stderr/exit code.

```bash
just test               # all tests (debug)
just test release       # all tests (release)
just test asan          # all tests under ASan + UBSan
just test tsan          # all tests under TSan
just test-one csv_test  # single test by name
```

## Code Coverage

```bash
just coverage           # build + test + HTML report (build/coverage/coverage/)
just coverage-report    # machine-readable coverage.info only
just open-coverage      # generate + open in browser
```

Coverage uses a dedicated build with `--coverage` flags. The `-ffile-prefix-map` reproducibility flag is disabled for coverage builds so lcov can resolve source paths. Test failure paths are marked with `LCOV_EXCL_START`/`LCOV_EXCL_STOP`.

## Formatting and Linting

```bash
just format             # clang-format all sources (120-column, Allman braces)
just format-check       # check without modifying (used by pre-commit hook)
just lint               # clang-tidy on all sources (warnings as errors)
```

The `.clang-tidy` config enables aggressive checks from bugprone, cert, cppcoreguidelines, hicpp, misc, modernize, performance, portability, and readability groups. It adds `-Wno-unknown-warning-option` to suppress GCC-specific warning flags that clang doesn't understand. Naming conventions: `snake_case` for functions/variables/namespaces, `CamelCase` for classes, `_` suffix for private members.

## Documentation

```bash
just docs               # HTML + PDF (build/docs/)
just docs-html          # HTML only
just open-docs          # generate + open in browser
just deps               # CMake dependency graph as SVG (build/deps.svg)
just open-deps          # generate + open in browser
```

Doxygen generates docs with call graphs, include graphs, and directory dependency diagrams (requires graphviz). All C++ source files must have doxygen comments. Public API functions must be annotated with their library's export macro.

## Installation and Packaging

```bash
just install                # install to system prefix (release)
just install-to /tmp/test   # install to custom prefix
just package                # create TGZ, DEB, RPM via CPack
nix build                   # build Nix package
guix build -f package.scm   # build Guix package
makepkg                     # build Arch Linux .pkg.tar.zst
```

All libraries (`core`, `csv`, `json`, `convert`) and both binaries (`csv2json`, `json2csv`) have CMake install rules. Each library builds as **both shared and static** (`libscaffold-<name>.so.0.1.0` + `libscaffold-<name>.a`). Downstream consumers can `find_package(scaffold REQUIRED COMPONENTS core csv json convert)` and link against `scaffold::<name>` (shared) or `scaffold::<name>_static`.

**Per-component packaging:** All distributions (DEB, RPM, APK, pkg.tar.zst) are split into per-component packages following Debian convention: `libscaffold-<lib>` (runtime), `libscaffold-<lib>-dev` (headers + static + symlink), plus `scaffold-csv2json`/`scaffold-json2csv` (binaries) and `libscaffold-dev` (umbrella dev package with CMake config). Each profile produces 11 packages per distro.

**Static binaries:** The `static` CMake preset (`just build static`) enables `SCAFFOLD_STATIC_BINARIES=ON` which links binaries against the `*_static` library targets and adds `-static` link flag. Built on Alpine it produces musl-static binaries (~1.4 MB each) that run on any Linux distro without runtime deps. CI uploads them as `scaffold-csv2json-static-x86_64` / `scaffold-json2csv-static-x86_64` artifacts.

**SBOMs:** CI generates SPDX-JSON SBOMs via syft for every artifact — per-package (11 per distro×profile in the `*-sboms` artifact), build-environment (one `buildenv-<distro>-<profile>.spdx.json` per package job capturing toolchain state), static binary (ships inside the static binary artifacts), and source (in `scaffold-source-sbom` along with runner image metadata). We don't generate an SBOM for the outer Ubuntu runner — GitHub publishes those at actions/runner-images.

**Package definitions:**
- CPack (TGZ/DEB/RPM): configured in `cmake/Packaging.cmake`
- Nix: `packages.default` output in `flake.nix`
- Guix: `package.scm`
- Arch: `PKGBUILD` (uses `!lto` to avoid slim LTO objects in static archives)
- Alpine: `APKBUILD` (built with `abuild`)

## CI

`.github/workflows/ci.yml` is decomposed by concern:

- **`setup`** computes `SOURCE_DATE_EPOCH` from the git commit timestamp for reproducible builds; all building jobs read it via job outputs.
- **`nix`** runs `just check` + `nix build` end-to-end.
- **Single-run check jobs** (Debian trixie): `format-check`, `lint`, `docs`, `deps`, `coverage`. The latter three upload their outputs as artifacts.
- **`test` matrix** (4 distros × 4 profiles = 16 jobs): runs `just test <profile>` on each combination.
- **`package` matrix** (4 distros × 4 profiles = 16 jobs): produces per-component DEB/RPM/APK/pkg.tar.zst packages (~11 files each) plus monolithic TGZ.
- **`install-test` matrix** (7 jobs): verifies `apt install`/`dnf install`/`pacman -U`/`apk add`/TGZ extraction resolves dependencies correctly, runs the installed binary, and builds a `find_package` consumer project.
- **`static-binaries`** produces musl-static `csv2json-x86_64` / `json2csv-x86_64` binaries on Alpine; smoke-tested on alpine/debian/fedora/arch plus busybox (via `docker run`).

For local CI container builds, use `just ci-build <distro>` — runs `just check` inside a container matching that distro.

`guix.yml` is a separate manual-dispatch-only workflow (Guix bootstrap is slow; its clang-tools is incompatible with GCC 15's C++26 libstdc++ macros so it skips `just lint`).

## Architecture

**Libraries** (`libs/<name>/`): Each has `include/<name>/`, `src/`, and `tests/`. Public headers are exposed via `target_include_directories(... PUBLIC include)`. Each library has a CMake-generated export header (`<name>/<name>_export.hpp`) for shared library support. Public functions are annotated with `<NAME>_EXPORT`.

**Dependency graph:**
```
csv2json → convert → csv  → core
                     json → core

json2csv → csv  → core
           json → core
```

**Binaries** (`bin/<name>/`): Each links against libraries via `target_link_libraries(... PRIVATE ...)`. Binary integration tests use `libs/test_support/subprocess.hpp` to spawn the built binary and assert on its output.

**`libs/test_support/`**: Header-only INTERFACE library providing `subprocess::run()` for launching child processes with arbitrary args/env and capturing stdout, stderr, and exit code.

**`cmake/`**: Reusable modules — `Ccache.cmake` (auto-detects ccache), `Coverage.cmake` (lcov targets), `Sanitizers.cmake` (ASan/UBSan/TSan), `Version.cmake` (generates `scaffold/version.hpp` from project version), `Install.cmake` (CMake package config and export set), `Packaging.cmake` (CPack for TGZ/DEB/RPM), `scaffold-config.cmake.in` (package config template with COMPONENTS support).

## Compiler and Linker Settings

- **C++26** with GCC, `-Werror` and aggressive warnings (see root `CMakeLists.txt`)
- **Ninja** generator (set in `CMakePresets.json` base preset)
- **mold** linker with SHA-256 build IDs
- **ccache** auto-detected and used when available
- **`-ffile-prefix-map`** remaps source paths for reproducible builds (disabled for coverage)
- **C++ module scanning disabled** (`CMAKE_CXX_SCAN_FOR_MODULES OFF`) to keep compile_commands.json clean for clang-tidy
- **Release binaries are stripped** (`-s` linker flag in release preset)

## Reproducible Builds

Release builds are deterministic given the same toolchain. For full reproducibility, pin `SOURCE_DATE_EPOCH`:

```bash
SOURCE_DATE_EPOCH=0 just build release
```

## Adding a New Library

1. Create `libs/<name>/` with `include/<name>/`, `src/`, `tests/`
2. Add `CMakeLists.txt` with `add_library`, `target_include_directories(... PUBLIC include ${CMAKE_BINARY_DIR}/generated)`, `generate_export_header`, and test executable
3. Add `#include <<name>/<name>_export.hpp>` to the public header and annotate API with `<NAME>_EXPORT`
4. Add `add_subdirectory(libs/<name>)` to root `CMakeLists.txt`

## Adding a New Binary

1. Create `bin/<name>/` with `main.cpp` and optionally `tests/`
2. Add `CMakeLists.txt` with `add_executable`, `target_link_libraries`
3. For integration tests: link against `test_support`, define `<NAME>_EXECUTABLE="$<TARGET_FILE:<name>>"`, use `subprocess::run()`
4. Add `add_subdirectory(bin/<name>)` to root `CMakeLists.txt`
5. Add a VS Code launch configuration in `.vscode/launch.json`

## Adding a CI Container

1. Create `ci/<distro>/Dockerfile` — install toolchain, `COPY . .`, `RUN just check`
2. Add the distro name to the matrix in `.github/workflows/ci.yml`
3. Test with `just ci-build <distro>`

## Documentation Maintenance

After making architectural changes — adding, removing, or renaming files, directories, libraries, binaries, or CI targets — update the relevant `README.md` files and this `CLAUDE.md` to reflect the changes. Every directory has a `README.md` with a file/directory table that must stay in sync with the actual contents. The root `README.md` has a project layout table and architecture diagram that must also be kept current.
