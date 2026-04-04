add_library(json src/json.cpp)
add_library(scaffold::json ALIAS json)

target_include_directories(json
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(json PUBLIC core)

generate_export_header(json
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/json/json_export.hpp
)

# ---------- Install ----------
install(
    TARGETS   json
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/json
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/json/json_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/json
)

# ---------- Tests ----------
add_executable(json_test tests/json_test.cpp)
target_link_libraries(json_test PRIVATE json)
add_test(NAME json_test COMMAND json_test)
