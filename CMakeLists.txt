# =============================================================================
# LibreVMM - Unified Build System Entry Point
# =============================================================================
#
# This is the main CMakeLists.txt that orchestrates the multi-build-system
# environment for LibreVMM. It integrates:
#   - VirtualBox core (kBuild)
#   - QEMU TCG engine (Meson) -> libtcg_x86
#   - 86Box devices (CMake)
#   - Bochs devices/BIOS (autoconf)
#   - SeaBIOS firmware (Kconfig/Makefile)
#   - OpenBIOS firmware (Kconfig/Makefile)
#   - SDL2 (CMake)
#
# =============================================================================

cmake_minimum_required(VERSION 3.21)
project(LibreVMM VERSION 0.1.0 LANGUAGES C CXX ASM)

# -----------------------------------------------------------------------------
# Global Configuration
# -----------------------------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable strict checking by default
if(NOT DEFINED CMAKE_C_FLAGS_INIT)
    set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -Werror=implicit-function-declaration")
endif()
if(NOT DEFINED CMAKE_CXX_FLAGS_INIT)
    set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -Werror")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# -----------------------------------------------------------------------------
# Include Directories
# -----------------------------------------------------------------------------
# Project include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/devices)
include_directories(${CMAKE_SOURCE_DIR}/resources)

# Vendor include directories
include_directories(${CMAKE_SOURCE_DIR}/vendor)
if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/virtualbox)
    include_directories(${CMAKE_SOURCE_DIR}/vendor/virtualbox/src/VBox/Additions)
endif()
if(EXISTS ${CMAKE_SOURCE_DIR}/vendor/qemu)
    include_directories(${CMAKE_SOURCE_DIR}/vendor/qemu/include)
    include_directories(${CMAKE_SOURCE_DIR}/vendor/qemu/target/i386)
endif()

# -----------------------------------------------------------------------------
# Platform Detection
# -----------------------------------------------------------------------------
if(WIN32)
    set(TARGET_PLATFORM "windows")
    message(STATUS "Target platform: Windows")
elseif(APPLE)
    set(TARGET_PLATFORM "macos")
    message(STATUS "Target platform: macOS")
elseif(UNIX)
    set(TARGET_PLATFORM "linux")
    message(STATUS "Target platform: Linux")
endif()

# -----------------------------------------------------------------------------
# Build Options
# -----------------------------------------------------------------------------
option(BUILD_TCG "Build TCG (Tiny Code Generator) software emulation" ON)
option(BUILD_QEMU_DEVICES "Import QEMU devices" ON)
option(BUILD_86BOX_DEVICES "Import 86Box devices" ON)
option(BUILD_BOCHS_DEVICES "Import Bochs devices" ON)
option(BUILD_SEABIOS "Build SeaBIOS firmware" ON)
option(BUILD_OPENBIOS "Build OpenBIOS firmware" ON)
option(BUILD_SDL "Build with SDL2 support" ON)
option(BUILD_QT "Build Qt GUI frontend" ON)
option(BUILD_HEADLESS "Build headless/server variant (no GUI)" OFF)
option(BUILD_TESTS "Build test suite" ON)

# Execution backend options
option(EXEC_HM "Enable hardware virtualization (VT-x/AMD-V)" ON)
option(EXEC_TCG_JIT "Enable TCG JIT compilation" ON)
option(EXEC_TCG_INTERP "Enable TCG interpreter (no-JIT mode)" OFF)

# Platform options
option(TARGET_UWP "Build for UWP restricted environment" OFF)
option(TARGET_IOS "Build for iOS (JIT disabled)" OFF)
option(TARGET_ANDROID "Build for Android" OFF)

# VirtualBox options
option(VBOX_EXT_DEVICES "Enable external device plugins" ON)

# -----------------------------------------------------------------------------
# Build Profiles
# -----------------------------------------------------------------------------
# Define build profile presets
set(BUILD_PROFILE "desktop-full" CACHE STRING "Build profile: desktop-full, desktop-nogui, uwp-restricted, android, ios, headless-server, minimal")

# Profile configurations
set(PROFILE_DESKTOP_FULL
    "BUILD_QT=ON"
    "BUILD_HEADLESS=OFF"
    "EXEC_HM=ON"
    "EXEC_TCG_JIT=ON"
    "EXEC_TCG_INTERP=OFF"
    "TARGET_UWP=OFF"
)

set(PROFILE_DESKTOP_NOGUI
    "BUILD_QT=OFF"
    "BUILD_HEADLESS=ON"
    "EXEC_HM=ON"
    "EXEC_TCG_JIT=ON"
    "EXEC_TCG_INTERP=OFF"
    "TARGET_UWP=OFF"
)

set(PROFILE_UWP_RESTRICTED
    "BUILD_QT=ON"
    "BUILD_HEADLESS=OFF"
    "EXEC_HM=OFF"
    "EXEC_TCG_JIT=OFF"
    "EXEC_TCG_INTERP=ON"
    "TARGET_UWP=ON"
)

set(PROFILE_ANDROID
    "BUILD_QT=OFF"
    "BUILD_HEADLESS=OFF"
    "EXEC_HM=OFF"
    "EXEC_TCG_JIT=OFF"
    "EXEC_TCG_INTERP=ON"
    "TARGET_ANDROID=ON"
)

set(PROFILE_IOS
    "BUILD_QT=OFF"
    "BUILD_HEADLESS=OFF"
    "EXEC_HM=OFF"
    "EXEC_TCG_JIT=OFF"
    "EXEC_TCG_INTERP=ON"
    "TARGET_IOS=ON"
)

set(PROFILE_HEADLESS_SERVER
    "BUILD_QT=OFF"
    "BUILD_HEADLESS=ON"
    "EXEC_HM=ON"
    "EXEC_TCG_JIT=ON"
    "EXEC_TCG_INTERP=OFF"
    "TARGET_UWP=OFF"
)

# Minimal smoke-test build: core + TCG + one BIOS + no GUI
set(PROFILE_MINIMAL
    "BUILD_QT=OFF"
    "BUILD_HEADLESS=ON"
    "BUILD_TCG=ON"
    "EXEC_HM=OFF"
    "EXEC_TCG_JIT=ON"
    "EXEC_TCG_INTERP=OFF"
    "BUILD_QEMU_DEVICES=OFF"
    "BUILD_86BOX_DEVICES=OFF"
    "BUILD_BOCHS_DEVICES=OFF"
    "BUILD_SEABIOS=ON"
    "BUILD_OPENBIOS=OFF"
    "BUILD_SDL=OFF"
)

# Apply build profile if specified
if(NOT BUILD_PROFILE STREQUAL "custom")
    set(PROFILE_VARS "")
    if(BUILD_PROFILE STREQUAL "desktop-full")
        set(PROFILE_VARS ${PROFILE_DESKTOP_FULL})
    elseif(BUILD_PROFILE STREQUAL "desktop-nogui")
        set(PROFILE_VARS ${PROFILE_DESKTOP_NOGUI})
    elseif(BUILD_PROFILE STREQUAL "uwp-restricted")
        set(PROFILE_VARS ${PROFILE_UWP_RESTRICTED})
    elseif(BUILD_PROFILE STREQUAL "android")
        set(PROFILE_VARS ${PROFILE_ANDROID})
    elseif(BUILD_PROFILE STREQUAL "ios")
        set(PROFILE_VARS ${PROFILE_IOS})
    elseif(BUILD_PROFILE STREQUAL "headless-server")
        set(PROFILE_VARS ${PROFILE_HEADLESS_SERVER})
    elseif(BUILD_PROFILE STREQUAL "minimal")
        set(PROFILE_VARS ${PROFILE_MINIMAL})
    else()
        message(WARNING "Unknown build profile: ${BUILD_PROFILE}")
    endif()

    # Apply profile options
    foreach(OPT ${PROFILE_VARS})
        string(REPLACE "=" ";" OPT_LIST ${OPT})
        list(LENGTH OPT_LIST OPT_LEN)
        if(OPT_LEN EQUAL 2)
            list(GET OPT_LIST 0 OPT_NAME)
            list(GET OPT_LIST 1 OPT_VALUE)
            set(${OPT_NAME} ${OPT_VALUE} CACHE BOOL "" FORCE)
        endif()
    endforeach()

    message(STATUS "Build profile: ${BUILD_PROFILE}")
    message(STATUS "Profile options: ${PROFILE_VARS}")
endif()

# -----------------------------------------------------------------------------
# Vendor Directory Validation
# -----------------------------------------------------------------------------
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)

# Required vendor directories
set(REQUIRED_VENDORS
    virtualbox
    SDL
)

# Optional vendor directories
set(OPTIONAL_VENDORS
    qemu
    86Box
    Bochs
    seabios
    openbios
    dosbox-x
    kvm-guest-drivers-windows
)

# Validate required vendors exist
foreach(VENDOR ${REQUIRED_VENDORS})
    if(NOT EXISTS ${VENDOR_DIR}/${VENDOR})
        message(FATAL_ERROR "Required vendor directory missing: ${VENDOR_DIR}/${VENDOR}")
    endif()
    message(STATUS "Found required vendor: ${VENDOR}")
endforeach()

# Validate optional vendors
foreach(VENDOR ${OPTIONAL_VENDORS})
    if(EXISTS ${VENDOR_DIR}/${VENDOR})
        message(STATUS "Found optional vendor: ${VENDOR}")
        set(HAS_${VENDOR} TRUE)
    else()
        message(STATUS "Optional vendor not found: ${VENDOR}")
        set(HAS_${VENDOR} FALSE)
    endif()
endforeach()

# Conditional feature enables based on vendor availability
if(HAS_qemu AND BUILD_TCG)
    set(ENABLE_TCG TRUE)
else()
    set(ENABLE_TCG FALSE)
    if(BUILD_TCG)
        message(WARNING "QEMU vendor not found - TCG support will be disabled")
    endif()
endif()

if(HAS_86Box AND BUILD_86BOX_DEVICES)
    set(ENABLE_86BOX TRUE)
else()
    set(ENABLE_86BOX FALSE)
    if(BUILD_86BOX_DEVICES AND NOT HAS_86Box)
        message(WARNING "86Box vendor not found - 86Box devices will be disabled")
    endif()
endif()

if(HAS_Bochs AND BUILD_BOCHS_DEVICES)
    set(ENABLE_BOCHS TRUE)
else()
    set(ENABLE_BOCHS FALSE)
endif()

if(HAS_seabios AND BUILD_SEABIOS)
    set(ENABLE_SEABIOS TRUE)
else()
    set(ENABLE_SEABIOS FALSE)
endif()

if(HAS_openbios AND BUILD_OPENBIOS)
    set(ENABLE_OPENBIOS TRUE)
else()
    set(ENABLE_OPENBIOS FALSE)
endif()

# -----------------------------------------------------------------------------
# VBOX_EXT_DEVICES_DIR - External Device Modules Directory
# -----------------------------------------------------------------------------
# Device modules are compiled into this directory and loaded at runtime
set(VBOX_EXT_DEVICES_DIR "ext-devices" CACHE PATH "Directory for external device modules")

# Create the output directory for device modules
set(VBOX_EXT_DEVICES_OUTPUT_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${VBOX_EXT_DEVICES_DIR})
file(MAKE_DIRECTORY ${VBOX_EXT_DEVICES_OUTPUT_DIR})

message(STATUS "External devices directory: ${VBOX_EXT_DEVICES_DIR}")
message(STATUS "External devices output: ${VBOX_EXT_DEVICES_OUTPUT_DIR}")

# -----------------------------------------------------------------------------
# Platform Detection
# -----------------------------------------------------------------------------
if(WIN32)
    set(HOST_OS "Windows")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x64")
        set(HOST_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|aarch64")
        set(HOST_ARCH "ARM64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86|i386|i686")
        set(HOST_ARCH "x86")
    endif()
elseif(UNIX)
    if(APPLE)
        set(HOST_OS "macOS")
    else()
        set(HOST_OS "Linux")
    endif()
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
        set(HOST_ARCH "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(HOST_ARCH "ARM64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm*")
        set(HOST_ARCH "ARM32")
    endif()
endif()

message(STATUS "Host OS: ${HOST_OS}")
message(STATUS "Host architecture: ${HOST_ARCH}")

# -----------------------------------------------------------------------------
# Compiler Feature Detection
# -----------------------------------------------------------------------------
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)
include(CheckFunctionExists)

# Check for required headers and functions
check_include_file("stdint.h HAVE_STDINT_H")
check_include_file("stdbool.h HAVE_STDBOOL_H")
check_include_file("string.h HAVE_STRING_H")
check_include_file("stdlib.h HAVE_STDLIB_H")

# -----------------------------------------------------------------------------
# Subdirectory Inclusion
# -----------------------------------------------------------------------------
# Add cmake modules path for custom FindPackage modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Include subdirectories based on build options
add_subdirectory(src/core)
add_subdirectory(src/vmm)

if(ENABLE_TCG)
    add_subdirectory(vendor/qemu)
endif()

if(ENABLE_86BOX)
    add_subdirectory(vendor/86Box)
endif()

if(ENABLE_BOCHS)
    add_subdirectory(vendor/Bochs/bochs)
endif()

if(ENABLE_SEABIOS)
    add_subdirectory(vendor/seabios)
endif()

if(ENABLE_OPENBIOS)
    add_subdirectory(vendor/openbios)
endif()

if(BUILD_SDL)
    add_subdirectory(vendor/SDL)
endif()

if(BUILD_QT)
    add_subdirectory(src/frontend/qt)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Build Summary
# -----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "========================================")
message(STATUS "LibreVMM Build Configuration Summary")
message(STATUS "========================================")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build profile: ${BUILD_PROFILE}")
message(STATUS "Host OS: ${HOST_OS}")
message(STATUS "Host architecture: ${HOST_ARCH}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Features:")
message(STATUS "  TCG: ${ENABLE_TCG}")
message(STATUS "  86Box devices: ${ENABLE_86BOX}")
message(STATUS "  Bochs devices: ${ENABLE_BOCHS}")
message(STATUS "  SeaBIOS: ${ENABLE_SEABIOS}")
message(STATUS "  OpenBIOS: ${ENABLE_OPENBIOS}")
message(STATUS "  SDL2: ${BUILD_SDL}")
message(STATUS "  Qt GUI: ${BUILD_QT}")
message(STATUS "  Headless: ${BUILD_HEADLESS}")
message(STATUS "")
message(STATUS "Execution backends:")
message(STATUS "  HM (Hardware): ${EXEC_HM}")
message(STATUS "  TCG JIT: ${EXEC_TCG_JIT}")
message(STATUS "  TCG Interpreter: ${EXEC_TCG_INTERP}")
message(STATUS "========================================")
message(STATUS "")
