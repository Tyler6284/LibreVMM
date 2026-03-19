# =============================================================================
# FindVirtualBox.cmake
# =============================================================================
# Finds the VirtualBox vendor sources
#
# This module defines:
#   VIRTUALBOX_FOUND        - True if VirtualBox sources are available
#   VIRTUALBOX_SOURCE_DIR   - Path to the VirtualBox source directory
#   VIRTUALBOX_KBUILD_DIR  - Path to the kBuild configuration directory
#   VIRTUALBOX_VERSION     - Detected VirtualBox version
#
# Optional:
#   VIRTUALBOX_ROOT_DIR    - Override the default search path
# =============================================================================

# Default search paths
set(_virtualbox_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/virtualbox
    ${CMAKE_SOURCE_DIR}/../vendor/virtualbox
    $ENV{VIRTUALBOX_ROOT}
)

if(VIRTUALBOX_ROOT_DIR)
    list(INSERT _virtualbox_search_paths 0 ${VIRTUALBOX_ROOT_DIR})
endif()

# Find the VirtualBox source directory
find_path(VIRTUALBOX_SOURCE_DIR
    NAMES
        Config.kmk
        Makefile.kmk
    PATHS
        ${_virtualbox_search_paths}
    NO_DEFAULT_PATH
)

# Find the kBuild configuration directory
if(VIRTUALBOX_SOURCE_DIR)
    set(VIRTUALBOX_KBUILD_DIR ${VIRTUALBOX_SOURCE_DIR})
    
    # Try to detect version from Version.kmk or similar
    if(EXISTS ${VIRTUALBOX_SOURCE_DIR}/Version.kmk)
        file(STRINGS ${VIRTUALBOX_SOURCE_DIR}/Version.kmk _version_line 
            REGEX "^(VBOX_VERSION_MAJOR|VBOX_VERSION_STRING)"
            LIMIT_COUNT 2)
        foreach(_line ${_version_line})
            if(_line MATCHES "VBOX_VERSION_MAJOR[ ]*=[ ]*([0-9]+)")
                set(VIRTUALBOX_VERSION_MAJOR ${CMAKE_MATCH_1})
            endif()
            if(_line MATCHES "VBOX_VERSION_MINOR[ ]*=[ ]*([0-9]+)")
                set(VIRTUALBOX_VERSION_MINOR ${CMAKE_MATCH_1})
            endif()
        endforeach()
        if(DEFINED VIRTUALBOX_VERSION_MAJOR AND DEFINED VIRTUALBOX_VERSION_MINOR)
            set(VIRTUALBOX_VERSION "${VIRTUALBOX_VERSION_MAJOR}.${VIRTUALBOX_VERSION_MINOR}")
        endif()
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(VirtualBox
        REQUIRED_VARS
            VIRTUALBOX_SOURCE_DIR
            VIRTUALBOX_KBUILD_DIR
        VERSION_VAR
            VIRTUALBOX_VERSION
    )
    
    # Mark as advanced
    mark_as_advanced(VIRTUALBOX_SOURCE_DIR VIRTUALBOX_KBUILD_DIR)
endif()

# Create imported targets if found
if(VIRTUALBOX_FOUND AND NOT TARGET VirtualBox::VirtualBox)
    add_library(VirtualBox::VirtualBox INTERFACE IMPORTED)
    set_target_properties(VirtualBox::VirtualBox PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${VIRTUALBOX_SOURCE_DIR}"
    )
endif()
