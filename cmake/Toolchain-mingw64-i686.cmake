# =============================================================================
# Cross-compilation toolchain for Windows x86 (32-bit) using MinGW-w64
# =============================================================================
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw64-i686.cmake -B build
# =============================================================================

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# MinGW-w64 toolchain prefix (32-bit)
set(COMPILER_PREFIX i686-w64-mingw32)

# Compiler paths
set(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${COMPILER_PREFIX}-windres)
set(CMAKE_AR ${COMPILER_PREFIX}-ar)
set(CMAKE_RANLIB ${COMPILER_PREFIX}-ranlib)
set(CMAKE_LINKER ${COMPILER_PREFIX}-ld)

# Search paths
set(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# Don't search for programs in the build host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Only search for libraries and includes in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Windows-specific flags
set(CMAKE_C_FLAGS_INIT "-mthreads -DWINVER=0x0601 -D_WIN32_WINNT=0x0601")
set(CMAKE_CXX_FLAGS_INIT "-mthreads -DWINVER=0x0601 -D_WIN32_WINNT=0x0601")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")

# Output directories for cross-compilation
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-i686)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-i686/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/win-i686/lib)
