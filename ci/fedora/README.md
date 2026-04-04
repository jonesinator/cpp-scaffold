# ci/fedora/

CI container for Fedora (latest).

| File | Description |
|---|---|
| [Dockerfile](Dockerfile) | Installs toolchain via dnf (including libasan/libtsan/libubsan runtimes), copies source, runs `just check` |
