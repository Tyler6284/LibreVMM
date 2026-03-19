# =============================================================================
# Cross-compilation toolchain for macOS — multi-architecture support
# Supports: x86_64, arm64, ppc (PowerPC 32-bit), ppc64 (PowerPC 64-bit)
# OS range: Mac OS X 10.4 (Tiger) through latest macOS
# =============================================================================
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-macos-universal.cmake \
#         -DLIBREVMM_MACOS_ARCHS="x86_64;arm64" \
#         -B build
#
# Architecture/deployment target compatibility matrix:
#
#   Architecture  | Min Target | Max Target | Notes
#   --------------|------------|------------|----------------------------------
#   ppc           | 10.4       | 10.5       | Requires PowerPC cross-compiler
#   ppc64         | 10.4       | 10.5       | Requires PowerPC cross-compiler
#   i386          | 10.4       | 10.14      | Dropped in Catalina (10.15)
#   x86_64        | 10.5       | current    | First-class from Snow Leopard
#   arm64         | 11.0       | current    | Apple Silicon only
#
# Architecture group presets (set LIBREVMM_MACOS_ARCHS to one of these,
# or pass a custom semicolon-separated list):
#
#   modern-universal  →  x86_64;arm64         (macOS 11.0+)
#   legacy-intel      →  x86_64               (10.5 – current)
#   legacy-intel-full →  i386;x86_64          (10.5 – 10.14)
#   legacy-ppc        →  ppc;ppc64            (10.4 – 10.5, cross-compiler req.)
#   legacy-ppc32      →  ppc                  (10.4 – 10.5, cross-compiler req.)
#   legacy-ppc64      →  ppc64                (10.4 – 10.5, cross-compiler req.)
#   tiger-universal   →  ppc;ppc64;i386       (10.4 – 10.5, cross-compiler req.)
#
# PowerPC cross-compiler:
#   Modern Apple clang does not support ppc/ppc64 targets. A separate
#   cross-compiler is required. Set LIBREVMM_PPC_TOOLCHAIN_PREFIX to the
#   install prefix of your PowerPC toolchain, e.g.:
#     -DLIBREVMM_PPC_TOOLCHAIN_PREFIX=/opt/ppc-apple-darwin
#   or place powerpc-apple-darwin-gcc on your PATH.
#
# Combining ppc and arm64 architectures:
#   Apple's fat binary format (Mach-O universal) supports both ppc and arm64
#   slices technically, but the resulting binary will only run on its native
#   architecture. LibreVMM builds targeting both legacy ppc and modern arm64
#   are intended as separate distribution artifacts, not a single executable.
#   Use LIBREVMM_MACOS_ARCHS to produce each slice separately, then combine
#   with lipo if needed.
#
# Requirements:
#   - Xcode command line tools (for x86_64 and arm64)
#   - PowerPC cross-compiler (for ppc and ppc64)
#   - Old macOS SDK (10.4 or 10.5) for deployment targets below 10.6
#     Available from: https://github.com/phracker/MacOSX-SDKs
# =============================================================================

set(CMAKE_SYSTEM_NAME Darwin)

# =============================================================================
# User-configurable options
# =============================================================================

# Target architecture(s). Accepts a preset name or a semicolon-separated list
# of architecture names (ppc, ppc64, i386, x86_64, arm64).
set(LIBREVMM_MACOS_ARCHS "modern-universal" CACHE STRING
    "Target architecture(s): preset name or semicolon-separated list")

# Deployment target override. Leave empty to auto-select the minimum target
# that is valid for all requested architectures.
set(LIBREVMM_MACOS_DEPLOYMENT_TARGET "" CACHE STRING
    "macOS deployment target override (e.g. 10.4, 10.9, 11.0). \
     Leave empty for automatic selection.")

# PowerPC cross-compiler prefix. Only needed when ppc or ppc64 is in the
# architecture list. The toolchain will look for
# ${LIBREVMM_PPC_TOOLCHAIN_PREFIX}/bin/powerpc-apple-darwin-gcc.
set(LIBREVMM_PPC_TOOLCHAIN_PREFIX "" CACHE PATH
    "Install prefix for PowerPC cross-compiler (required for ppc/ppc64 targets)")

# =============================================================================
# Resolve architecture preset to a concrete list
# =============================================================================

if(LIBREVMM_MACOS_ARCHS STREQUAL "modern-universal")
    set(_RESOLVED_ARCHS "x86_64;arm64")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "legacy-intel")
    set(_RESOLVED_ARCHS "x86_64")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "legacy-intel-full")
    set(_RESOLVED_ARCHS "i386;x86_64")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "legacy-ppc")
    set(_RESOLVED_ARCHS "ppc;ppc64")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "legacy-ppc32")
    set(_RESOLVED_ARCHS "ppc")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "legacy-ppc64")
    set(_RESOLVED_ARCHS "ppc64")
elseif(LIBREVMM_MACOS_ARCHS STREQUAL "tiger-universal")
    set(_RESOLVED_ARCHS "ppc;ppc64;i386")
else()
    # Treat the value as a literal semicolon-separated list
    set(_RESOLVED_ARCHS "${LIBREVMM_MACOS_ARCHS}")
endif()

# Detect which architecture families are present
set(_HAS_PPC   FALSE)
set(_HAS_PPC64 FALSE)
set(_HAS_I386  FALSE)
set(_HAS_X8664 FALSE)
set(_HAS_ARM64 FALSE)

foreach(_ARCH IN LISTS _RESOLVED_ARCHS)
    if(_ARCH STREQUAL "ppc")
        set(_HAS_PPC TRUE)
    elseif(_ARCH STREQUAL "ppc64")
        set(_HAS_PPC64 TRUE)
    elseif(_ARCH STREQUAL "i386")
        set(_HAS_I386 TRUE)
    elseif(_ARCH STREQUAL "x86_64")
        set(_HAS_X8664 TRUE)
    elseif(_ARCH STREQUAL "arm64")
        set(_HAS_ARM64 TRUE)
    else()
        message(FATAL_ERROR
            "[LibreVMM] Unknown architecture '${_ARCH}'. "
            "Valid values: ppc, ppc64, i386, x86_64, arm64")
    endif()
endforeach()

# =============================================================================
# Validate architecture combinations and select deployment target
# =============================================================================

# arm64 requires 11.0 or later
if(_HAS_ARM64 AND (_HAS_PPC OR _HAS_PPC64))
    message(WARNING
        "[LibreVMM] arm64 and ppc/ppc64 cannot be combined into a single "
        "runnable fat binary. The build will produce separate architecture "
        "slices. Consider building each architecture group independently.")
endif()

# Determine the auto-selected minimum deployment target
if(_HAS_ARM64)
    set(_AUTO_TARGET "11.0")
elseif(_HAS_X8664)
    set(_AUTO_TARGET "10.5")
elseif(_HAS_I386)
    set(_AUTO_TARGET "10.4")
elseif(_HAS_PPC OR _HAS_PPC64)
    set(_AUTO_TARGET "10.4")
else()
    set(_AUTO_TARGET "10.4")
endif()

# Apply user override if provided, otherwise use auto-selected value
if(LIBREVMM_MACOS_DEPLOYMENT_TARGET STREQUAL "")
    set(_DEPLOYMENT_TARGET "${_AUTO_TARGET}")
else()
    set(_DEPLOYMENT_TARGET "${LIBREVMM_MACOS_DEPLOYMENT_TARGET}")
endif()

# Enforce known constraint: arm64 cannot deploy below 11.0
if(_HAS_ARM64)
    if(_DEPLOYMENT_TARGET VERSION_LESS "11.0")
        message(WARNING
            "[LibreVMM] arm64 requires deployment target >= 11.0. "
            "Overriding '${_DEPLOYMENT_TARGET}' to '11.0'.")
        set(_DEPLOYMENT_TARGET "11.0")
    endif()
endif()

# Enforce known constraint: ppc/ppc64 cannot deploy above 10.5
if(_HAS_PPC OR _HAS_PPC64)
    if(_DEPLOYMENT_TARGET VERSION_GREATER "10.5")
        message(WARNING
            "[LibreVMM] ppc/ppc64 are not supported above Mac OS X 10.5. "
            "Deployment target '${_DEPLOYMENT_TARGET}' may produce unusable "
            "binaries for those slices.")
    endif()
endif()

# Enforce known constraint: i386 was dropped in macOS 10.15 (Catalina)
if(_HAS_I386)
    if(_DEPLOYMENT_TARGET VERSION_GREATER_EQUAL "10.15")
        message(WARNING
            "[LibreVMM] i386 is not supported on macOS 10.15 (Catalina) or "
            "later. The i386 slice may fail to link or run.")
    endif()
endif()

# =============================================================================
# Apply resolved values to CMake cache variables
# =============================================================================

set(CMAKE_OSX_ARCHITECTURES "${_RESOLVED_ARCHS}" CACHE STRING
    "macOS target architectures" FORCE)

set(CMAKE_OSX_DEPLOYMENT_TARGET "${_DEPLOYMENT_TARGET}" CACHE STRING
    "macOS deployment target" FORCE)

set(CMAKE_SYSTEM_PROCESSOR "${_RESOLVED_ARCHS}")

# =============================================================================
# Compiler selection
# =============================================================================

set(_NEEDS_PPC_COMPILER FALSE)
if(_HAS_PPC OR _HAS_PPC64)
    set(_NEEDS_PPC_COMPILER TRUE)
endif()

if(NOT _NEEDS_PPC_COMPILER)
    # Standard path: use Apple clang for x86_64 and/or arm64
    find_program(CMAKE_C_COMPILER   NAMES clang   REQUIRED)
    find_program(CMAKE_CXX_COMPILER NAMES clang++ REQUIRED)
    set(CMAKE_C_COMPILER_ID   Clang)
    set(CMAKE_CXX_COMPILER_ID Clang)

else()
    # PowerPC path: requires a cross-compiler.
    # Search order:
    #   1. LIBREVMM_PPC_TOOLCHAIN_PREFIX/bin/
    #   2. PATH (standard cross-compiler naming convention)
    #   3. Homebrew prefix (common community install location)

    set(_PPC_GCC_NAMES
        powerpc-apple-darwin-gcc
        powerpc-apple-darwin8-gcc
        powerpc-apple-darwin9-gcc
        ppc-apple-darwin-gcc)
    set(_PPC_GXX_NAMES
        powerpc-apple-darwin-g++
        powerpc-apple-darwin8-g++
        powerpc-apple-darwin9-g++
        ppc-apple-darwin-g++)

    set(_PPC_SEARCH_PATHS "")
    if(NOT LIBREVMM_PPC_TOOLCHAIN_PREFIX STREQUAL "")
        list(APPEND _PPC_SEARCH_PATHS
            "${LIBREVMM_PPC_TOOLCHAIN_PREFIX}/bin")
    endif()
    # Homebrew cross-compiler install locations
    list(APPEND _PPC_SEARCH_PATHS
        "/opt/homebrew/bin"
        "/usr/local/bin"
        "/opt/local/bin")

    find_program(_PPC_C_COMPILER
        NAMES   ${_PPC_GCC_NAMES}
        PATHS   ${_PPC_SEARCH_PATHS}
        NO_DEFAULT_PATH)
    find_program(_PPC_C_COMPILER
        NAMES   ${_PPC_GCC_NAMES})

    find_program(_PPC_CXX_COMPILER
        NAMES   ${_PPC_GXX_NAMES}
        PATHS   ${_PPC_SEARCH_PATHS}
        NO_DEFAULT_PATH)
    find_program(_PPC_CXX_COMPILER
        NAMES   ${_PPC_GXX_NAMES})

    if(NOT _PPC_C_COMPILER)
        message(FATAL_ERROR
            "[LibreVMM] PowerPC cross-compiler not found. "
            "Install a powerpc-apple-darwin toolchain and either:\n"
            "  - Set LIBREVMM_PPC_TOOLCHAIN_PREFIX to its install prefix, or\n"
            "  - Add the toolchain bin/ directory to your PATH.\n"
            "Common options:\n"
            "  - Build via crosstool-ng targeting powerpc-apple-darwin9\n"
            "  - Use an archived Xcode 3.x toolchain (10.5 SDK era)\n"
            "  - https://github.com/tpoechtrager/cctools-port (includes ld)")
    endif()

    if(_HAS_X8664 OR _HAS_ARM64 OR _HAS_I386)
        # Mixed build: ppc + modern architecture. Use clang for modern slices
        # and the cross-compiler for ppc slices. CMake cannot drive two
        # compilers in a single build tree — this configuration produces the
        # ppc/ppc64 slice only. Run a separate clang build for the modern
        # slice and combine with lipo.
        message(WARNING
            "[LibreVMM] Mixed ppc and modern-architecture build requested. "
            "This toolchain file will compile only the ppc/ppc64 slice. "
            "Build the x86_64/arm64 slice separately with the default "
            "toolchain and combine slices using lipo.")
        set(_RESOLVED_ARCHS_PPC_ONLY "")
        if(_HAS_PPC)
            list(APPEND _RESOLVED_ARCHS_PPC_ONLY "ppc")
        endif()
        if(_HAS_PPC64)
            list(APPEND _RESOLVED_ARCHS_PPC_ONLY "ppc64")
        endif()
        set(CMAKE_OSX_ARCHITECTURES "${_RESOLVED_ARCHS_PPC_ONLY}" CACHE STRING
            "macOS target architectures (ppc slice only)" FORCE)
    endif()

    set(CMAKE_C_COMPILER   "${_PPC_C_COMPILER}"   CACHE FILEPATH "" FORCE)
    set(CMAKE_CXX_COMPILER "${_PPC_CXX_COMPILER}" CACHE FILEPATH "" FORCE)
    set(CMAKE_C_COMPILER_ID   GNU)
    set(CMAKE_CXX_COMPILER_ID GNU)
endif()

# =============================================================================
# SDK / sysroot
# =============================================================================

# For deployment targets below 10.6 a matching old SDK is required.
# xcrun cannot locate SDKs older than what the installed Xcode provides.
# Place old SDKs (from https://github.com/phracker/MacOSX-SDKs) under
# /Library/Developer/CommandLineTools/SDKs/ or point CMAKE_OSX_SYSROOT
# directly.
#
# For targets >= 10.6 with clang, xcrun will locate the current SDK
# automatically and the version-min flag enforces the deployment target.

if(_DEPLOYMENT_TARGET VERSION_LESS "10.6" AND NOT _NEEDS_PPC_COMPILER)
    # Try to locate an appropriately old SDK
    set(_OLD_SDK_PATHS
        "/Library/Developer/CommandLineTools/SDKs/MacOSX${_DEPLOYMENT_TARGET}.sdk"
        "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${_DEPLOYMENT_TARGET}.sdk"
        "/Developer/SDKs/MacOSX${_DEPLOYMENT_TARGET}.sdk")
    foreach(_SDK_PATH IN LISTS _OLD_SDK_PATHS)
        if(EXISTS "${_SDK_PATH}")
            set(CMAKE_OSX_SYSROOT "${_SDK_PATH}" CACHE PATH "" FORCE)
            break()
        endif()
    endforeach()
    if(NOT CMAKE_OSX_SYSROOT)
        message(WARNING
            "[LibreVMM] Could not locate MacOSX${_DEPLOYMENT_TARGET}.sdk. "
            "Building for deployment target ${_DEPLOYMENT_TARGET} without a "
            "matching SDK may produce incorrect results. "
            "Download old SDKs from https://github.com/phracker/MacOSX-SDKs")
    endif()
endif()

# =============================================================================
# Compiler flags
# =============================================================================

# -mmacosx-version-min is the canonical flag for clang and GCC alike
set(CMAKE_C_FLAGS_INIT
    "-mmacosx-version-min=${_DEPLOYMENT_TARGET}")
set(CMAKE_CXX_FLAGS_INIT
    "-mmacosx-version-min=${_DEPLOYMENT_TARGET}")

# ppc64: instruct the compiler to target 64-bit PowerPC ABI
if(_HAS_PPC64 AND NOT _HAS_PPC)
    string(APPEND CMAKE_C_FLAGS_INIT   " -m64")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -m64")
endif()

# i386: enforce 32-bit output when only i386 is requested
if(_HAS_I386 AND NOT _HAS_X8664 AND NOT _HAS_ARM64)
    string(APPEND CMAKE_C_FLAGS_INIT   " -m32")
    string(APPEND CMAKE_CXX_FLAGS_INIT " -m32")
endif()

# =============================================================================
# Search paths
# =============================================================================

set(CMAKE_FIND_ROOT_PATH
    "/usr/local"
    "/opt/local"
    "/opt/homebrew")

# Homebrew on Apple Silicon installs to /opt/homebrew; on Intel to /usr/local
if(EXISTS "/opt/homebrew")
    list(PREPEND CMAKE_FIND_ROOT_PATH "/opt/homebrew")
endif()

if(NOT LIBREVMM_PPC_TOOLCHAIN_PREFIX STREQUAL "")
    list(PREPEND CMAKE_FIND_ROOT_PATH "${LIBREVMM_PPC_TOOLCHAIN_PREFIX}")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# =============================================================================
# Frameworks
# =============================================================================

set(CMAKE_FRAMEWORK_SEARCH_PATHS
    /System/Library/Frameworks
    /Library/Frameworks
    ${CMAKE_SOURCE_DIR}/vendor/SDL/frameworks)

# =============================================================================
# Output directories — include arch tag so multi-slice builds don't collide
# =============================================================================

# Build a short tag from the resolved architecture list for directory naming
string(REPLACE ";" "-" _ARCH_TAG "${_RESOLVED_ARCHS}")
# Clean up the tag if it was overridden to ppc-only above
if(DEFINED _RESOLVED_ARCHS_PPC_ONLY)
    string(REPLACE ";" "-" _ARCH_TAG "${_RESOLVED_ARCHS_PPC_ONLY}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_SOURCE_DIR}/dist/macos-${_ARCH_TAG}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    "${CMAKE_SOURCE_DIR}/dist/macos-${_ARCH_TAG}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
    "${CMAKE_SOURCE_DIR}/dist/macos-${_ARCH_TAG}/lib")

# =============================================================================
# Summary
# =============================================================================

message(STATUS "[LibreVMM] macOS toolchain configured:")
message(STATUS "  Architectures     : ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "  Deployment target : ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  C compiler        : ${CMAKE_C_COMPILER}")
message(STATUS "  C++ compiler      : ${CMAKE_CXX_COMPILER}")
message(STATUS "  Output directory  : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
if(CMAKE_OSX_SYSROOT)
    message(STATUS "  SDK sysroot       : ${CMAKE_OSX_SYSROOT}")
endif()
