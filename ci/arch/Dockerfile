FROM archlinux:latest

RUN pacman -Syu --noconfirm --needed \
    # Build toolchain
    gcc \
    cmake \
    ninja \
    ccache \
    mold \
    # Static analysis and formatting
    clang \
    # Testing and coverage
    lcov \
    # Documentation
    doxygen \
    graphviz \
    texlive-basic \
    texlive-latexrecommended \
    texlive-latexextra \
    texlive-fontsrecommended \
    texlive-plaingeneric \
    # Task runner
    just \
    # Misc
    make \
    && pacman -Scc --noconfirm

WORKDIR /src
COPY . .

RUN just check
