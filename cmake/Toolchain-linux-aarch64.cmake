# =============================================================================
# Cross-compilation toolchain for Linux ARM64 (aarch64)
# =============================================================================
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-linux-aarch64.cmake -B build
# Requires: aarch64-linux-gnu-gcc, aarch64-linux-gnu-g++ toolchain installed
# =============================================================================

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# ARM64 toolchain prefix
set(COMPILER_PREFIX aarch64-linux-gnu)

# Compiler paths
set(CMAKE_C_COMPILER ${COMPILER_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}-g++)
set(CMAKE_AR ${COMPILER_PREFIX}-ar)
set(CMAKE_RANLIB ${COMPILER_PREFIX}-ranlib)
set(CMAKE_LINKER ${COMPILER_PREFIX}-ld)
set(CMAKE_STRIP ${COMPILER_PREFIX}-strip)

# Search paths - adjust for your cross-compilation sysroot if needed
set(CMAKE_FIND_ROOT_PATH /usr/${COMPILER_PREFIX})

# Don't search for programs in the build host
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# Only search for libraries and includes in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ARM64-specific flags
set(CMAKE_C_FLAGS_INIT "-march=armv8-a")
set(CMAKE_CXX_FLAGS_INIT "-march=armv8-a")

# Output directories for cross-compilation
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/linux-aarch64)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/linux-aarch64/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/linux-aarch64/lib)
