if(ENABLE_COVERAGE)
    message(STATUS "Code coverage enabled")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC gcov-based coverage: instrumented at the machine-code level.
        # lcov + genhtml are used for reporting (see targets below).
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        # MC/DC (Modified Condition/Decision Coverage): instruments each boolean
        # sub-expression so lcov can report whether it independently affected the
        # decision. Requires GCC 14+ and lcov 2.2+.
        add_compile_options(-fcondition-coverage)
        add_link_options(--coverage)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # Clang source-based coverage: instrumented at the AST level, so only
        # source-level conditions get branch points (no STL template noise).
        # llvm-profdata + llvm-cov are used for reporting.
        # -fcoverage-mcdc enables MC/DC (Modified Condition/Decision Coverage),
        # matching the GCC -fcondition-coverage metric.
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc)
        add_link_options(-fprofile-instr-generate)
    else()
        message(WARNING "Coverage not supported for compiler ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# ============================================================================
# GCC gcov coverage (lcov + genhtml)
# ============================================================================
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
                --ignore-errors inconsistent,inconsistent,unused,unused,mismatch,mismatch
                # Branch + MC/DC coverage.
                --rc lcov_branch_coverage=1
                --rc mcdc_coverage=1
                # region,branch_region: honour LCOV_EXCL_{START,STOP,LINE}
                # markers (lcov applies these by default, but an explicit
                # --filter replaces the default rather than extending it).
                # brace,blank: filter out gcov closing-brace / blank-line
                # artifacts.
                # exception: filter out GCC's implicit exception-handling
                # branches (every C++ function call site gets a "thrown /
                # not-thrown" branch pair that inflates uncovered counts).
                # branch: filter compiler-generated branches with no
                # corresponding source-level conditional.
                --filter region,branch_region,brace,blank,exception,branch)
        endif()
    endif()
endif()

if(LCOV AND GENHTML)
    add_custom_target(coverage-html
        COMMENT "Generating HTML coverage report (gcov)"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${GENHTML} coverage.info
                           --output-directory ${CMAKE_BINARY_DIR}/coverage
                           --branch-coverage --legend --dark-mode
                           --prefix ${CMAKE_SOURCE_DIR}
                           ${LCOV_EXTRA_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_target(coverage-report
        COMMENT "Generating machine-readable coverage report (gcov)"
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR}
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        COMMAND ${LCOV} --remove coverage.info "/usr/*"
                        --output-file coverage.info
                        ${LCOV_EXTRA_ARGS}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
elseif(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(WARNING "lcov/genhtml not found — gcov coverage targets unavailable")
endif()

# ============================================================================
# Clang source-based coverage (llvm-profdata + llvm-cov)
# ============================================================================
find_program(LLVM_PROFDATA llvm-profdata)
find_program(LLVM_COV llvm-cov)

if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
    if(ENABLE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(WARNING "llvm-profdata/llvm-cov not found — Clang coverage targets unavailable")
    endif()
endif()

# Call scaffold_add_clang_coverage_targets() from the root CMakeLists.txt
# AFTER all add_subdirectory() calls, so that SCAFFOLD_TEST_EXECUTABLES is
# fully populated and the test targets exist for generator expressions.
function(scaffold_add_clang_coverage_targets)
    if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
        return()
    endif()

    get_property(_test_exes GLOBAL PROPERTY SCAFFOLD_TEST_EXECUTABLES)
    get_property(_libs GLOBAL PROPERTY SCAFFOLD_LIBRARIES)
    get_property(_bins GLOBAL PROPERTY SCAFFOLD_BINARIES)
    if(NOT _test_exes)
        return()
    endif()

    # llvm-cov needs every instrumented binary to decode coverage mappings:
    # test executables (run by ctest), shared libraries (contain library
    # source mappings), and the project binaries (exercised via subprocess).
    # The first target is the primary binary; rest become -object flags.
    set(_all_targets ${_test_exes})
    foreach(_lib IN LISTS _libs)
        list(APPEND _all_targets ${_lib})
    endforeach()
    foreach(_bin IN LISTS _bins)
        list(APPEND _all_targets ${_bin})
    endforeach()

    list(GET _all_targets 0 _primary)
    list(REMOVE_AT _all_targets 0)
    set(_object_flags "")
    foreach(_tgt IN LISTS _all_targets)
        list(APPEND _object_flags -object $<TARGET_FILE:${_tgt}>)
    endforeach()

    set(_profdata ${CMAKE_BINARY_DIR}/coverage.profdata)
    set(_profraw_pattern ${CMAKE_BINARY_DIR}/*.profraw)

    add_custom_target(clang-coverage-html
        COMMENT "Generating HTML coverage report (Clang source-based)"
        # Merge all per-process .profraw files into one .profdata.
        COMMAND ${LLVM_PROFDATA} merge -sparse
                ${_profraw_pattern}
                -o ${_profdata}
        # Generate annotated HTML report.
        COMMAND ${LLVM_COV} show
                --format=html
                --show-branches=count
                --show-expansions
                --show-mcdc
                --instr-profile=${_profdata}
                --output-dir=${CMAKE_BINARY_DIR}/coverage
                --ignore-filename-regex=/usr/
                $<TARGET_FILE:${_primary}> ${_object_flags}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    add_custom_target(clang-coverage-report
        COMMENT "Generating coverage summary (Clang source-based)"
        COMMAND ${LLVM_PROFDATA} merge -sparse
                ${_profraw_pattern}
                -o ${_profdata}
        # Terminal summary.
        COMMAND ${LLVM_COV} report
                --show-mcdc-summary
                --instr-profile=${_profdata}
                --ignore-filename-regex=/usr/
                $<TARGET_FILE:${_primary}> ${_object_flags}
        # Machine-readable lcov export.
        COMMAND ${LLVM_COV} export
                --format=lcov
                --instr-profile=${_profdata}
                --ignore-filename-regex=/usr/
                $<TARGET_FILE:${_primary}> ${_object_flags}
                > ${CMAKE_BINARY_DIR}/coverage.lcov
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endfunction()
