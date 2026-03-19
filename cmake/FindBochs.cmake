# =============================================================================
# FindBochs.cmake
# =============================================================================
# Finds the Bochs vendor sources (for BIOS and device import)
#
# This module defines:
#   BOCHS_FOUND      - True if Bochs sources are available
#   BOCHS_SOURCE_DIR - Path to the Bochs source directory
#   BOCHS_VERSION    - Detected Bochs version
#   BOCHS_BIOS_DIR   - Path to the BIOS binaries directory
#
# Optional:
#   BOCHS_ROOT_DIR   - Override the default search path
# =============================================================================

set(_bochs_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/Bochs/bochs
    ${CMAKE_SOURCE_DIR}/../vendor/Bochs/bochs
)

if(BOCHS_ROOT_DIR)
    list(INSERT _bochs_search_paths 0 ${BOCHS_ROOT_DIR})
endif()

# Find the Bochs source directory
find_path(BOCHS_SOURCE_DIR
    NAMES
        configure
        Makefile.in
    PATHS
        ${_bochs_search_paths}
    NO_DEFAULT_PATH
)

if(BOCHS_SOURCE_DIR)
    # Check for BIOS directory
    find_path(BOCHS_BIOS_DIR
        NAMES
            BIOS-bochs-latest
            bios.bin
        PATHS
            ${BOCHS_SOURCE_DIR}
        PATH_SUFFIXES
            bios
            bios/BIOS-bochs-latest
    )
    
    # Try to detect version
    if(EXISTS ${BOCHS_SOURCE_DIR}/VERSION)
        file(READ ${BOCHS_SOURCE_DIR}/VERSION BOCHS_VERSION)
        string(STRIP "${BOCHS_VERSION}" BOCHS_VERSION)
    elseif(EXISTS ${BOCHS_SOURCE_DIR}/changelog)
        file(STRINGS ${BOCHS_SOURCE_DIR}/changelog _version_line
            REGEX "^Bochs[ ]*[0-9]+\\.[0-9]+"
            LIMIT_COUNT 1)
        if(_version_line)
            string(REGEX REPLACE "^Bochs[^0-9]*([0-9\\.]+).*" "\\1" BOCHS_VERSION "${_version_line}")
        endif()
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Bochs
        REQUIRED_VARS
            BOCHS_SOURCE_DIR
        VERSION_VAR
            BOCHS_VERSION
    )
    
    mark_as_advanced(BOCHS_SOURCE_DIR BOCHS_BIOS_DIR)
endif()

# Create imported target
if(BOCHS_FOUND AND NOT TARGET Bochs::Device)
    add_library(Bochs::Device INTERFACE IMPORTED)
    set_target_properties(Bochs::Device PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${BOCHS_SOURCE_DIR}"
    )
endif()
