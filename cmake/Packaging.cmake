# ---------- CPack configuration ----------
set(CPACK_PACKAGE_NAME "scaffold")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "scaffold authors")
set(CPACK_PACKAGE_CONTACT "scaffold-dev@example.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://example.com/scaffold")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()

# Profile name for package filename — the asan/tsan/coverage presets all use
# CMAKE_BUILD_TYPE=Debug, so we derive the suffix from the enabled flags instead.
if(ENABLE_ASAN)
    set(_profile_suffix "asan")
elseif(ENABLE_TSAN)
    set(_profile_suffix "tsan")
elseif(ENABLE_COVERAGE)
    set(_profile_suffix "coverage")
else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" _profile_suffix)
endif()
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${_profile_suffix}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# ---------- Generators ----------
set(CPACK_GENERATOR "TGZ;DEB;RPM")

# ---------- Runtime deps (vary by sanitizer profile) ----------
set(_deb_deps "libc6 (>= 2.34), libstdc++6 (>= 14)")
set(_rpm_deps "glibc >= 2.34, libstdc++ >= 14")
if(ENABLE_ASAN)
    # ASan+UBSan binaries need libasan + libubsan at runtime
    string(APPEND _deb_deps ", libasan8, libubsan1")
    string(APPEND _rpm_deps ", libasan, libubsan")
elseif(ENABLE_TSAN)
    # TSan binaries need libtsan at runtime
    string(APPEND _deb_deps ", libtsan2")
    string(APPEND _rpm_deps ", libtsan")
endif()

# ---------- DEB ----------
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "${_deb_deps}")

# ---------- RPM ----------
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_REQUIRES "${_rpm_deps}")

# ---------- Source package ----------
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "/[.]git/" "[.]gitignore")

include(CPack)
