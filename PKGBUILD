# Maintainer: scaffold-dev@example.com
_profile="${SCAFFOLD_PROFILE:-release}"
pkgname=scaffold
_pkgdesc_suffix=""
if [ "$_profile" != "release" ]; then
    pkgname="scaffold-${_profile}"
    _pkgdesc_suffix=" (${_profile} build)"
fi
pkgver=0.1.0
pkgrel=1
pkgdesc="A modern C++26 project scaffold${_pkgdesc_suffix}"
arch=('x86_64')
url="https://example.com/scaffold"
license=('MIT')
depends=('gcc-libs')
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

package() {
    cd "$startdir"
    DESTDIR="$pkgdir" cmake --install build/${_profile}
}
