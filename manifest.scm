;; Guix development environment for the scaffold project.
;; Usage: guix shell -m manifest.scm

(specifications->manifest
 (list
  ;; Build toolchain
  "gcc-toolchain@15"
  "cmake"
  "ninja"
  "ccache"
  "mold"

  ;; Static analysis, formatting, and language server
  "clang-toolchain"

  ;; Testing and coverage
  "lcov"

  ;; Documentation
  "doxygen"
  "graphviz"
  "texlive-collection-latexrecommended"
  "texlive-collection-latexextra"
  "texlive-collection-fontsrecommended"
  "texlive-collection-plaingeneric"
  "texlive-wasysym"

  ;; Task runner
  "just"

  ;; Debugging
  "gdb"

  ;; Version control
  "git"))
