find_program(CCACHE ccache)

if(CCACHE)
    message(STATUS "ccache found: ${CCACHE}")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
    message(STATUS "ccache not found, building without cache")
endif()
