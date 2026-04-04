if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()

# coverage-html  — generate an HTML report under build/coverage/
# coverage-report — generate a machine-readable lcov .info file
find_program(LCOV lcov)
find_program(GENHTML genhtml)

# lcov 2.x supports error suppression categories (inconsistent, unused) and
# --rc derive_function_end_line=0 that lcov 1.x doesn't understand. Detect the
# major version so we only pass 2.x-specific flags when supported.
set(LCOV_EXTRA_ARGS "")
if(LCOV)
    execute_process(
        COMMAND ${LCOV} --version
        OUTPUT_VARIABLE LCOV_VERSION_OUTPUT
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(LCOV_VERSION_OUTPUT MATCHES "LCOV version ([0-9]+)")
        set(LCOV_MAJOR_VERSION ${CMAKE_MATCH_1})
        if(LCOV_MAJOR_VERSION GREATER_EQUAL 2)
            set(LCOV_EXTRA_ARGS
                --rc derive_function_end_line=0
                --ignore-errors inconsistent,inconsistent,unused,unused)
        endif()
    endif()
endif()

if(LCOV AND GENHTML)
    add_custom_target(coverage-html
        COMMENT "Generating HTML coverage report"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${GENHTML} coverage.info
                           --output-directory ${CMAKE_BINARY_DIR}/coverage
                           ${LCOV_EXTRA_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_target(coverage-report
        COMMENT "Generating machine-readable coverage report"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
elseif(ENABLE_COVERAGE)
    message(WARNING "lcov/genhtml not found — coverage targets unavailable")
endif()
