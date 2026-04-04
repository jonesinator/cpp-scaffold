# Maintainer: Scaffold Dev <scaffold-dev@example.com>
_profile="${SCAFFOLD_PROFILE:-release}"
pkgname=scaffold
_pkgdesc_suffix=""
if [ "$_profile" = "debug" ]; then
    pkgname=scaffold-debug
    _pkgdesc_suffix=" (debug build)"
fi
pkgver=0.1.0
pkgrel=0
pkgdesc="A modern C++26 project scaffold${_pkgdesc_suffix}"
url="https://example.com/scaffold"
arch="x86_64"
license="MIT"
depends="libstdc++"
makedepends="cmake samurai mold"
source=""
builddir="$startdir"
options="!strip"

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
