# ci/

Dockerfiles for CI container builds and install-test resources. Each distro subdirectory targets a different Linux distribution. All containers install the full toolchain, copy the source, and run `just check`.

Build any container with: `just ci-build <distro>`

| Directory | Base image | libc | Notes |
|---|---|---|---|
| [trixie/](trixie/) | `debian:trixie-slim` | glibc | Debian testing, GCC 14 |
| [fedora/](fedora/) | `fedora:latest` | glibc | Fedora, GCC 15, requires explicit sanitizer runtime packages |
| [alpine/](alpine/) | `alpine:edge` | musl | Alpine with samurai (Ninja-compatible), catches glibc-specific assumptions |
| [arch/](arch/) | `archlinux:latest` | glibc | Arch Linux rolling release, matches the primary dev environment |
| [install-test/](install-test/) | — | — | Minimal consumer project used by CI to verify `find_package(scaffold)` works after package installation |
