add_executable(csv2json main.cpp)
target_link_libraries(csv2json PRIVATE convert)

# ---------- Install ----------
install(
    TARGETS   csv2json
    RUNTIME   DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT csv2json_bin
)

# ---------- Tests ----------
add_executable(csv2json_test tests/csv2json_test.cpp)
target_link_libraries(csv2json_test PRIVATE test_support)
target_compile_definitions(csv2json_test PRIVATE CSV2JSON_EXECUTABLE="$<TARGET_FILE:csv2json>")
add_test(NAME csv2json_test COMMAND csv2json_test)
