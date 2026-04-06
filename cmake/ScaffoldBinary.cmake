# scaffold_add_binary(
#     NAME    <bin>          # required; drives target/component names and the
#                            #           <NAME>_EXECUTABLE test macro
#     SOURCES <file>...      # optional; defaults to main.cpp
#     DEPENDS <lib>...       # scaffold libraries to link against (core, csv, ...)
#     TIMEOUT <seconds>      # optional; ctest timeout for <NAME>_test, defaults to 60
# )
#
# Produces:
#   - executable target <NAME> (from main.cpp unless SOURCES overrides)
#   - link against shared DEPENDS (normal build) or <dep>_static + `-static`
#     (when SCAFFOLD_STATIC_BINARIES=ON)
#   - install rule under COMPONENT <NAME>_bin
#   - <NAME>_test executable from tests/<NAME>_test.cpp, linking test_support,
#     with <NAME_UPPER>_EXECUTABLE="$<TARGET_FILE:<NAME>>" defined
#
# Assumes <NAME> is a valid C identifier (no hyphens) so that the derived
# <NAME_UPPER>_EXECUTABLE macro is usable.
function(scaffold_add_binary)
    cmake_parse_arguments(ARG "" "NAME;TIMEOUT" "SOURCES;DEPENDS" ${ARGN})
    if(NOT ARG_NAME)
        message(FATAL_ERROR "scaffold_add_binary: NAME is required")
    endif()
    if(NOT ARG_SOURCES)
        set(ARG_SOURCES main.cpp)
    endif()
    if(NOT ARG_TIMEOUT)
        set(ARG_TIMEOUT 60)
    endif()

    add_executable(${ARG_NAME} ${ARG_SOURCES})

    if(SCAFFOLD_STATIC_BINARIES)
        foreach(_dep IN LISTS ARG_DEPENDS)
            target_link_libraries(${ARG_NAME} PRIVATE ${_dep}_static)
        endforeach()
        target_link_options(${ARG_NAME} PRIVATE -static)
    else()
        foreach(_dep IN LISTS ARG_DEPENDS)
            target_link_libraries(${ARG_NAME} PRIVATE ${_dep})
        endforeach()
    endif()

    # ---------- Install ----------
    install(
        TARGETS   ${ARG_NAME}
        RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT ${ARG_NAME}_bin
    )

    # ---------- Tests ----------
    string(TOUPPER "${ARG_NAME}" _upper)
    add_executable(${ARG_NAME}_test tests/${ARG_NAME}_test.cpp)
    target_link_libraries(${ARG_NAME}_test PRIVATE test_support)
    target_compile_definitions(${ARG_NAME}_test PRIVATE ${_upper}_EXECUTABLE="$<TARGET_FILE:${ARG_NAME}>")
    add_test(NAME ${ARG_NAME}_test COMMAND ${ARG_NAME}_test)
    set_tests_properties(${ARG_NAME}_test PROPERTIES TIMEOUT ${ARG_TIMEOUT})

    # ---------- Registry ----------
    # See scaffold_add_library for rationale.
    set_property(GLOBAL APPEND PROPERTY SCAFFOLD_BINARIES ${ARG_NAME})
    set_property(GLOBAL PROPERTY SCAFFOLD_BINARY_DEPS_${ARG_NAME} "${ARG_DEPENDS}")
    set_property(GLOBAL APPEND PROPERTY SCAFFOLD_TEST_EXECUTABLES ${ARG_NAME}_test)
endfunction()
