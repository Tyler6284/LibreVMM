# =============================================================================
# FindSeaBIOS.cmake
# =============================================================================
# Finds the SeaBIOS vendor sources (for firmware)
#
# This module defines:
#   SEABIOS_FOUND      - True if SeaBIOS sources are available
#   SEABIOS_SOURCE_DIR - Path to the SeaBIOS source directory
#   SEABIOS_VERSION    - Detected SeaBIOS version
#
# Optional:
#   SEABIOS_ROOT_DIR   - Override the default search path
# =============================================================================

set(_seabios_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/seabios
    ${CMAKE_SOURCE_DIR}/../vendor/seabios
)

if(SEABIOS_ROOT_DIR)
    list(INSERT _seabios_search_paths 0 ${SEABIOS_ROOT_DIR})
endif()

# Find the SeaBIOS source directory
find_path(SEABIOS_SOURCE_DIR
    NAMES
        Makefile
        Kconfig
    PATHS
        ${_seabios_search_paths}
    NO_DEFAULT_PATH
)

if(SEABIOS_SOURCE_DIR)
    # Try to detect version
    if(EXISTS ${SEABIOS_SOURCE_DIR}/VERSION)
        file(READ ${SEABIOS_SOURCE_DIR}/VERSION SEABIOS_VERSION)
        string(STRIP "${SEABIOS_VERSION}" SEABIOS_VERSION)
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SeaBIOS
        REQUIRED_VARS
            SEABIOS_SOURCE_DIR
        VERSION_VAR
            SEABIOS_VERSION
    )
    
    mark_as_advanced(SEABIOS_SOURCE_DIR)
endif()

# Create imported target
if(SEABIOS_FOUND AND NOT TARGET SeaBIOS::Firmware)
    add_library(SeaBIOS::Firmware INTERFACE IMPORTED)
    set_target_properties(SeaBIOS::Firmware PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SEABIOS_SOURCE_DIR}"
    )
endif()
