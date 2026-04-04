# ---------- CPack configuration ----------
set(CPACK_PACKAGE_NAME "scaffold")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "scaffold authors")
set(CPACK_PACKAGE_CONTACT "scaffold-dev@example.com")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://example.com/scaffold")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()

string(TOLOWER "${CMAKE_BUILD_TYPE}" _build_type)
set(CPACK_PACKAGE_FILE_NAME
    "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${_build_type}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# ---------- Generators ----------
set(CPACK_GENERATOR "TGZ;DEB;RPM")

# ---------- DEB ----------
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.34), libstdc++6 (>= 14)")

# ---------- RPM ----------
set(CPACK_RPM_PACKAGE_LICENSE "MIT")
set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
set(CPACK_RPM_PACKAGE_REQUIRES "glibc >= 2.34, libstdc++ >= 14")

# ---------- Source package ----------
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_SOURCE_IGNORE_FILES "/build/" "/[.]git/" "[.]gitignore")

include(CPack)
