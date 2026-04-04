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

# ---------- Per-component packaging (DEB + RPM) ----------
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_GROUPING IGNORE)
# TGZ stays monolithic (contains all components).

# Shared system runtime deps, appended to per-package deps below.
set(_deb_sys_deps "libc6 (>= 2.34), libstdc++6 (>= 14)")
set(_rpm_sys_deps "glibc >= 2.34, libstdc++ >= 14")
if(ENABLE_ASAN)
    string(APPEND _deb_sys_deps ", libasan8, libubsan1")
    string(APPEND _rpm_sys_deps ", libasan, libubsan")
elseif(ENABLE_TSAN)
    string(APPEND _deb_sys_deps ", libtsan2")
    string(APPEND _rpm_sys_deps ", libtsan")
endif()

# Debian naming: SOVERSION (=0) is baked into runtime package name
set(_soversion 0)

# ---------- DEB package metadata ----------
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

# Per-component DEB package names and dependencies.
# Runtime library packages (libscaffold-<name>0)
foreach(_lib core csv json convert)
    string(TOUPPER "${_lib}_LIB" _comp)
    set(CPACK_DEBIAN_${_comp}_PACKAGE_NAME "libscaffold-${_lib}${_soversion}")
endforeach()
set(CPACK_DEBIAN_CORE_LIB_PACKAGE_DEPENDS "${_deb_sys_deps}")
set(CPACK_DEBIAN_CSV_LIB_PACKAGE_DEPENDS "libscaffold-core${_soversion} (= ${PROJECT_VERSION}), ${_deb_sys_deps}")
set(CPACK_DEBIAN_JSON_LIB_PACKAGE_DEPENDS "libscaffold-core${_soversion} (= ${PROJECT_VERSION}), ${_deb_sys_deps}")
set(CPACK_DEBIAN_CONVERT_LIB_PACKAGE_DEPENDS "libscaffold-csv${_soversion} (= ${PROJECT_VERSION}), libscaffold-json${_soversion} (= ${PROJECT_VERSION}), ${_deb_sys_deps}")

# Development packages (libscaffold-<name>-dev)
foreach(_lib core csv json convert)
    string(TOUPPER "${_lib}_DEV" _comp)
    set(CPACK_DEBIAN_${_comp}_PACKAGE_NAME "libscaffold-${_lib}-dev")
    set(CPACK_DEBIAN_${_comp}_PACKAGE_SECTION "libdevel")
endforeach()
set(CPACK_DEBIAN_CORE_DEV_PACKAGE_DEPENDS "libscaffold-core${_soversion} (= ${PROJECT_VERSION})")
set(CPACK_DEBIAN_CSV_DEV_PACKAGE_DEPENDS "libscaffold-csv${_soversion} (= ${PROJECT_VERSION}), libscaffold-core-dev (= ${PROJECT_VERSION})")
set(CPACK_DEBIAN_JSON_DEV_PACKAGE_DEPENDS "libscaffold-json${_soversion} (= ${PROJECT_VERSION}), libscaffold-core-dev (= ${PROJECT_VERSION})")
set(CPACK_DEBIAN_CONVERT_DEV_PACKAGE_DEPENDS "libscaffold-convert${_soversion} (= ${PROJECT_VERSION}), libscaffold-csv-dev (= ${PROJECT_VERSION}), libscaffold-json-dev (= ${PROJECT_VERSION})")

# CMake config umbrella package (libscaffold-dev)
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_NAME "libscaffold-dev")
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_SECTION "libdevel")
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_DEPENDS
    "libscaffold-core-dev (= ${PROJECT_VERSION}), libscaffold-csv-dev (= ${PROJECT_VERSION}), libscaffold-json-dev (= ${PROJECT_VERSION}), libscaffold-convert-dev (= ${PROJECT_VERSION})")

# Binary packages
set(CPACK_DEBIAN_CSV2JSON_BIN_PACKAGE_NAME "scaffold-csv2json")
set(CPACK_DEBIAN_CSV2JSON_BIN_PACKAGE_DEPENDS "libscaffold-convert${_soversion} (= ${PROJECT_VERSION}), ${_deb_sys_deps}")
set(CPACK_DEBIAN_JSON2CSV_BIN_PACKAGE_NAME "scaffold-json2csv")
set(CPACK_DEBIAN_JSON2CSV_BIN_PACKAGE_DEPENDS "libscaffold-csv${_soversion} (= ${PROJECT_VERSION}), libscaffold-json${_soversion} (= ${PROJECT_VERSION}), ${_deb_sys_deps}")

# ---------- RPM package metadata ----------
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")

# Runtime library packages
foreach(_lib core csv json convert)
    string(TOUPPER "${_lib}_LIB" _comp)
    set(CPACK_RPM_${_comp}_PACKAGE_NAME "libscaffold-${_lib}")
endforeach()
set(CPACK_RPM_CORE_LIB_PACKAGE_REQUIRES "${_rpm_sys_deps}")
set(CPACK_RPM_CSV_LIB_PACKAGE_REQUIRES "libscaffold-core = ${PROJECT_VERSION}, ${_rpm_sys_deps}")
set(CPACK_RPM_JSON_LIB_PACKAGE_REQUIRES "libscaffold-core = ${PROJECT_VERSION}, ${_rpm_sys_deps}")
set(CPACK_RPM_CONVERT_LIB_PACKAGE_REQUIRES "libscaffold-csv = ${PROJECT_VERSION}, libscaffold-json = ${PROJECT_VERSION}, ${_rpm_sys_deps}")

# Development packages
foreach(_lib core csv json convert)
    string(TOUPPER "${_lib}_DEV" _comp)
    set(CPACK_RPM_${_comp}_PACKAGE_NAME "libscaffold-${_lib}-devel")
endforeach()
set(CPACK_RPM_CORE_DEV_PACKAGE_REQUIRES "libscaffold-core = ${PROJECT_VERSION}")
set(CPACK_RPM_CSV_DEV_PACKAGE_REQUIRES "libscaffold-csv = ${PROJECT_VERSION}, libscaffold-core-devel = ${PROJECT_VERSION}")
set(CPACK_RPM_JSON_DEV_PACKAGE_REQUIRES "libscaffold-json = ${PROJECT_VERSION}, libscaffold-core-devel = ${PROJECT_VERSION}")
set(CPACK_RPM_CONVERT_DEV_PACKAGE_REQUIRES "libscaffold-convert = ${PROJECT_VERSION}, libscaffold-csv-devel = ${PROJECT_VERSION}, libscaffold-json-devel = ${PROJECT_VERSION}")

# CMake config umbrella package
set(CPACK_RPM_CMAKE_CONFIG_PACKAGE_NAME "libscaffold-devel")
set(CPACK_RPM_CMAKE_CONFIG_PACKAGE_REQUIRES
    "libscaffold-core-devel = ${PROJECT_VERSION}, libscaffold-csv-devel = ${PROJECT_VERSION}, libscaffold-json-devel = ${PROJECT_VERSION}, libscaffold-convert-devel = ${PROJECT_VERSION}")

# Binary packages
set(CPACK_RPM_CSV2JSON_BIN_PACKAGE_NAME "scaffold-csv2json")
set(CPACK_RPM_CSV2JSON_BIN_PACKAGE_REQUIRES "libscaffold-convert = ${PROJECT_VERSION}, ${_rpm_sys_deps}")
set(CPACK_RPM_JSON2CSV_BIN_PACKAGE_NAME "scaffold-json2csv")
set(CPACK_RPM_JSON2CSV_BIN_PACKAGE_REQUIRES "libscaffold-csv = ${PROJECT_VERSION}, libscaffold-json = ${PROJECT_VERSION}, ${_rpm_sys_deps}")

# ---------- Source package ----------
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "/[.]git/" "[.]gitignore")

include(CPack)
