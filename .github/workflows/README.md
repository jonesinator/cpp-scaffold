# .github/workflows/

GitHub Actions workflow definitions.

| File | Description |
|---|---|
| [ci.yml](ci.yml) | Main CI pipeline — runs `just check` on Nix, Guix, and container matrix (Trixie, Fedora, Alpine, Arch); builds Nix and Guix packages; builds distribution packages (DEB, RPM, TGZ, pkg.tar.zst) and runs install tests on clean containers |
