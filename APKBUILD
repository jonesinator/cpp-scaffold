# Maintainer: Scaffold Dev <scaffold-dev@example.com>
# Profile is selected via SCAFFOLD_PROFILE env var (release/debug/asan/tsan).
# The profile only controls which build tree we install from; package names
# stay the same so that different-profile APKs conflict on install (user picks one).
_profile="${SCAFFOLD_PROFILE:-release}"

pkgname=libscaffold-core
pkgver=0.1.0
pkgrel=0
pkgdesc="scaffold core runtime library"
url="https://example.com/scaffold"
arch="x86_64"
license="MIT"
depends=""
makedepends="cmake samurai mold"
source=""
builddir="$startdir"

subpackages="
    libscaffold-core-dev:_core_dev
    libscaffold-csv:_csv_lib
    libscaffold-csv-dev:_csv_dev
    libscaffold-json:_json_lib
    libscaffold-json-dev:_json_dev
    libscaffold-convert:_convert_lib
    libscaffold-convert-dev:_convert_dev
    libscaffold-dev:_umbrella_dev
    scaffold-csv2json:_csv2json_bin
    scaffold-json2csv:_json2csv_bin
"

build() {
    cd "$startdir"
    cmake --preset ${_profile} -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build --preset ${_profile}
}

check() {
    cd "$startdir"
    ctest --preset ${_profile}
}

# Main package: install everything; subpackages amove their files out.
# What remains in $pkgdir becomes libscaffold-core (the core runtime .so files).
package() {
    cd "$startdir"
    DESTDIR="$pkgdir" cmake --install build/${_profile}
}

_core_dev() {
    pkgdesc="scaffold core development files"
    depends="libscaffold-core=${pkgver}-r${pkgrel}"
    amove usr/include/core usr/lib/libscaffold-core.so usr/lib/libscaffold-core.a
}

_csv_lib() {
    pkgdesc="scaffold csv runtime library"
    depends="libscaffold-core=${pkgver}-r${pkgrel}"
    amove usr/lib/libscaffold-csv.so.*
}

_csv_dev() {
    pkgdesc="scaffold csv development files"
    depends="libscaffold-csv=${pkgver}-r${pkgrel} libscaffold-core-dev=${pkgver}-r${pkgrel}"
    amove usr/include/csv usr/lib/libscaffold-csv.so usr/lib/libscaffold-csv.a
}

_json_lib() {
    pkgdesc="scaffold json runtime library"
    depends="libscaffold-core=${pkgver}-r${pkgrel}"
    amove usr/lib/libscaffold-json.so.*
}

_json_dev() {
    pkgdesc="scaffold json development files"
    depends="libscaffold-json=${pkgver}-r${pkgrel} libscaffold-core-dev=${pkgver}-r${pkgrel}"
    amove usr/include/json usr/lib/libscaffold-json.so usr/lib/libscaffold-json.a
}

_convert_lib() {
    pkgdesc="scaffold convert runtime library"
    depends="libscaffold-csv=${pkgver}-r${pkgrel} libscaffold-json=${pkgver}-r${pkgrel}"
    amove usr/lib/libscaffold-convert.so.*
}

_convert_dev() {
    pkgdesc="scaffold convert development files"
    depends="libscaffold-convert=${pkgver}-r${pkgrel} libscaffold-csv-dev=${pkgver}-r${pkgrel} libscaffold-json-dev=${pkgver}-r${pkgrel}"
    amove usr/include/convert usr/lib/libscaffold-convert.so usr/lib/libscaffold-convert.a
}

_umbrella_dev() {
    pkgdesc="scaffold umbrella dev package (CMake integration)"
    depends="
        libscaffold-core-dev=${pkgver}-r${pkgrel}
        libscaffold-csv-dev=${pkgver}-r${pkgrel}
        libscaffold-json-dev=${pkgver}-r${pkgrel}
        libscaffold-convert-dev=${pkgver}-r${pkgrel}
    "
    amove usr/include/scaffold usr/lib/cmake/scaffold
}

_csv2json_bin() {
    pkgdesc="scaffold csv2json converter"
    depends="libscaffold-convert=${pkgver}-r${pkgrel}"
    amove usr/bin/csv2json
}

_json2csv_bin() {
    pkgdesc="scaffold json2csv converter"
    depends="libscaffold-csv=${pkgver}-r${pkgrel} libscaffold-json=${pkgver}-r${pkgrel}"
    amove usr/bin/json2csv
}
