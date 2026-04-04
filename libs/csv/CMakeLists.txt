# ---------- Library targets (both shared and static) ----------
add_library(csv SHARED src/csv.cpp)
add_library(csv_static STATIC src/csv.cpp)
add_library(scaffold::csv ALIAS csv)
add_library(scaffold::csv_static ALIAS csv_static)

set_target_properties(csv PROPERTIES
    OUTPUT_NAME scaffold-csv
    SOVERSION 0
    VERSION ${PROJECT_VERSION}
)
set_target_properties(csv_static PROPERTIES
    OUTPUT_NAME scaffold-csv
)

foreach(tgt csv csv_static)
    target_include_directories(${tgt}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endforeach()

target_link_libraries(csv PUBLIC core)
target_link_libraries(csv_static PUBLIC core_static)

generate_export_header(csv
    EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/generated/csv/csv_export.hpp
)
target_compile_definitions(csv_static PUBLIC csv_STATIC_DEFINE)

# ---------- Install ----------
install(
    TARGETS   csv
    EXPORT    scaffoldTargets
    LIBRARY   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT csv_lib
              NAMELINK_COMPONENT csv_dev
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT csv_lib
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    TARGETS   csv_static
    EXPORT    scaffoldTargets
    ARCHIVE   DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT csv_dev
    INCLUDES  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    DIRECTORY   include/csv
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT   csv_dev
)

install(
    FILES       ${CMAKE_BINARY_DIR}/generated/csv/csv_export.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/csv
    COMPONENT   csv_dev
)

# ---------- Tests ----------
add_executable(csv_test tests/csv_test.cpp)
target_link_libraries(csv_test PRIVATE csv)
add_test(NAME csv_test COMMAND csv_test)
