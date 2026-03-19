# =============================================================================
# FindOpenBIOS.cmake
# =============================================================================
# Finds the OpenBIOS vendor sources (for firmware)
#
# This module defines:
#   OPENBIOS_FOUND      - True if OpenBIOS sources are available
#   OPENBIOS_SOURCE_DIR - Path to the OpenBIOS source directory
#   OPENBIOS_VERSION    - Detected OpenBIOS version
#
# Optional:
#   OPENBIOS_ROOT_DIR   - Override the default search path
# =============================================================================

set(_openbios_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/openbios
    ${CMAKE_SOURCE_DIR}/../vendor/openbios
)

if(OPENBIOS_ROOT_DIR)
    list(INSERT _openbios_search_paths 0 ${OPENBIOS_ROOT_DIR})
endif()

# Find the OpenBIOS source directory
find_path(OPENBIOS_SOURCE_DIR
    NAMES
        Makefile
        Kconfig
    PATHS
        ${_openbios_search_paths}
    NO_DEFAULT_PATH
)

if(OPENBIOS_SOURCE_DIR)
    # Try to detect version
    if(EXISTS ${OPENBIOS_SOURCE_DIR}/VERSION)
        file(READ ${OPENBIOS_SOURCE_DIR}/VERSION OPENBIOS_VERSION)
        string(STRIP "${OPENBIOS_VERSION}" OPENBIOS_VERSION)
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(OpenBIOS
        REQUIRED_VARS
            OPENBIOS_SOURCE_DIR
        VERSION_VAR
            OPENBIOS_VERSION
    )
    
    mark_as_advanced(OPENBIOS_SOURCE_DIR)
endif()

# Create imported target
if(OPENBIOS_FOUND AND NOT TARGET OpenBIOS::Firmware)
    add_library(OpenBIOS::Firmware INTERFACE IMPORTED)
    set_target_properties(OpenBIOS::Firmware PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${OPENBIOS_SOURCE_DIR}"
    )
endif()
