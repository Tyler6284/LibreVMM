# =============================================================================
# Cross-compilation toolchain for Windows ARM32 (ARMv7) using MinGW-w64
# =============================================================================
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw64-arm32.cmake -B build
# Note: Windows ARM32 is primarily for Windows on ARM tablets, not common on desktop
# =============================================================================

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# ARM32 Windows toolchain prefix
if(DEFINED ENV{ARM32_TOOLCHAIN_PREFIX})
    set(COMPILER_PREFIX $ENV{ARM32_TOOLCHAIN_PREFIX})
else()
    # Try to use clang for ARM32 Windows
    set(COMPILER_PREFIX armv7-w64-mingw32)
endif()

# Compiler paths (using clang if available)
find_program(CMAKE_C_COMPILER NAMES ${COMPILER_PREFIX}-clang clang)
find_program(CMAKE_CXX_COMPILER NAMES ${COMPILER_PREFIX}-clang++ clang++)
find_program(CMAKE_AR NAMES ${COMPILER_PREFIX}-llvm-ar llvm-ar)
find_program(CMAKE_RANLIB NAMES ${COMPILER_PREFIX}-llvm-ranlib llvm-ranlib)
find_program(CMAKE_LINKER NAMES ${COMPILER_PREFIX}-ld.lld ld.lld)
find_program(CMAKE_RC_COMPILER NAMES ${COMPILER_PREFIX}-llvm-rc llvm-rc)

# Search paths
set(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# Don't search for programs in the build host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Only search for libraries and includes in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows ARM32-specific flags
set(CMAKE_C_FLAGS_INIT "-mthreads -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -march=armv7")
set(CMAKE_CXX_FLAGS_INIT "-mthreads -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -march=armv7")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-flavor ld.lld")

# Output directories for cross-compilation
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm32)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm32/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-arm32/lib)
