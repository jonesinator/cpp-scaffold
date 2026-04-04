# .github/workflows/

GitHub Actions workflow definitions.

| File | Description |
|---|---|
| [ci.yml](ci.yml) | Main CI pipeline — runs on push to main, pull requests, and manual dispatch. Decomposed check jobs (format-check, lint, docs, deps, coverage), per-distro test jobs, per-distro package + install-test matrix, and Nix build |
| [guix.yml](guix.yml) | Manual-dispatch-only Guix check + package build. Split out because the Guix toolchain bootstrap is very slow |
