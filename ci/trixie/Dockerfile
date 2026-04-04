FROM debian:trixie-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build toolchain
    g++ \
    cmake \
    ninja-build \
    ccache \
    mold \
    # Static analysis and formatting
    clang-format \
    clang-tidy \
    # Testing and coverage
    lcov \
    # Documentation
    doxygen \
    graphviz \
    texlive-latex-base \
    texlive-latex-recommended \
    texlive-latex-extra \
    texlive-fonts-recommended \
    texlive-plain-generic \
    # Task runner
    just \
    # Misc
    make \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

RUN just check
