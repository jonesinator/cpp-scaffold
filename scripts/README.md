# scripts/

Development scripts and git hooks. Install with `just setup`.

| File | Description |
|---|---|
| [pre-commit](pre-commit) | Git pre-commit hook that runs `just format-check` to block commits with formatting violations |
| [verify-reproducibility.sh](verify-reproducibility.sh) | Double-builds the release target with a perturbed environment and SHA-256-diffs the outputs. Used by `just verify-reproducibility` and the `reproducibility` CI job. Set `CPACK_GENERATORS="TGZ;DEB"` to also verify Debian packages. |
