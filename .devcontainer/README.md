# .devcontainer/

VS Code Dev Container configuration. Provides a Debian Trixie-based development environment with the full toolchain, debugger, and clangd language server.

| File | Description |
|---|---|
| [Dockerfile](Dockerfile) | Container image based on `debian:trixie-slim` with g++, cmake, ninja, mold, ccache, clangd, clang-format, clang-tidy, lcov, doxygen, graphviz, texlive, just, gdb, and gdbserver |
| [devcontainer.json](devcontainer.json) | VS Code integration — installs clangd/cmake-tools/cpptools extensions, runs `just configure` on creation |
