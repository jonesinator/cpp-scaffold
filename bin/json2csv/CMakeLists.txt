add_executable(json2csv main.cpp)
if(SCAFFOLD_STATIC_BINARIES)
    target_link_libraries(json2csv PRIVATE csv_static json_static)
    target_link_options(json2csv PRIVATE -static)
else()
    target_link_libraries(json2csv PRIVATE csv json)
endif()

# ---------- Install ----------
install(
    TARGETS   json2csv
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT json2csv_bin
)

# ---------- Tests ----------
add_executable(json2csv_test tests/json2csv_test.cpp)
target_link_libraries(json2csv_test PRIVATE test_support)
target_compile_definitions(json2csv_test PRIVATE JSON2CSV_EXECUTABLE="$<TARGET_FILE:json2csv>")
add_test(NAME json2csv_test COMMAND json2csv_test)
