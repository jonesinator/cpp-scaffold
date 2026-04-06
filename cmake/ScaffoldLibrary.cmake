# scaffold_add_library(
#     NAME    <lib>          # required; drives target/alias/output/component names
#     SOURCES <file>...      # optional; defaults to src/<NAME>.cpp
#     DEPENDS <lib>...       # optional; logical names (core, csv, json, convert)
#     TIMEOUT <seconds>      # optional; ctest timeout for <NAME>_test, defaults to 60
# )
#
# Produces:
#   - SHARED target <NAME> (libscaffold-<NAME>.so.0.<PROJECT_VERSION>)
#   - STATIC target <NAME>_static (libscaffold-<NAME>.a)
#   - scaffold::<NAME> / scaffold::<NAME>_static aliases
#   - generated/<NAME>/<NAME>_export.hpp (via generate_export_header)
#   - install rules under COMPONENTs <NAME>_lib (runtime) and <NAME>_dev (headers + static)
#   - <NAME>_test executable registered with ctest (from tests/<NAME>_test.cpp)
#
# DEPENDS wires shared↔shared and static↔static automatically: passing
# `DEPENDS core` links <NAME> to core and <NAME>_static to core_static.
function(scaffold_add_library)
    cmake_parse_arguments(ARG "" "NAME;TIMEOUT" "SOURCES;DEPENDS" ${ARGN})
    if(NOT ARG_NAME)
        message(FATAL_ERROR "scaffold_add_library: NAME is required")
    endif()
    if(NOT ARG_SOURCES)
        set(ARG_SOURCES "src/${ARG_NAME}.cpp")
    endif()
    if(NOT ARG_TIMEOUT)
        set(ARG_TIMEOUT 60)
    endif()

    # ---------- Library targets (both shared and static) ----------
    add_library(${ARG_NAME}        SHARED ${ARG_SOURCES})
    add_library(${ARG_NAME}_static STATIC ${ARG_SOURCES})
    add_library(scaffold::${ARG_NAME}        ALIAS ${ARG_NAME})
    add_library(scaffold::${ARG_NAME}_static ALIAS ${ARG_NAME}_static)

    set_target_properties(${ARG_NAME} PROPERTIES
        OUTPUT_NAME scaffold-${ARG_NAME}
        SOVERSION   0
        VERSION     ${PROJECT_VERSION}
    )
    set_target_properties(${ARG_NAME}_static PROPERTIES
        OUTPUT_NAME scaffold-${ARG_NAME}
    )

    foreach(_tgt ${ARG_NAME} ${ARG_NAME}_static)
        target_include_directories(${_tgt}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
    endforeach()

    # Pair shared↔shared and static↔static for every dep.
    foreach(_dep IN LISTS ARG_DEPENDS)
        target_link_libraries(${ARG_NAME}        PUBLIC ${_dep})
        target_link_libraries(${ARG_NAME}_static PUBLIC ${_dep}_static)
    endforeach()

    generate_export_header(${ARG_NAME}
        EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/${ARG_NAME}/${ARG_NAME}_export.hpp
    )
    target_compile_definitions(${ARG_NAME}_static PUBLIC ${ARG_NAME}_STATIC_DEFINE)

    # ---------- Install ----------
    install(
        TARGETS   ${ARG_NAME}
        EXPORT    scaffoldTargets
        LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${ARG_NAME}_lib
                  NAMELINK_COMPONENT ${ARG_NAME}_dev
        RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${ARG_NAME}_lib
        INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        TARGETS   ${ARG_NAME}_static
        EXPORT    scaffoldTargets
        ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT ${ARG_NAME}_dev
        INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        DIRECTORY   include/${ARG_NAME}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT   ${ARG_NAME}_dev
    )

    install(
        FILES       ${CMAKE_BINARY_DIR}/generated/${ARG_NAME}/${ARG_NAME}_export.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${ARG_NAME}
        COMPONENT   ${ARG_NAME}_dev
    )

    # ---------- Tests ----------
    add_executable(${ARG_NAME}_test tests/${ARG_NAME}_test.cpp)
    target_link_libraries(${ARG_NAME}_test PRIVATE ${ARG_NAME} test_support)
    add_test(NAME ${ARG_NAME}_test COMMAND ${ARG_NAME}_test)
    set_tests_properties(${ARG_NAME}_test PROPERTIES TIMEOUT ${ARG_TIMEOUT})

    # ---------- Registry ----------
    # Record in global registry so Packaging.cmake and scaffold-config.cmake.in
    # can derive the component list + DEB/RPM per-package dep strings without
    # hand-maintaining a parallel list of libs + their dependency graph.
    set_property(GLOBAL APPEND PROPERTY SCAFFOLD_LIBRARIES ${ARG_NAME})
    set_property(GLOBAL PROPERTY SCAFFOLD_LIBRARY_DEPS_${ARG_NAME} "${ARG_DEPENDS}")
    set_property(GLOBAL APPEND PROPERTY SCAFFOLD_TEST_EXECUTABLES ${ARG_NAME}_test)
endfunction()
