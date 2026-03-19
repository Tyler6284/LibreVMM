# =============================================================================
# Find86Box.cmake
# =============================================================================
# Finds the 86Box vendor sources (for legacy device import)
#
# This module defines:
#   86BOX_FOUND      - True if 86Box sources are available
#   86BOX_SOURCE_DIR - Path to the 86Box source directory
#   86BOX_VERSION    - Detected 86Box version
#   86BOX_ROMS_DIR   - Path to the ROMs directory
#
# Optional:
#   86BOX_ROOT_DIR   - Override the default search path
# =============================================================================

set(_86box_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/86Box
    ${CMAKE_SOURCE_DIR}/../vendor/86Box
)

if(86BOX_ROOT_DIR)
    list(INSERT _86box_search_paths 0 ${86BOX_ROOT_DIR})
endif()

# Find the 86Box source directory
find_path(86BOX_SOURCE_DIR
    NAMES
        CMakeLists.txt
        src/main.cpp
    PATHS
        ${_86box_search_paths}
    NO_DEFAULT_PATH
)

if(86BOX_SOURCE_DIR)
    # Check for ROMs directory
    find_path(86BOX_ROMS_DIR
        NAMES
            bios
        PATHS
            ${86BOX_SOURCE_DIR}
        PATH_SUFFIXES
            roms
            roms/bios
    )
    
    # Try to detect version
    if(EXISTS ${86BOX_SOURCE_DIR}/CHANGELOG)
        file(STRINGS ${86BOX_SOURCE_DIR}/CHANGELOG _version_line
            REGEX "^v?[0-9]+\.[0-9]+"
            LIMIT_COUNT 1)
        if(_version_line)
            string(REGEX REPLACE "^v?([0-9\\.]+).*" "\\1" 86BOX_VERSION "${_version_line}")
        endif()
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(86Box
        REQUIRED_VARS
            86BOX_SOURCE_DIR
        VERSION_VAR
            86BOX_VERSION
    )
    
    mark_as_advanced(86BOX_SOURCE_DIR 86BOX_ROMS_DIR)
endif()

# Create imported target
if(86BOX_FOUND AND NOT TARGET 86Box::Device)
    add_library(86Box::Device INTERFACE IMPORTED)
    set_target_properties(86Box::Device PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${86BOX_SOURCE_DIR}/src"
    )
endif()
