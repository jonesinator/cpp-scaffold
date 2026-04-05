# scripts/

Development scripts and git hooks. Install with `just setup`.

| File | Description |
|---|---|
| [pre-commit](pre-commit) | Git pre-commit hook that runs `just format-check` to block commits with formatting violations |
| [verify-reproducibility.sh](verify-reproducibility.sh) | Wraps Debian's `reprotest`: stages a clean source tree, builds twice under perturbed variations (build_path, environment, fileordering, hostname, locales, time, TZ, umask, etc.), and diffs every artifact via `diffoscope`. Used by `just verify-reproducibility` and the `reproducibility` CI job. Override defaults via `REPROTEST_VARIATIONS`, `CPACK_GENERATORS`. |
