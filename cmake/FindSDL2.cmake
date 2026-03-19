# =============================================================================
# FindSDL2.cmake
# =============================================================================
# Finds the SDL2 vendor sources
#
# This module defines:
#   SDL2_FOUND      - True if SDL2 sources are available
#   SDL2_SOURCE_DIR - Path to the SDL2 source directory
#   SDL2_VERSION    - Detected SDL2 version
#
# Optional:
#   SDL2_ROOT_DIR   - Override the default search path
# =============================================================================

set(_sdl2_search_paths
    ${CMAKE_SOURCE_DIR}/vendor/SDL
    ${CMAKE_SOURCE_DIR}/../vendor/SDL
    ${CMAKE_SOURCE_DIR}/vendor/SDL2
    ${CMAKE_SOURCE_DIR}/../vendor/SDL2
)

if(SDL2_ROOT_DIR)
    list(INSERT _sdl2_search_paths 0 ${SDL2_ROOT_DIR})
endif()

# Find the SDL2 source directory
find_path(SDL2_SOURCE_DIR
    NAMES
        CMakeLists.txt
        include/SDL.h
    PATHS
        ${_sdl2_search_paths}
    NO_DEFAULT_PATH
)

if(SDL2_SOURCE_DIR)
    # Try to detect version
    if(EXISTS ${SDL2_SOURCE_DIR}/VERSION)
        file(READ ${SDL2_SOURCE_DIR}/VERSION SDL2_VERSION)
        string(STRIP "${SDL2_VERSION}" SDL2_VERSION)
    elseif(EXISTS ${SDL2_SOURCE_DIR}/include/SDL_version.h)
        file(STRINGS ${SDL2_SOURCE_DIR}/include/SDL_version.h SDL2_VERSION_STRING
            REGEX "SDL_VERSION"
            LIMIT_COUNT 1)
        if(SDL2_VERSION_STRING)
            string(REGEX REPLACE ".*\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*" "\\1" 
                SDL2_VERSION "${SDL2_VERSION_STRING}")
        endif()
    endif()
    
    # Set standard variables
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SDL2
        REQUIRED_VARS
            SDL2_SOURCE_DIR
        VERSION_VAR
            SDL2_VERSION
    )
    
    mark_as_advanced(SDL2_SOURCE_DIR)
endif()

# Create imported target
if(SDL2_FOUND AND NOT TARGET SDL2::SDL2)
    add_library(SDL2::SDL2 INTERFACE IMPORTED)
    set_target_properties(SDL2::SDL2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_SOURCE_DIR}/include"
    )
endif()
