# ci/

Per-distro Dockerfiles used for **local CI container builds** (via `just ci-build <distro>`), and the install-test consumer project. Each Dockerfile installs the full toolchain and runs `just check` end-to-end — useful for reproducing CI failures locally.

**Note:** the actual GitHub Actions workflow does NOT use these Dockerfiles. It uses base distro images directly with explicit, decomposed steps (see [.github/workflows/ci.yml](../.github/workflows/ci.yml)). The Dockerfiles here are for local debugging convenience.

Build any container locally with: `just ci-build <distro>`

| Directory | Base image | libc | Notes |
|---|---|---|---|
| [trixie/](trixie/) | `debian:trixie-slim` | glibc | Debian testing, GCC 14 |
| [fedora/](fedora/) | `fedora:latest` | glibc | Fedora, GCC 15, requires explicit sanitizer runtime packages |
| [alpine/](alpine/) | `alpine:edge` | musl | Alpine with samurai (Ninja-compatible), catches glibc-specific assumptions |
| [arch/](arch/) | `archlinux:latest` | glibc | Arch Linux rolling release, matches the primary dev environment |
| [install-test/](install-test/) | — | — | Minimal consumer project used by CI to verify `find_package(scaffold)` works after package installation |
