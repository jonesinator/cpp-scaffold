add_library(test_support INTERFACE)
target_include_directories(test_support INTERFACE include)

# Self-tests. test_support is INTERFACE-only (no install, no shared/static
# variants), so it doesn't use scaffold_add_library().
add_executable(test_support_test tests/test_support_test.cpp)
target_link_libraries(test_support_test PRIVATE test_support)
add_test(NAME test_support_test COMMAND test_support_test)
set_tests_properties(test_support_test PROPERTIES TIMEOUT 60)
set_property(GLOBAL APPEND PROPERTY SCAFFOLD_TEST_EXECUTABLES test_support_test)
