# Maintainer: scaffold-dev@example.com
pkgname=scaffold
pkgver=0.1.0
pkgrel=1
pkgdesc="A modern C++26 project scaffold"
arch=('x86_64')
url="https://example.com/scaffold"
license=('MIT')
depends=('gcc-libs')
makedepends=('cmake' 'ninja' 'mold' 'gcc')
options=('staticlibs' '!lto')
source=()

build() {
    cd "$startdir"
    cmake --preset release -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build --preset release
}

check() {
    cd "$startdir"
    ctest --preset release
}

package() {
    cd "$startdir"
    DESTDIR="$pkgdir" cmake --install build/release
}
