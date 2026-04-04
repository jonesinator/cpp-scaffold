{
  description = "scaffold – C++26 project development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};

      gcc = pkgs.gcc15;

      texlive = pkgs.texliveBasic.withPackages (ps: with ps; [
        collection-latexrecommended
        collection-latexextra
        collection-fontsrecommended
        collection-plaingeneric
        wasysym
      ]);
    in
    {
      packages.${system}.default = pkgs.gcc15Stdenv.mkDerivation {
        pname = "scaffold";
        version = "0.1.0";

        src = ./.;

        nativeBuildInputs = with pkgs; [
          cmake
          ninja
          mold
        ];

        cmakeFlags = [
          "-DCMAKE_BUILD_TYPE=Release"
          "-DCMAKE_EXE_LINKER_FLAGS_RELEASE=-s"
          "-DCMAKE_SHARED_LINKER_FLAGS_RELEASE=-s"
        ];

        doCheck = true;

        meta = {
          description = "scaffold project";
          platforms = [ system ];
        };
      };

      devShells.${system}.default = pkgs.mkShell.override { stdenv = pkgs.gcc15Stdenv; } {
        packages = with pkgs; [
          # Build toolchain
          gcc
          cmake
          ninja
          ccache
          mold

          # Static analysis, formatting, and language server
          clang-tools

          # Testing and coverage
          lcov

          # Documentation
          doxygen
          graphviz
          texlive

          # Task runner
          just

          # Debugging
          gdb

          # Version control
          git
        ];

        shellHook = ''
          echo "scaffold dev shell — GCC $(g++ -dumpversion), CMake $(cmake --version | head -1 | awk '{print $3}')"
        '';
      };
    };
}
