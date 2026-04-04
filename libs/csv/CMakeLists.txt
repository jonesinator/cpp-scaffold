add_library(csv src/csv.cpp)
add_library(scaffold::csv ALIAS csv)

target_include_directories(csv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(csv PUBLIC core)

generate_export_header(csv
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/csv/csv_export.hpp
)

# ---------- Install ----------
install(
    TARGETS   csv
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/csv
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/csv/csv_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/csv
)

# ---------- Tests ----------
add_executable(csv_test tests/csv_test.cpp)
target_link_libraries(csv_test PRIVATE csv)
add_test(NAME csv_test COMMAND csv_test)
