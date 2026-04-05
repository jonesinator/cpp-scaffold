# scaffold

A modern C++26 project scaffold with multi-library architecture, comprehensive tooling, and cross-platform CI. Designed as a starting point for large C++ projects that need rigorous build infrastructure from day one.

## What This Is

This repository is a fully functional project template — not a library or application. It demonstrates and validates a complete C++ development workflow: building, testing, static analysis, code coverage, documentation generation, reproducible builds, packaging, and continuous integration across multiple Linux distributions.

The example code (libraries `core`, `csv`, `json`, `convert` and binaries `csv2json`, `json2csv`) exists solely to exercise every part of the build infrastructure. Replace it with your own code; the scaffolding around it is the point.

## Features

### Build System

- **C++26** with GCC 15 and aggressive `-Werror` warnings
- **CMake 3.30+** with named presets for all build configurations
- **Ninja** generator for fast parallel builds
- **mold** linker with SHA-256 content-addressed build IDs
- **ccache** auto-detected and used transparently
- **Shared library support** with per-library CMake export headers and visibility control
- **Generated version header** stamped from `CMakeLists.txt` project version

### Build Profiles

| Profile | Purpose |
|---|---|
| `debug` | Development builds with full debug symbols for GDB |
| `release` | Optimized, stripped binaries |
| `coverage` | Debug build instrumented for lcov code coverage |
| `asan` | Debug build with AddressSanitizer + UndefinedBehaviorSanitizer |
| `tsan` | Debug build with ThreadSanitizer |

### Testing

- **No test framework** — tests are plain C++ executables returning `EXIT_SUCCESS` or `EXIT_FAILURE`, run via ctest
- **Library unit tests** capture stdout and assert on output
- **Binary integration tests** use a subprocess runner (`test_support`) to launch the actual compiled binary with arbitrary arguments and environment, then verify stdout, stderr, and exit code
- **Sanitizer testing** runs the full test suite under ASan+UBSan and TSan to catch memory errors and data races

### Static Analysis and Formatting

- **clang-format** enforces a 120-column, Allman-brace style
- **clang-tidy** runs an aggressive set of checks from bugprone, cert, cppcoreguidelines, hicpp, misc, modernize, performance, portability, and readability — all treated as errors
- **Pre-commit hook** blocks commits that violate formatting rules

### Documentation

- **Doxygen** generates HTML and PDF documentation with call graphs, include graphs, and directory dependency diagrams (via Graphviz)
- **CMake dependency graph** exported as SVG via `cmake --graphviz`

### Packaging

The project supports installation via `cmake --install` and packaging for multiple distributions with **per-component Debian-style splits**. Each library ships as both a runtime shared library (`.so`) and a static archive (`.a`) — consumers pick their preferred link type.

**Per-component package structure:**

| Package | Contents |
|---|---|
| `libscaffold-<lib>` (runtime) | `libscaffold-<lib>.so.0` shared library |
| `libscaffold-<lib>-dev` | `libscaffold-<lib>.so` symlink + `libscaffold-<lib>.a` + headers |
| `libscaffold-dev` | Umbrella dev package with CMake config (`find_package(scaffold)`) |
| `scaffold-csv2json`, `scaffold-json2csv` | Binaries, depend on the runtime libs |

Each profile produces 11 packages per distro. Packaging formats:

- **CPack** generates per-component DEB, RPM packages + monolithic TGZ
- **`abuild`** splits `APKBUILD` into subpackages (`.apk`)
- **`makepkg`** uses `PKGBUILD` split-packages array (`.pkg.tar.zst`)
- **Nix** — `nix build` produces a Nix derivation from [flake.nix](flake.nix)
- **Guix** — `guix build -f package.scm` produces a Guix package from [package.scm](package.scm)

`find_package(scaffold COMPONENTS core csv json convert)` links against `scaffold::<name>` (shared) or `scaffold::<name>_static`.

**Static binaries:** for download-and-run usage, the `static` CMake preset builds fully musl-linked binaries (`csv2json-x86_64`, `json2csv-x86_64`) that run on any Linux distribution regardless of libc. Built on Alpine; smoke-tested against Alpine, Debian, Fedora, Arch, and busybox in CI.

**SBOMs** (Software Bill of Materials): CI generates SPDX-JSON SBOMs via [syft](https://github.com/anchore/syft) for every published artifact:

- **Per-package SBOMs** — one per `.deb`/`.rpm`/`.apk`/`.pkg.tar.zst`/`.tar.gz` file
- **Build-environment SBOMs** — one per distro × profile combination, capturing the toolchain state
- **Static binary SBOMs** — shipped alongside each static binary
- **Source SBOM** — one covering the repository source tree, plus runner image metadata

The outer GitHub Actions runner isn't SBOM-scanned by us — GitHub publishes runner image SBOMs at [actions/runner-images](https://github.com/actions/runner-images/releases); the `scaffold-source-sbom` artifact records which runner version was used.

**SLSA build provenance**: every published package and static binary is accompanied by a [SLSA v1 provenance attestation](https://slsa.dev/) generated via `actions/attest-build-provenance`. Consumers can verify an artifact came from this repo's CI with:

```bash
gh attestation verify path/to/artifact --repo owner/scaffold
```

**CVE scanning**: every CI run scans all generated SBOMs with [Grype](https://github.com/anchore/grype) against public vulnerability databases. The build fails on critical-severity CVEs; full JSON reports are uploaded as the `scaffold-cve-reports` artifact for every severity level.

**Releases**: Pushing a git tag matching `v*` (e.g., `v0.2.0`) triggers the release job, which downloads all CI artifacts, generates a SHA256SUMS manifest, and creates a GitHub Release with all package files + SBOMs + static binaries attached.

**Supply-chain pinning**: All GitHub Actions are pinned to commit SHAs (with version comments), all container images are pinned to SHA256 digests, and the Ubuntu runner is pinned to `ubuntu-24.04`. [Dependabot](.github/dependabot.yml) updates these weekly.

### Reproducible Builds

Release builds are deterministic given the same toolchain:

- `-ffile-prefix-map` remaps absolute source paths to `.`
- mold linker produces content-addressed SHA-256 build IDs
- `SOURCE_DATE_EPOCH` can be set to pin `__DATE__`/`__TIME__` macros
- `CMAKE_BUILD_RPATH_USE_ORIGIN=ON` rewrites build-tree `DT_RUNPATH` to `$ORIGIN`-relative so the source directory doesn't leak into ELF binaries
- `CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS` + a post-install `ALL_COMPONENTS` chmod force 0755 on every staged directory, so packaging is independent of the build's umask
- Stripped release binaries contain no debug symbols or local path information

**Verification:** `just verify-reproducibility` (or [`scripts/verify-reproducibility.sh`](scripts/verify-reproducibility.sh)) wraps Debian's [`reprotest`](https://salsa.debian.org/reproducible-builds/reprotest). It stages a clean source tree, runs the build twice under systematically perturbed environments (`build_path`, `environment`, `exec_path`, `fileordering` via disorderfs, `home`, `locales`, `time`, `timezone`, `umask`, and `domain_host`/hostname where available), and diffs every artifact with `diffoscope`. Covers binaries, shared/static libraries, the TGZ, and all 12 per-component DEBs. The [`reproducibility` CI job](.github/workflows/ci.yml) runs this against `trixie-slim` on every push. See [CLAUDE.md § Reproducible Builds](CLAUDE.md) for the list of variations intentionally left off.

### Development Environments

Four equivalent, fully validated environments:

| Environment | Entry | Pinned |
|---|---|---|
| Native | Direct (tools installed locally) | No |
| [Nix flake](flake.nix) | `nix develop` | Yes (`flake.lock`) |
| [Guix manifest](manifest.scm) | `guix shell -m manifest.scm` | Via Guix time-machine |
| [Dev container](.devcontainer/) | VS Code Dev Containers extension | Via Dockerfile |

The Nix flake and Guix manifest provide fully reproducible, hermetic toolchains. The dev container provides a consistent environment for VS Code users. All four environments include the same tools: GCC, CMake, Ninja, mold, ccache, clang-format, clang-tidy, lcov, doxygen, Graphviz, texlive, just, and GDB.

### Continuous Integration

GitHub Actions runs a decomposed-by-concern pipeline:

- **Single-run check jobs** (Debian trixie): `format-check`, `lint`, `docs`, `deps`, `coverage` — the latter three upload their outputs as artifacts.
- **`test` matrix** (16 jobs): 4 distros × 4 profiles (debug/release/asan/tsan).
- **`package` matrix** (16 jobs): per-component DEB/RPM/APK/pkg.tar.zst packages plus monolithic TGZ for each distro × profile.
- **`install-test` matrix** (7 jobs): verifies native and TGZ install on clean containers, tests the binary plus `find_package` consumer integration.
- **Nix**: validates the Nix flake with `nix build` + smoke-test.
- **Static binaries**: musl-static `csv2json` and `json2csv` for x86_64, smoke-tested on alpine/debian/fedora/arch/busybox.
- **Guix** (manual-dispatch): in a separate workflow, validates the Guix package definition.

All building jobs use `SOURCE_DATE_EPOCH` computed from the git commit timestamp, making builds reproducible per commit.

## Task Runner

All common operations are available as [just](https://github.com/casey/just) recipes. Run `just` with no arguments to see the full list:

```
just build [profile]        Build the project
just test [profile]         Build + run all tests
just test-one <name>        Run a single test by name
just coverage               Tests + HTML coverage report
just format                 Format all source files
just format-check           Check formatting (no modifications)
just lint                   Run clang-tidy
just docs                   Generate HTML + PDF documentation
just deps                   Generate CMake dependency graph as SVG
just install [profile]      Install to system prefix (release default)
just install-to <prefix>    Install to a custom prefix
just package [profile]      Create distribution packages (TGZ, DEB, RPM)
just check                  Full validation across all profiles
just ci-build <distro>      Build a CI container (trixie, fedora, alpine, arch)
just setup                  Install pre-commit hooks
just open-coverage          Generate + open coverage report
just open-docs              Generate + open HTML docs
just open-deps              Generate + open dependency graph
just clean                  Remove all build artifacts
```

Profiles: `debug` (default), `release`, `coverage`, `asan`, `tsan`, `clang-debug` (clang portability canary; CI-only).

## Architecture

```
libs/core       ← foundation (no dependencies)
libs/csv        ← depends on core
libs/json       ← depends on core
libs/convert    ← depends on csv + json (diamond on core)
bin/csv2json    ← links convert (transitively: csv, json, core)
bin/json2csv    ← links csv + json directly
```

Libraries expose public headers via `include/<name>/` and use CMake-generated export macros (`<NAME>_EXPORT`) for shared library support. Binary integration tests use `libs/test_support/` to spawn the compiled executable and assert on its behavior.

## Project Layout

| Path | Description |
|---|---|
| [libs/](libs/) | Libraries — each with public headers, source, tests, and export headers |
| [bin/](bin/) | Executables — each links against one or more libraries |
| [cmake/](cmake/) | Reusable CMake modules (ccache, coverage, sanitizers, version, install, packaging) |
| [ci/](ci/) | CI container Dockerfiles and install-test consumer project |
| [scripts/](scripts/) | Git hooks and development scripts |
| [.devcontainer/](.devcontainer/) | VS Code Dev Container configuration |
| [.vscode/](.vscode/) | VS Code workspace settings (debug, build tasks) |
| [.github/](.github/) | GitHub Actions CI workflow |
| [CMakeLists.txt](CMakeLists.txt) | Root build definition |
| [CMakePresets.json](CMakePresets.json) | Named build/test presets for all profiles |
| [justfile](justfile) | Task runner recipes |
| [flake.nix](flake.nix) | Nix development environment and package definition |
| [flake.lock](flake.lock) | Pinned Nix dependency versions |
| [manifest.scm](manifest.scm) | Guix development environment |
| [package.scm](package.scm) | Guix package definition |
| [PKGBUILD](PKGBUILD) | Arch Linux package definition |
| [APKBUILD](APKBUILD) | Alpine Linux package definition |
| [Doxyfile](Doxyfile) | Doxygen documentation configuration |
| [.clang-format](.clang-format) | Code formatting rules (120 columns, Allman braces) |
| [.clang-tidy](.clang-tidy) | Static analysis checks and naming conventions |
| [.clangd](.clangd) | Language server configuration (compilation database path) |
| [.editorconfig](.editorconfig) | Editor-agnostic whitespace and encoding settings |
| [.gitattributes](.gitattributes) | Line ending normalization and binary file markers |
| [.gitignore](.gitignore) | Ignored paths (build artifacts, user presets, package outputs) |
| [.dockerignore](.dockerignore) | Excluded paths for container builds |
| [LICENSE](LICENSE) | MIT license |
| [CLAUDE.md](CLAUDE.md) | Guidance for Claude Code AI assistant |

## Getting Started

```bash
# Clone and enter the repository
git clone <url>
cd scaffold

# Option A: Use native tools (Arch Linux)
just setup          # install pre-commit hook
just test           # build + test
just check          # full validation

# Option B: Use Nix
nix develop
just check

# Option C: Use Guix
guix shell -m manifest.scm
just check

# Option D: Use Dev Container
# Open in VS Code → "Reopen in Container"
```

## Extending the Scaffold

To add a new library, binary, or CI target, see the relevant section in [CLAUDE.md](CLAUDE.md).

## License

MIT — see [LICENSE](LICENSE).
