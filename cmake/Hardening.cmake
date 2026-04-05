# Compile/link hardening flags for release builds.
#
# Follows the OpenSSF Compiler Options Hardening Guide for C and C++:
#   https://best.openssf.org/Compiler-Hardening-Guides/Compiler-Options-Hardening-Guide-for-C-and-C++
#
# All flags are scoped to the Release configuration via generator expressions
# so sanitizer, coverage, and debug builds are unaffected. Each flag is probed
# for toolchain support before use so older/alternate compilers degrade
# gracefully.

include(CheckCXXCompilerFlag)
include(CheckLinkerFlag)

option(ENABLE_HARDENING "Enable OpenSSF compiler/linker hardening in Release builds" ON)

if(NOT ENABLE_HARDENING)
    message(STATUS "Hardening disabled (ENABLE_HARDENING=OFF)")
    return()
endif()

message(STATUS "Hardening enabled for Release builds")

# Helper: probe a compile flag and, if supported, apply it in Release only.
function(_scaffold_add_release_compile_flag flag)
    string(MAKE_C_IDENTIFIER "HAVE_CXX_${flag}" var)
    check_cxx_compiler_flag("${flag}" ${var})
    if(${var})
        add_compile_options("$<$<CONFIG:Release>:${flag}>")
    endif()
endfunction()

# Helper: probe a linker flag and, if supported, apply it in Release only.
function(_scaffold_add_release_link_flag flag)
    string(MAKE_C_IDENTIFIER "HAVE_LD_${flag}" var)
    check_linker_flag(CXX "${flag}" ${var})
    if(${var})
        add_link_options("$<$<CONFIG:Release>:${flag}>")
    endif()
endfunction()

# ---------- Compile-time hardening ----------

# Stack-smashing protection: emit canaries around functions with stack buffers
# or address-taken locals.
_scaffold_add_release_compile_flag(-fstack-protector-strong)

# Stack-clash protection: probe the stack in page-sized increments to prevent
# guard-page skipping via large allocas.
_scaffold_add_release_compile_flag(-fstack-clash-protection)

# Control-Flow Enforcement Technology (CET): Intel IBT + SHSTK. x86 only.
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|i[3-6]86|amd64|AMD64)$")
    _scaffold_add_release_compile_flag(-fcf-protection=full)
endif()

# Trap on signed integer overflow (defined behavior under -ftrapv) is too
# costly; we rely on UBSan in the asan preset for overflow detection instead.

# _FORTIFY_SOURCE=3 adds compile-time and runtime checks for common libc
# misuses (buffer overflows in str*/mem*/printf-family functions). Requires
# an optimization level >= -O1, so it is Release-only by necessity.
# We undef first to silence glibc's redefinition warning if the toolchain
# already injects a default value (which -Werror would otherwise promote).
check_cxx_compiler_flag("-D_FORTIFY_SOURCE=3" HAVE_FORTIFY_SOURCE_3)
if(HAVE_FORTIFY_SOURCE_3)
    add_compile_options(
        "$<$<CONFIG:Release>:-U_FORTIFY_SOURCE>"
        "$<$<CONFIG:Release>:-D_FORTIFY_SOURCE=3>"
    )
endif()

# ---------- Link-time hardening ----------

# Full RELRO: mark the GOT read-only after relocation.
_scaffold_add_release_link_flag(-Wl,-z,relro)

# BIND_NOW: resolve all symbols at load time (complements RELRO).
_scaffold_add_release_link_flag(-Wl,-z,now)

# Non-executable stack (usually default, but be explicit).
_scaffold_add_release_link_flag(-Wl,-z,noexecstack)

# Place code and data on separate pages to reduce gadget availability.
_scaffold_add_release_link_flag(-Wl,-z,separate-code)

# ---------- Position-Independent Executables ----------

# PIE enables ASLR for executables. Skipped for fully-static musl binaries
# because static+PIE requires static-pie support and linker coordination
# that the static preset does not currently guarantee.
if(NOT SCAFFOLD_STATIC_BINARIES)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()
