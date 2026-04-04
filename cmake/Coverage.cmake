if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# coverage-html  — generate an HTML report under build/coverage/
# coverage-report — generate a machine-readable lcov .info file
find_program(LCOV lcov)
find_program(GENHTML genhtml)

set(LCOV_IGNORE --ignore-errors inconsistent,inconsistent,unused,unused)

if(LCOV AND GENHTML)
    add_custom_target(coverage-html
        COMMENT "Generating HTML coverage report"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        --rc derive_function_end_line=0
                        ${LCOV_IGNORE}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        --rc derive_function_end_line=0
                        ${LCOV_IGNORE}
        COMMAND ${GENHTML} coverage.info
                           --output-directory ${CMAKE_BINARY_DIR}/coverage
                           ${LCOV_IGNORE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_target(coverage-report
        COMMENT "Generating machine-readable coverage report"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        --rc derive_function_end_line=0
                        ${LCOV_IGNORE}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        --rc derive_function_end_line=0
                        ${LCOV_IGNORE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
elseif(ENABLE_COVERAGE)
    message(WARNING "lcov/genhtml not found — coverage targets unavailable")
endif()
