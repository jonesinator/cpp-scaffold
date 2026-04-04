include(CMakePackageConfigHelpers)

# ---------- Version header ----------
install(
    FILES       ${CMAKE_BINARY_DIR}/generated/scaffold/version.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scaffold
)

# ---------- Package version file ----------
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# ---------- Package config from template ----------
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
)

# ---------- Config files ----------
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/scaffold-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/scaffold
)
