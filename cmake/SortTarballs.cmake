# Post-build CPack script: re-archive every .tar.gz with --sort=name,
# --owner=0/--group=0, and gzip -n so the TGZ is reproducible regardless
# of the filesystem's readdir() order and the packaging user's identity.
#
# Invoked via CPACK_POST_BUILD_SCRIPTS after each CPack generator runs.
# Receives ${CPACK_PACKAGE_FILES} as a semicolon-separated list of every
# package that was produced (TGZ, DEB, RPM). We filter to .tar.gz only;
# DEB/RPM construct their own sorted inner archives via dpkg-deb/rpmbuild.

if(NOT DEFINED ENV{SOURCE_DATE_EPOCH})
    message(FATAL_ERROR "SOURCE_DATE_EPOCH must be set for reproducible tarball normalization")
endif()

foreach(pkg IN LISTS CPACK_PACKAGE_FILES)
    if(NOT pkg MATCHES "\\.tar\\.gz$")
        continue()
    endif()

    message(STATUS "Normalizing tarball: ${pkg}")
    set(tmp "${pkg}.sort-tmp")
    file(REMOVE_RECURSE "${tmp}")
    file(MAKE_DIRECTORY "${tmp}")

    # Extract. CPack's TGZ wraps everything in a single top-level
    # directory named <package>-<version>-<profile>-<arch>/.
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar -xzf "${pkg}"
        WORKING_DIRECTORY "${tmp}"
        RESULT_VARIABLE rc
    )
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "Failed to extract ${pkg} (rc=${rc})")
    endif()
    file(GLOB children RELATIVE "${tmp}" "${tmp}/*")
    list(LENGTH children n)
    if(NOT n EQUAL 1)
        message(FATAL_ERROR "Expected one top-level dir in ${pkg}, got: ${children}")
    endif()
    list(GET children 0 root)

    # Re-create the tarball with deterministic ordering & ownership. Need
    # GNU tar (not cmake -E tar) for --sort=name, --owner, --group, --mtime.
    file(REMOVE "${pkg}")
    execute_process(
        COMMAND tar
            --sort=name
            --owner=0 --group=0 --numeric-owner
            "--mtime=@$ENV{SOURCE_DATE_EPOCH}"
            --format=gnu
            -cf - "${root}"
        WORKING_DIRECTORY "${tmp}"
        COMMAND gzip -n -9
        OUTPUT_FILE "${pkg}"
        RESULT_VARIABLE rc
    )
    if(NOT rc EQUAL 0)
        file(REMOVE_RECURSE "${tmp}")
        message(FATAL_ERROR "Failed to sort-repack ${pkg} (rc=${rc})")
    endif()
    file(REMOVE_RECURSE "${tmp}")
endforeach()
