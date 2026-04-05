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

# Per-component DEB package names and dependencies are derived from the
# library/binary registry populated by scaffold_add_library/_binary(). See
# cmake/ScaffoldLibrary.cmake for the global-property contract.
get_property(_scaffold_libs GLOBAL PROPERTY SCAFFOLD_LIBRARIES)
get_property(_scaffold_bins GLOBAL PROPERTY SCAFFOLD_BINARIES)

# Runtime library packages (libscaffold-<name>0)
foreach(_lib IN LISTS _scaffold_libs)
    string(TOUPPER "${_lib}_LIB" _comp)
    set(CPACK_DEBIAN_${_comp}_PACKAGE_NAME "libscaffold-${_lib}${_soversion}")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_LIBRARY_DEPS_${_lib})
    set(_dep_str "")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str "libscaffold-${_dep}${_soversion} (= ${PROJECT_VERSION}), ")
    endforeach()
    set(CPACK_DEBIAN_${_comp}_PACKAGE_DEPENDS "${_dep_str}${_deb_sys_deps}")
endforeach()

# Development packages (libscaffold-<name>-dev): self-runtime + each direct
# dep's -dev package. Transitive dev deps are pulled in via each direct dep's
# own -dev package, so we don't walk the full closure here.
foreach(_lib IN LISTS _scaffold_libs)
    string(TOUPPER "${_lib}_DEV" _comp)
    set(CPACK_DEBIAN_${_comp}_PACKAGE_NAME "libscaffold-${_lib}-dev")
    set(CPACK_DEBIAN_${_comp}_PACKAGE_SECTION "libdevel")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_LIBRARY_DEPS_${_lib})
    set(_dep_str "libscaffold-${_lib}${_soversion} (= ${PROJECT_VERSION})")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str ", libscaffold-${_dep}-dev (= ${PROJECT_VERSION})")
    endforeach()
    set(CPACK_DEBIAN_${_comp}_PACKAGE_DEPENDS "${_dep_str}")
endforeach()

# CMake config umbrella package (libscaffold-dev) — depends on every -dev.
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_NAME "libscaffold-dev")
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_SECTION "libdevel")
set(_umbrella_deps "")
foreach(_lib IN LISTS _scaffold_libs)
    if(_umbrella_deps)
        string(APPEND _umbrella_deps ", ")
    endif()
    string(APPEND _umbrella_deps "libscaffold-${_lib}-dev (= ${PROJECT_VERSION})")
endforeach()
set(CPACK_DEBIAN_CMAKE_CONFIG_PACKAGE_DEPENDS "${_umbrella_deps}")

# Binary packages (scaffold-<name>) — depend on each linked library's runtime.
foreach(_bin IN LISTS _scaffold_bins)
    string(TOUPPER "${_bin}_BIN" _comp)
    set(CPACK_DEBIAN_${_comp}_PACKAGE_NAME "scaffold-${_bin}")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_BINARY_DEPS_${_bin})
    set(_dep_str "")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str "libscaffold-${_dep}${_soversion} (= ${PROJECT_VERSION}), ")
    endforeach()
    set(CPACK_DEBIAN_${_comp}_PACKAGE_DEPENDS "${_dep_str}${_deb_sys_deps}")
endforeach()

# ---------- RPM package metadata ----------
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
# Force rpmbuild into reproducible mode via rpm macros injected into every
# generated spec. rpm's own reproducibility knobs:
#   _buildhost                         — pins RPMTAG_BUILDHOST (otherwise
#                                        rpmbuild records gethostname(),
#                                        which reprotest's domain_host
#                                        variation shifts between runs)
#   use_source_date_epoch_as_buildtime — forces RPMTAG_BUILDTIME to
#                                        $SOURCE_DATE_EPOCH instead of
#                                        wall-clock time(NULL)
#   clamp_mtime_to_source_date_epoch   — clamps every file's mtime in the
#                                        cpio payload to SOURCE_DATE_EPOCH
#   source_date_epoch_from_changelog 0 — don't auto-derive SDE from the
#                                        (empty) changelog; trust the env
# See https://rpm-software-management.github.io/rpm/manual/reproducible_builds.html
set(CPACK_RPM_SPEC_MORE_DEFINE "\
%define _buildhost reproducible.build
%define source_date_epoch_from_changelog 0
%define use_source_date_epoch_as_buildtime 1
%define clamp_mtime_to_source_date_epoch 1")

# Runtime library packages (libscaffold-<name>)
foreach(_lib IN LISTS _scaffold_libs)
    string(TOUPPER "${_lib}_LIB" _comp)
    set(CPACK_RPM_${_comp}_PACKAGE_NAME "libscaffold-${_lib}")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_LIBRARY_DEPS_${_lib})
    set(_dep_str "")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str "libscaffold-${_dep} = ${PROJECT_VERSION}, ")
    endforeach()
    set(CPACK_RPM_${_comp}_PACKAGE_REQUIRES "${_dep_str}${_rpm_sys_deps}")
endforeach()

# Development packages (libscaffold-<name>-devel)
foreach(_lib IN LISTS _scaffold_libs)
    string(TOUPPER "${_lib}_DEV" _comp)
    set(CPACK_RPM_${_comp}_PACKAGE_NAME "libscaffold-${_lib}-devel")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_LIBRARY_DEPS_${_lib})
    set(_dep_str "libscaffold-${_lib} = ${PROJECT_VERSION}")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str ", libscaffold-${_dep}-devel = ${PROJECT_VERSION}")
    endforeach()
    set(CPACK_RPM_${_comp}_PACKAGE_REQUIRES "${_dep_str}")
endforeach()

# CMake config umbrella package
set(CPACK_RPM_CMAKE_CONFIG_PACKAGE_NAME "libscaffold-devel")
set(_umbrella_deps "")
foreach(_lib IN LISTS _scaffold_libs)
    if(_umbrella_deps)
        string(APPEND _umbrella_deps ", ")
    endif()
    string(APPEND _umbrella_deps "libscaffold-${_lib}-devel = ${PROJECT_VERSION}")
endforeach()
set(CPACK_RPM_CMAKE_CONFIG_PACKAGE_REQUIRES "${_umbrella_deps}")

# Binary packages
foreach(_bin IN LISTS _scaffold_bins)
    string(TOUPPER "${_bin}_BIN" _comp)
    set(CPACK_RPM_${_comp}_PACKAGE_NAME "scaffold-${_bin}")
    get_property(_deps GLOBAL PROPERTY SCAFFOLD_BINARY_DEPS_${_bin})
    set(_dep_str "")
    foreach(_dep IN LISTS _deps)
        string(APPEND _dep_str "libscaffold-${_dep} = ${PROJECT_VERSION}, ")
    endforeach()
    set(CPACK_RPM_${_comp}_PACKAGE_REQUIRES "${_dep_str}${_rpm_sys_deps}")
endforeach()

# ---------- Source package ----------
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "/[.]git/" "[.]gitignore")

# ---------- Reproducibility: normalize .tar.gz entry order + owner ----------
# CPack's TGZ generator walks the staging tree via readdir(), which gives
# filesystem-dependent entry ordering (ext4 vs btrfs vs disorderfs mount
# all produce different orderings). Re-archive each .tar.gz with
# --sort=name, --owner=0, --group=0 after CPack runs.
set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/SortTarballs.cmake")

include(CPack)
