#!/usr/bin/env bash
# Verify that two independent release builds produce byte-identical output,
# using Debian's reprotest. SOURCE_DATE_EPOCH is held constant; reprotest
# varies build_path, HOME, umask, TZ, LC_ALL, file ordering (disorderfs),
# environment variables, exec_path, wallclock time (faketime), and hostname
# (if `domainname` is available) between the two builds.
#
# Usage:
#   scripts/verify-reproducibility.sh [--verbose]
#
# Environment:
#   SOURCE_DATE_EPOCH      If unset, computed from the HEAD commit timestamp.
#   CPACK_GENERATORS       Generators to drive (default: "TGZ;DEB" if
#                          dpkg-deb is present, else "TGZ").
#   REPROTEST_VARIATIONS   Override the default variation set. See
#                          `man reprotest` / its --variations docs.
#
# Exit status:
#   0    all artifacts reproduced identically
#   1    reprotest reported at least one diff; report is on stdout
#   2    usage / environment error
#   127  reprotest (or a required dependency) not installed

set -euo pipefail

if ! command -v reprotest >/dev/null 2>&1; then
    cat >&2 <<'EOF'
reprotest not installed. Install via:
  Debian/Ubuntu:  apt install reprotest disorderfs
  Fedora:         dnf install reprotest disorderfs
  Arch:           pacman -S reprotest disorderfs
EOF
    exit 127
fi

srcdir="$(cd "$(dirname "$0")/.." && pwd)"
cd "$srcdir"

# Default SOURCE_DATE_EPOCH from git HEAD if available.
if [ -z "${SOURCE_DATE_EPOCH:-}" ]; then
    if git rev-parse HEAD >/dev/null 2>&1; then
        SOURCE_DATE_EPOCH=$(git log -1 --format=%ct)
    else
        echo "SOURCE_DATE_EPOCH must be set outside a git checkout" >&2
        exit 2
    fi
fi
export SOURCE_DATE_EPOCH

# Default CPACK_GENERATORS: include DEB only if dpkg-deb is present.
if [ -z "${CPACK_GENERATORS:-}" ]; then
    if command -v dpkg-deb >/dev/null 2>&1; then
        CPACK_GENERATORS="TGZ;DEB"
    else
        CPACK_GENERATORS="TGZ"
    fi
fi
export CPACK_GENERATORS

# Default variation set. Skipped variations (with reasoning):
#   -kernel:      requires a different host kernel (impractical in CI)
#   -num_cpus:    interacts badly with disorderfs on some systems
#                 (our project is too small for parallel-link races anyway)
#   -user_group:  reprotest silently no-ops this without additional user
#                 accounts configured via --variations=user_group.available+=
# Note on aslr: counterintuitively we keep it ENABLED (+aslr). When disabled,
# reprotest appends `-R` to the setarch wrapper to forcibly disable ASLR,
# which calls personality(2) and fails with EPERM under the default Docker
# seccomp profile in CI. Enabling aslr means "leave it alone" in reprotest's
# model — no personality() syscall needed. ASLR doesn't affect build output,
# so either value is correct from a reproducibility standpoint.
# domain_host is enabled only if the `domainname` binary is available.
# (Debian ships it in the `hostname` package; Arch only has it via AUR.)
default_variations="+aslr,+build_path,+environment,+exec_path,+fileordering"
default_variations+=",+home,+locales,+time,+timezone,+umask"
if command -v domainname >/dev/null 2>&1; then
    default_variations+=",+domain_host"
else
    echo "note: 'domainname' not found; disabling domain_host variation" >&2
    echo "      (install the 'hostname'/'nis' package for full coverage)" >&2
fi
: "${REPROTEST_VARIATIONS:=-all,$default_variations}"

# Stage a clean copy of the source tree so reprotest's whole-tree copy
# skips build/, .git/, .cache/, .claude/ — they'd make the copy huge and
# CMakeCache.txt pointing at the original source dir breaks the build.
stage=$(mktemp -d -t scaffold-repro-stage.XXXXXX)
trap 'rm -rf "$stage"' EXIT
tar -C "$srcdir" \
    --exclude=./build \
    --exclude=./.git \
    --exclude=./.cache \
    --exclude=./.claude \
    -cf - . | tar -C "$stage" -xf -

cd "$stage"

# The build command: cmake build + cpack (under umask 022, since dpkg-deb
# records umask into .deb ar-member modes) + copy artifacts into a single
# directory reprotest can compare with one glob.
build_cmd='
    cmake --preset release >/dev/null &&
    cmake --build --preset release 2>&1 | tail -2 &&
    (cd build/release && umask 022 && cpack -G "'"$CPACK_GENERATORS"'" >/dev/null) &&
    mkdir -p build/release/_repro-artifacts &&
    cp build/release/scaffold-*.tar.gz \
       build/release/bin/csv2json/csv2json \
       build/release/bin/json2csv/json2csv \
       build/release/libs/core/libscaffold-core.so.0.1.0 \
       build/release/libs/csv/libscaffold-csv.so.0.1.0 \
       build/release/libs/json/libscaffold-json.so.0.1.0 \
       build/release/libs/convert/libscaffold-convert.so.0.1.0 \
       build/release/libs/core/libscaffold-core.a \
       build/release/libs/csv/libscaffold-csv.a \
       build/release/libs/json/libscaffold-json.a \
       build/release/libs/convert/libscaffold-convert.a \
       build/release/_repro-artifacts/ &&
    if ls build/release/scaffold-*.deb >/dev/null 2>&1; then
        cp build/release/scaffold-*.deb build/release/_repro-artifacts/
    fi
'

verbose_flag=()
if [ "${1:-}" = "--verbose" ]; then
    verbose_flag=(--verbose)
fi

exec reprotest \
    "${verbose_flag[@]}" \
    --variations="$REPROTEST_VARIATIONS" \
    "$build_cmd" \
    'build/release/_repro-artifacts/*'
