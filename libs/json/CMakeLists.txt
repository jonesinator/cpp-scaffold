# ---------- Library targets (both shared and static) ----------
add_library(json SHARED src/json.cpp)
add_library(json_static STATIC src/json.cpp)
add_library(scaffold::json ALIAS json)
add_library(scaffold::json_static ALIAS json_static)

set_target_properties(json PROPERTIES
    OUTPUT_NAME scaffold-json
    SOVERSION 0
    VERSION ${PROJECT_VERSION}
)
set_target_properties(json_static PROPERTIES
    OUTPUT_NAME scaffold-json
)

foreach(tgt json json_static)
    target_include_directories(${tgt}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endforeach()

target_link_libraries(json PUBLIC core)
target_link_libraries(json_static PUBLIC core_static)

generate_export_header(json
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/json/json_export.hpp
)
target_compile_definitions(json_static PUBLIC json_STATIC_DEFINE)

# ---------- Install ----------
install(
    TARGETS   json
    EXPORT    scaffoldTargets
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT json_lib
              NAMELINK_COMPONENT json_dev
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT json_lib
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS   json_static
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT json_dev
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/json
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT   json_dev
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/json/json_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json
    COMPONENT   json_dev
)

# ---------- Tests ----------
add_executable(json_test tests/json_test.cpp)
target_link_libraries(json_test PRIVATE json)
add_test(NAME json_test COMMAND json_test)
