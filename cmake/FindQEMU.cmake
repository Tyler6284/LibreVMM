# =============================================================================
# FindQEMU.cmake
# =============================================================================
# Finds the QEMU vendor sources (for TCG integration)
#
# This module defines:
#   QEMU_FOUND         - True if QEMU sources are available
#   QEMU_SOURCE_DIR    - Path to the QEMU source directory
#   QEMU_VERSION       - Detected QEMU version
#   QEMU_TCG_FOUND     - True if TCG can be built
#
# Optional:
#   QEMU_ROOT_DIR      - Override the default search path
# =============================================================================

set(_qemu_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/qemu
    ${CMAKE_SOURCE_DIR}/../vendor/qemu
)

if(QEMU_ROOT_DIR)
    list(INSERT _qemu_search_paths 0 ${QEMU_ROOT_DIR})
endif()

# Find the QEMU source directory
find_path(QEMU_SOURCE_DIR
    NAMES
        meson.build
        configure
    PATHS
        ${_qemu_search_paths}
    NO_DEFAULT_PATH
)

# Check for TCG-specific files
if(QEMU_SOURCE_DIR)
    set(QEMU_TCG_SOURCES
        ${QEMU_SOURCE_DIR}/accel/tcg/tcg.c
        ${QEMU_SOURCE_DIR}/tcg/tcg.c
    )
    
    set(QEMU_TCG_FOUND FALSE)
    foreach(_src ${QEMU_TCG_SOURCES})
        if(EXISTS ${_src})
            set(QEMU_TCG_FOUND TRUE)
            break()
        endif()
    endforeach()
    
    # Try to detect version
    if(EXISTS ${QEMU_SOURCE_DIR}/version)
        file(READ ${QEMU_SOURCE_DIR}/version QEMU_VERSION)
        string(STRIP "${QEMU_VERSION}" QEMU_VERSION)
    elseif(EXISTS ${QEMU_SOURCE_DIR}/VERSION)
        file(READ ${QEMU_SOURCE_DIR}/VERSION QEMU_VERSION)
        string(STRIP "${QEMU_VERSION}" QEMU_VERSION)
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(QEMU
        REQUIRED_VARS
            QEMU_SOURCE_DIR
        VERSION_VAR
            QEMU_VERSION
    )
    
    mark_as_advanced(QEMU_SOURCE_DIR)
endif()

# Create imported targets if found
if(QEMU_FOUND AND NOT TARGET QEMU::TCG)
    add_library(QEMU::TCG INTERFACE IMPORTED)
    set_target_properties(QEMU::TCG PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${QEMU_SOURCE_DIR}"
    )
endif()
