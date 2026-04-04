# ---------- Library targets (both shared and static) ----------
add_library(core SHARED src/core.cpp)
add_library(core_static STATIC src/core.cpp)
add_library(scaffold::core ALIAS core)
add_library(scaffold::core_static ALIAS core_static)

set_target_properties(core PROPERTIES
    OUTPUT_NAME scaffold-core
    SOVERSION 0
    VERSION ${PROJECT_VERSION}
)
set_target_properties(core_static PROPERTIES
    OUTPUT_NAME scaffold-core
)

foreach(tgt core core_static)
    target_include_directories(${tgt}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endforeach()

generate_export_header(core
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/core/core_export.hpp
)
target_compile_definitions(core_static PUBLIC core_STATIC_DEFINE)

# ---------- Install ----------
install(
    TARGETS   core
    EXPORT    scaffoldTargets
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT core_lib
              NAMELINK_COMPONENT core_dev
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT core_lib
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS   core_static
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT core_dev
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/core
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT   core_dev
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/core/core_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/core
    COMPONENT   core_dev
)

# ---------- Tests ----------
add_executable(core_test tests/core_test.cpp)
target_link_libraries(core_test PRIVATE core)
add_test(NAME core_test COMMAND core_test)
