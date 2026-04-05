cmake_minimum_required(VERSION 3.30)
project(scaffold
    VERSION 0.1.0
    DESCRIPTION "scaffold project"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

# ---------- Install paths ----------
include(GNUInstallDirs)

# ---------- ccache ----------
include(cmake/Ccache.cmake)

# ---------- Linker ----------
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")

# ---------- Reproducibility ----------
if(NOT ENABLE_COVERAGE)
    add_compile_options(-ffile-prefix-map=${CMAKE_SOURCE_DIR}=.)
endif()
add_link_options(-Wl,--build-id=sha256)

# ---------- Compiler warnings ----------
add_compile_options(
    -Wall -Wextra -Wpedantic -Werror
    -Wconversion -Wsign-conversion -Wshadow
    -Wnon-virtual-dtor -Wold-style-cast -Wcast-align
    -Woverloaded-virtual -Wunused
    -Wnull-dereference -Wdouble-promotion
    -Wformat=2 -Wimplicit-fallthrough
    -Wmisleading-indentation
    -Wduplicated-cond -Wduplicated-branches
    -Wlogical-op -Wuseless-cast
    -Wsuggest-override
)

# ---------- Coverage (opt-in) ----------
option(ENABLE_COVERAGE "Build with code-coverage instrumentation" OFF)
include(cmake/Coverage.cmake)

# ---------- Sanitizers (opt-in) ----------
option(ENABLE_ASAN "Build with AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)
include(cmake/Sanitizers.cmake)

# ---------- Static binaries (opt-in, musl + libstdc++ fully static) ----------
option(SCAFFOLD_STATIC_BINARIES "Statically link scaffold binaries (musl + libstdc++)" OFF)

# ---------- Shared library support ----------
# We always build both shared and static variants of each library (see
# libs/*/CMakeLists.txt). Visibility hiding is needed for proper shared library
# symbol management via generate_export_header.
include(GenerateExportHeader)
add_compile_options(-fvisibility=hidden -fvisibility-inlines-hidden)

# ---------- Version header ----------
include(cmake/Version.cmake)

# ---------- Testing ----------
enable_testing()

# ---------- Libraries ----------
add_subdirectory(libs/core)
add_subdirectory(libs/csv)
add_subdirectory(libs/json)
add_subdirectory(libs/convert)
add_subdirectory(libs/test_support)

# ---------- Binaries ----------
add_subdirectory(bin/csv2json)
add_subdirectory(bin/json2csv)

# ---------- Install / Packaging ----------
include(cmake/Install.cmake)
include(cmake/Packaging.cmake)
