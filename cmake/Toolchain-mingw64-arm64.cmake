# =============================================================================
# Cross-compilation toolchain for Windows ARM64 using MinGW-w64
# =============================================================================
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw64-arm64.cmake -B build
# Requires: aarch64-linux-gnu cross-compiler or LLVM for Windows ARM64
# =============================================================================

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM64)

# ARM64 Windows toolchain prefix (using LLVM if available, otherwise aarch64-w64-mingw32)
if(DEFINED ENV{ARM64_TOOLCHAIN_PREFIX})
    set(COMPILER_PREFIX $ENV{ARM64_TOOLCHAIN_PREFIX})
else()
    set(COMPILER_PREFIX aarch64-w64-mingw32)
endif()

# Compiler paths
set(CMAKE_C_COMPILER ${COMPILER_PREFIX}-clang)
set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-clang++)
set(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-llvm-rc)
set(CMAKE_AR ${COMPILER_PREFIX}-llvm-ar)
set(CMAKE_RANLIB ${COMPILER_PREFIX}-llvm-ranlib)
set(CMAKE_LINKER ${COMPILER_PREFIX}-ld.lld)

# Search paths
set(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# Don't search for programs in the build host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Only search for libraries and includes in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows ARM64-specific flags
set(CMAKE_C_FLAGS_INIT "-mthreads -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00")
set(CMAKE_CXX_FLAGS_INIT "-mthreads -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-flavor ld.lld")

# Output directories for cross-compilation
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm64/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm64/lib)
