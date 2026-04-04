FROM fedora:latest

RUN dnf install -y --setopt=install_weak_deps=False \
    # Build toolchain
    gcc-c++ \
    cmake \
    ninja-build \
    ccache \
    mold \
    # Sanitizer runtimes
    libasan \
    libtsan \
    libubsan \
    # Static analysis and formatting
    clang-tools-extra \
    # Testing and coverage
    lcov \
    # Documentation
    doxygen \
    graphviz \
    texlive-collection-latexrecommended \
    texlive-collection-latexextra \
    texlive-collection-fontsrecommended \
    texlive-collection-plaingeneric \
    texlive-wasysym \
    # Task runner
    just \
    # Misc
    make \
    && dnf clean all

WORKDIR /src
COPY . .

RUN just check
