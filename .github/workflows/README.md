# .github/workflows/

GitHub Actions workflow definitions.

| File | Description |
|---|---|
| [ci.yml](ci.yml) | Main CI pipeline — triggered on push to main, pull requests, and manual dispatch |
| [guix.yml](guix.yml) | Manual-dispatch-only Guix check + package build. Split out because the Guix toolchain bootstrap is very slow and its clang-tools is incompatible with GCC 15's C++26 libstdc++ macros |

## ci.yml structure

| Job | Scope |
|---|---|
| `setup` | Computes `SOURCE_DATE_EPOCH` from the git commit timestamp; exposed to all building jobs for reproducible builds |
| `nix` | `just check` + `nix build` + binary smoke-test |
| `format-check`, `lint`, `docs`, `deps`, `coverage` | Single-run concerns (Debian trixie); `docs`/`deps`/`coverage` upload their outputs as artifacts |
| `test` | 4 distros × 4 profiles = **16 jobs** running `just test <profile>` |
| `package` | 4 distros × 4 profiles = **16 jobs** producing per-component DEB/RPM/APK/pkg.tar.zst packages (~11 files each) + monolithic TGZ |
| `install-test` | 4 distros × native+TGZ formats = 7 jobs verifying package install + `find_package` consumer |
| `static-binaries` | Produces musl-static `csv2json-x86_64` / `json2csv-x86_64` binaries |
| `static-binaries-smoke-test` | Runs static binary on Alpine/Debian/Fedora/Arch |
| `static-binaries-busybox-test` | Runs static binary inside busybox via `docker run` |
