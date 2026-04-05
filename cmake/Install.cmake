include(CMakePackageConfigHelpers)

# ---------- Version header (part of cmake_config component) ----------
install(
    FILES       ${CMAKE_BINARY_DIR}/generated/scaffold/version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scaffold
    COMPONENT   cmake_config
)

# ---------- Package version file ----------
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# ---------- Package config from template ----------
# SCAFFOLD_KNOWN_COMPONENTS is populated by scaffold_add_library() via a global
# property — see cmake/ScaffoldLibrary.cmake. This keeps the component list in
# lockstep with what's actually built.
get_property(SCAFFOLD_KNOWN_COMPONENTS GLOBAL PROPERTY SCAFFOLD_LIBRARIES)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/scaffold-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scaffold
)

# ---------- Export set ----------
install(
    EXPORT      scaffoldTargets
    NAMESPACE   scaffold::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scaffold
    COMPONENT   cmake_config
)

# ---------- Config files ----------
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scaffold
    COMPONENT   cmake_config
)

# ---------- Reproducibility: normalize staged directory permissions ----------
# Implicit parent directories created while installing files (bin/, include/,
# lib/, lib/cmake/, ...) inherit permissions from the current umask.
# CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS only covers directories
# created by explicit install(DIRECTORY ...) commands, not these implicit
# parents, so TGZ/DEB/RPM hashes diverge between a umask-022 build and
# a umask-027 build. Force every directory under the staging prefix to
# 0755 as the last install step so archive content is umask-independent.
install(CODE [[
    # Honor DESTDIR (set by `cmake --install --prefix=... DESTDIR=...` and by
    # CPack staging). Without this, file(CHMOD) would try to chmod paths
    # under a non-existent /usr/local/... during a DESTDIR install.
    set(_repro_prefix "${CMAKE_INSTALL_PREFIX}")
    if(DEFINED ENV{DESTDIR} AND NOT "$ENV{DESTDIR}" STREQUAL "")
        set(_repro_prefix "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
    endif()
    file(GLOB_RECURSE _repro_entries LIST_DIRECTORIES true "${_repro_prefix}/*")
    foreach(_entry IN LISTS _repro_entries)
        if(IS_DIRECTORY "${_entry}")
            file(CHMOD "${_entry}" PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)
        endif()
    endforeach()
]] ALL_COMPONENTS)
