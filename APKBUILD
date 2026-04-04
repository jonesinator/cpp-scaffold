# Maintainer: Scaffold Dev <scaffold-dev@example.com>
_profile="${SCAFFOLD_PROFILE:-release}"
pkgname=scaffold
_pkgdesc_suffix=""
if [ "$_profile" != "release" ]; then
    pkgname="scaffold-${_profile}"
    _pkgdesc_suffix=" (${_profile} build)"
fi
pkgver=0.1.0
pkgrel=0
pkgdesc="A modern C++26 project scaffold${_pkgdesc_suffix}"
url="https://example.com/scaffold"
arch="x86_64"
license="MIT"
depends="libstdc++"
# Sanitizer runtime libs (libasan.so.8, libubsan.so.1, libtsan.so.2) are auto-detected
# by abuild as "so:" dependencies and resolve to Alpine's `gcc` package.
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
