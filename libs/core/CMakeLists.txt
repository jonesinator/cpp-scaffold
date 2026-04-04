add_library(core src/core.cpp)
add_library(scaffold::core ALIAS core)

target_include_directories(core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

generate_export_header(core
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/core/core_export.hpp
)

# ---------- Install ----------
install(
    TARGETS   core
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/core/core_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/core
)

# ---------- Tests ----------
add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE core)
add_test(NAME core_test COMMAND core_test)
