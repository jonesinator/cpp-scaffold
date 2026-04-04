FROM alpine:edge

RUN apk add --no-cache \
    # Build toolchain
    g++ \
    cmake \
    samurai \
    ccache \
    mold \
    # Static analysis and formatting
    clang-extra-tools \
    # Testing and coverage
    lcov \
    # Documentation
    doxygen \
    graphviz \
    texlive \
    texlive-latexrecommended \
    texlive-latexextra \
    texlive-plaingeneric \
    texlive-most \
    # Task runner
    just \
    # Misc
    make \
    gzip \
    perl

WORKDIR /src
COPY . .

RUN just check
