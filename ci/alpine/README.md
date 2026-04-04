# ci/alpine/

CI container for Alpine Linux (edge). Uses musl libc instead of glibc, which catches portability issues.

| File | Description |
|---|---|
| [Dockerfile](Dockerfile) | Installs toolchain via apk (including gzip/perl for lcov), copies source, runs `just check` |
