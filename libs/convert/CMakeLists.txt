# ---------- Library targets (both shared and static) ----------
add_library(convert SHARED src/convert.cpp)
add_library(convert_static STATIC src/convert.cpp)
add_library(scaffold::convert ALIAS convert)
add_library(scaffold::convert_static ALIAS convert_static)

set_target_properties(convert PROPERTIES
    OUTPUT_NAME scaffold-convert
    SOVERSION 0
    VERSION ${PROJECT_VERSION}
)
set_target_properties(convert_static PROPERTIES
    OUTPUT_NAME scaffold-convert
)

foreach(tgt convert convert_static)
    target_include_directories(${tgt}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endforeach()

target_link_libraries(convert PUBLIC csv json)
target_link_libraries(convert_static PUBLIC csv_static json_static)

generate_export_header(convert
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/convert/convert_export.hpp
)
target_compile_definitions(convert_static PUBLIC convert_STATIC_DEFINE)

# ---------- Install ----------
install(
    TARGETS   convert
    EXPORT    scaffoldTargets
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT convert_lib
              NAMELINK_COMPONENT convert_dev
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT convert_lib
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS   convert_static
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT convert_dev
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/convert
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT   convert_dev
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/convert/convert_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/convert
    COMPONENT   convert_dev
)

# ---------- Tests ----------
add_executable(convert_test tests/convert_test.cpp)
target_link_libraries(convert_test PRIVATE convert)
add_test(NAME convert_test COMMAND convert_test)
