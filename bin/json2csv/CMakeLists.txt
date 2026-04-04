add_executable(json2csv main.cpp)
target_link_libraries(json2csv PRIVATE csv json)

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
