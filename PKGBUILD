# Maintainer: scaffold-dev@example.com
# Profile is selected via SCAFFOLD_PROFILE env var (release/debug/asan/tsan).
# Package names stay the same across profiles; the profile only controls
# which build tree is used. Different-profile pkgs conflict on install.
_profile="${SCAFFOLD_PROFILE:-release}"

pkgbase=scaffold
pkgname=(
    'libscaffold-core'
    'libscaffold-core-dev'
    'libscaffold-csv'
    'libscaffold-csv-dev'
    'libscaffold-json'
    'libscaffold-json-dev'
    'libscaffold-convert'
    'libscaffold-convert-dev'
    'libscaffold-dev'
    'scaffold-csv2json'
    'scaffold-json2csv'
)
pkgver=0.1.0
pkgrel=1
arch=('x86_64')
url="https://example.com/scaffold"
license=('MIT')
makedepends=('cmake' 'ninja' 'mold' 'gcc')
options=('staticlibs' '!lto' '!strip' '!debug')
source=()

build() {
    cd "$startdir"
    cmake --preset ${_profile} -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build --preset ${_profile}
}

check() {
    cd "$startdir"
    ctest --preset ${_profile}
}

package_libscaffold-core() {
    pkgdesc="scaffold core runtime library"
    depends=('gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component core_lib
}

package_libscaffold-core-dev() {
    pkgdesc="scaffold core development files"
    depends=("libscaffold-core=${pkgver}")
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component core_dev
}

package_libscaffold-csv() {
    pkgdesc="scaffold csv runtime library"
    depends=("libscaffold-core=${pkgver}" 'gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component csv_lib
}

package_libscaffold-csv-dev() {
    pkgdesc="scaffold csv development files"
    depends=("libscaffold-csv=${pkgver}" "libscaffold-core-dev=${pkgver}")
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component csv_dev
}

package_libscaffold-json() {
    pkgdesc="scaffold json runtime library"
    depends=("libscaffold-core=${pkgver}" 'gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component json_lib
}

package_libscaffold-json-dev() {
    pkgdesc="scaffold json development files"
    depends=("libscaffold-json=${pkgver}" "libscaffold-core-dev=${pkgver}")
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component json_dev
}

package_libscaffold-convert() {
    pkgdesc="scaffold convert runtime library"
    depends=("libscaffold-csv=${pkgver}" "libscaffold-json=${pkgver}" 'gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component convert_lib
}

package_libscaffold-convert-dev() {
    pkgdesc="scaffold convert development files"
    depends=("libscaffold-convert=${pkgver}" "libscaffold-csv-dev=${pkgver}" "libscaffold-json-dev=${pkgver}")
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component convert_dev
}

package_libscaffold-dev() {
    pkgdesc="scaffold umbrella dev package (CMake integration)"
    depends=(
        "libscaffold-core-dev=${pkgver}"
        "libscaffold-csv-dev=${pkgver}"
        "libscaffold-json-dev=${pkgver}"
        "libscaffold-convert-dev=${pkgver}"
    )
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component cmake_config
}

package_scaffold-csv2json() {
    pkgdesc="scaffold csv2json converter"
    depends=("libscaffold-convert=${pkgver}" 'gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component csv2json_bin
}

package_scaffold-json2csv() {
    pkgdesc="scaffold json2csv converter"
    depends=("libscaffold-csv=${pkgver}" "libscaffold-json=${pkgver}" 'gcc-libs')
    DESTDIR="$pkgdir" cmake --install "$startdir/build/${_profile}" --component json2csv_bin
}
