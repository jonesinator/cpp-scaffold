add_library(convert src/convert.cpp)
add_library(scaffold::convert ALIAS convert)

target_include_directories(convert
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(convert PUBLIC csv json)

generate_export_header(convert
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/convert/convert_export.hpp
)

# ---------- Install ----------
install(
    TARGETS   convert
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/convert
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/convert/convert_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/convert
)

# ---------- Tests ----------
add_executable(convert_test tests/convert_test.cpp)
target_link_libraries(convert_test PRIVATE convert)
add_test(NAME convert_test COMMAND convert_test)
