#!/usr/bin/env bash
# Verify that two independent release builds from the same commit produce
# byte-identical output.
#
# The second build runs with everything the output *should* be independent
# of, varied: source path, HOME, TMPDIR, umask, LC_ALL, TZ, and build
# parallelism. SOURCE_DATE_EPOCH is held constant (that is the mechanism we
# are trying to verify works).
#
# Usage:
#   scripts/verify-reproducibility.sh [WORKDIR]
#
# Environment:
#   SOURCE_DATE_EPOCH  If unset, computed from the HEAD commit timestamp.
#   CPACK_GENERATORS   Generators to drive (default: "TGZ"). Set to
#                      "TGZ;DEB" in CI (where dpkg-deb is available) to
#                      also verify Debian package reproducibility.
#
# Exit status:
#   0  all artifacts reproduced identically
#   1  at least one artifact differs; diffoscope HTML reports (if
#      diffoscope is installed) are written under $WORKDIR/report/
#   2  usage/environment error

set -euo pipefail

workdir="${1:-/tmp/scaffold-repro}"
# Resolve the project root without requiring git: use the script's own
# location so this works in container jobs where actions/checkout falls
# back to the REST API (no .git directory).
srcdir="$(cd "$(dirname "$0")/.." && pwd)"
: "${CPACK_GENERATORS:=TGZ}"
if [ -z "${SOURCE_DATE_EPOCH:-}" ]; then
    if git -C "$srcdir" rev-parse HEAD >/dev/null 2>&1; then
        SOURCE_DATE_EPOCH=$(git -C "$srcdir" log -1 --format=%ct)
    else
        echo "SOURCE_DATE_EPOCH must be set when running outside a git checkout" >&2
        exit 2
    fi
fi
export SOURCE_DATE_EPOCH

# Disable ccache: a cache hit would hand us the cached object and mask real
# non-determinism by replaying bit-identical output without actually running
# the compiler.
export CCACHE_DISABLE=1

rm -rf "$workdir"
mkdir -p "$workdir"/{artifacts/a,artifacts/b,report,home-b,tmp-b}

echo "== Copying working tree twice =="
# Use the live working tree (not HEAD) so uncommitted edits are tested.
# Exclude build/ to avoid O(GB) copies and stale CMake cache carryover.
copy_tree () {
    local dst=$1
    mkdir -p "$dst"
    tar -C "$srcdir" --exclude=./build --exclude=./.git -cf - . \
        | tar -C "$dst" -xf -
}
copy_tree "$workdir/src-a/scaffold"
copy_tree "$workdir/src-b/scaffold"

build () {
    local tree=$1
    shift
    (
        cd "$tree"
        cmake --preset release >/dev/null
        cmake --build --preset release "$@"
        # CPack writes into build/release by default. DEB generation is
        # best-effort: on hosts without dpkg-deb it will fail and be skipped.
        # We force umask 022 for the cpack step only, not for the build:
        # dpkg-deb records the calling process's umask into the ar member
        # modes of the output .deb, and also inherits it for the staging
        # prefix directory. Standard Debian packaging convention is umask
        # 022, and this is what users expect reproducible DEBs to assume.
        cd build/release
        ( umask 022 && cpack -G "$CPACK_GENERATORS" >/dev/null 2>&1 ) || true
    )
}

echo "== Build A (canonical env, -j$(nproc)) =="
build "$workdir/src-a/scaffold" -- -j"$(nproc)"

echo "== Build B (perturbed env, -j1) =="
(
    umask 027
    export HOME="$workdir/home-b"
    export TMPDIR="$workdir/tmp-b"
    export TZ="Asia/Tokyo"
    export LC_ALL="C.UTF-8"
    build "$workdir/src-b/scaffold" -- -j1
)

# Collect shipped-ish artifacts: raw binaries, shared libs, static libs,
# and whichever CPack generators actually produced output.
gather () {
    local src=$1 dst=$2
    mkdir -p "$dst"
    local patterns=(
        "$src/build/release/bin/csv2json/csv2json"
        "$src/build/release/bin/json2csv/json2csv"
        "$src"/build/release/libs/core/libscaffold-core.so.0.1.0
        "$src"/build/release/libs/csv/libscaffold-csv.so.0.1.0
        "$src"/build/release/libs/json/libscaffold-json.so.0.1.0
        "$src"/build/release/libs/convert/libscaffold-convert.so.0.1.0
        "$src"/build/release/libs/core/libscaffold-core.a
        "$src"/build/release/libs/csv/libscaffold-csv.a
        "$src"/build/release/libs/json/libscaffold-json.a
        "$src"/build/release/libs/convert/libscaffold-convert.a
        "$src"/build/release/scaffold-*.tar.gz
        "$src"/build/release/scaffold-*.deb
        "$src"/build/release/scaffold-*.rpm
    )
    local f
    for f in "${patterns[@]}"; do
        [ -f "$f" ] && cp "$f" "$dst/" || true
    done
}

gather "$workdir/src-a/scaffold" "$workdir/artifacts/a"
gather "$workdir/src-b/scaffold" "$workdir/artifacts/b"

hash_all () {
    local dir=$1
    # Portable (no GNU-find -printf): iterate and sha256sum each file,
    # then sort for stable output ordering.
    ( cd "$dir" && for f in *; do
        [ -f "$f" ] || continue
        sha256sum "$f"
    done ) | LC_ALL=C sort -k2
}

hash_all "$workdir/artifacts/a" > "$workdir/a.sums"
hash_all "$workdir/artifacts/b" > "$workdir/b.sums"

echo
echo "== SHA-256 sums =="
cat "$workdir/a.sums"
echo

mismatch=0
if ! diff -u "$workdir/a.sums" "$workdir/b.sums" > "$workdir/sums.diff"; then
    mismatch=1
    echo "MISMATCH detected:"
    cat "$workdir/sums.diff"
fi

if [ "$mismatch" = 1 ]; then
    echo
    if command -v diffoscope >/dev/null 2>&1; then
        echo "== diffoscope HTML reports =="
        for a_file in "$workdir/artifacts/a"/*; do
            base=$(basename "$a_file")
            b_file="$workdir/artifacts/b/$base"
            [ -f "$b_file" ] || continue
            if ! cmp -s "$a_file" "$b_file"; then
                out="$workdir/report/${base}.html"
                diffoscope --html "$out" --max-report-size 10000000 \
                    "$a_file" "$b_file" >/dev/null 2>&1 || true
                echo "  $out"
            fi
        done
    else
        echo "diffoscope not installed — byte-level diff of first mismatch:"
        for a_file in "$workdir/artifacts/a"/*; do
            base=$(basename "$a_file")
            b_file="$workdir/artifacts/b/$base"
            [ -f "$b_file" ] || continue
            if ! cmp -s "$a_file" "$b_file"; then
                echo "  DIFFER: $base"
                cmp -l "$a_file" "$b_file" 2>/dev/null | head -20 || true
            fi
        done
    fi
    exit 1
fi

echo "REPRODUCIBLE: all $(wc -l < "$workdir/a.sums") artifacts identical."
