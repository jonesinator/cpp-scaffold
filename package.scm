;; Guix package definition for the scaffold project.
;; Usage: guix build -f package.scm

(use-modules (guix packages)
             (guix gexp)
             (guix build-system cmake)
             ((guix licenses) #:prefix license:)
             (gnu packages)
             (gnu packages build-tools))

(package
  (name "scaffold")
  (version "0.1.0")
  (source (local-file "." "scaffold-checkout"
                       #:recursive? #t
                       #:select? (lambda (file stat)
                                   (not (or (string-contains file "/build/")
                                            (string-contains file "/.git/"))))))
  (build-system cmake-build-system)
  (arguments
   (list
    #:build-type "Release"
    #:configure-flags
    #~(list "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-s"
            "-DCMAKE_SHARED_LINKER_FLAGS_RELEASE=-s")))
  (native-inputs
   (list (specification->package "gcc-toolchain@15")
         (specification->package "ninja")
         (specification->package "mold")))
  (home-page "https://example.com/scaffold")
  (synopsis "A modern C++26 project scaffold")
  (description "A fully functional C++26 project template with multi-library
architecture, comprehensive tooling, and cross-platform CI.")
  (license license:expat))
