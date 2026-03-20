# LibreVMM Build Guide

This document provides complete build instructions for all supported host platforms and build profiles, covering the multi-build-system environment (kBuild, CMake, Meson, autoconf, Kconfig/Makefile).

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Quick Start](#quick-start)
3. [Build Profiles](#build-profiles)
4. [Platform-Specific Build Instructions](#platform-specific-build-instructions)
   - [Linux (x86_64)](#linux-x86_64)
   - [Linux (ARM64)](#linux-arm64)
   - [Windows (x86_64)](#windows-x86_64)
   - [Windows (ARM64)](#windows-arm64)
   - [Windows (ARM32)](#windows-arm32)
   - [macOS](#macos)
   - [BSD](#bsd)
   - [Haiku/BeOS](#beos)
   - [OpenIndiana](#indiana)
5. [Multi-Build-System Environment](#multi-build-system-environment)
6. [Vendor Build Integration](#vendor-build-integration)
7. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Required Tools

| Tool | Minimum Version | Purpose |
|------|-----------------|---------|
| CMake | 3.21 | Primary build system |
| C Compiler | C11 | Core compilation |
| C++ Compiler | C++17 | Core compilation |
| Meson | 0.59.0 | QEMU TCG build |
| Ninja | 1.10.0 | Meson backend |
| Autoconf | 2.69 | Bochs build |
| Make | 4.0 | SeaBIOS/OpenBIOS build |
| Kconfig | - | SeaBIOS/OpenBIOS configuration |

### Required Vendor Directories

Before building, ensure the following vendor directories exist in `vendor/`:

```
vendor/
├── virtualbox/      # Required - Core foundation
├── SDL/             # Required - Display/audio backend
├── qemu/            # Optional - TCG engine
├── 86Box/           # Optional - Legacy devices
├── Bochs/           # Optional - BIOS/devices
├── seabios/         # Optional - Firmware
└── openbios/        # Optional - Firmware
```

Run `scripts/setup-vendors.sh` to initialize vendor directories.

---

## Quick Start

### Basic Build (Desktop Full)

```bash
# Configure with default settings
cmake -B build

# Build
cmake --build build -j$(nproc)
```

### Minimal Build (Smoke Test)

```bash
# Minimal: core + TCG + one BIOS + no GUI
cmake -B build -DBUILD_PROFILE=minimal
cmake --build build -j$(nproc)
```

---

## Build Profiles

LibreVMM supports the following build profiles:

| Profile | Description | GUI | TCG | HM |
|---------|-------------|-----|-----|-----|
| `desktop-full` | Full desktop with Qt GUI (Multi-platform) | ✓ | ✓ | ✓ |
| `desktop-gdi` | Full desktop with Win32 GUI (Windows only) | ✓ | ✓ | ✓ |
| `desktop-nogui` | Desktop without GUI | ✗ | ✓ | ✓ |
| `uwp` | UWP sandbox build | ✓ | Depends on platform | Depends on platform |
| `android` | Android build | Mobile | Interpreter only | ✗ |
| `ios` | iOS build | Mobile | Interpreter only (unless JIT enabled)| Requires Apple Hypervisor to be enabled. |
| `headless-server` | Server without GUI | ✗ | ✓ | ✓ |
| `minimal` | Smoke test build | ✗ | ✓ | ✗ |

### Profile Selection

```bash
# Using CMake cache
cmake -B build -DBUILD_PROFILE=desktop-full

# Using command-line options
cmake -B build -DBUILD_QT=ON -DBUILD_TCG=ON -DEXEC_HM=ON
```

---

## Platform-Specific Build Instructions

### Linux (x86_64)

#### Native Build

```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    meson \
    python3 \
    python3-jinja2 \
    git \
    wget \
    xz-utils \
    libssl-dev \
    libpulse-dev \
    libasound2-dev \
    libgtk-3-dev \
    libqt5opengl5-dev \
    libxkbcommon-dev \
    libdbus-1-dev

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

#### Cross-Compilation for ARM64

```bash
# Install cross-compiler
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Configure with cross-compilation toolchain
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-linux-aarch64.cmake \
    -G Ninja

# Build
cmake --build build -j$(nproc)
```

### Linux (ARM64)

#### Native Build

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    meson \
    python3 \
    python3-jinja2 \
    git \
    wget \
    xz-utils \
    libssl-dev \
    libpulse-dev \
    libasound2-dev

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure (native)
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

### Windows (x86_64)

#### Using MinGW-w64

```bash
# Install MSYS2
winget install msys2.msys2

# Open MSYS2 MinGW x64 terminal and install dependencies
pacman -S \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-cmake \
    mingw-w64-x86_64-ninja \
    mingw-w64-x86_64-meson \
    mingw-w64-x86_64-python-jinja2 \
    mingw-w64-x86_64-libwinpthread-git \
    mingw-w64-x86_64-zlib \
    mingw-w64-x86_64-libpng \
    mingw-w64-x86_64 SDL2

# Navigate to project
cd /c/Projects/LibreVMM

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

#### Cross-Compilation from Linux

```bash
# Install MinGW-w64
sudo apt-get install -y mingw-w64

# Configure for Windows x86_64
cmake -B build-win \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw64-x86_64.cmake \
    -G Ninja

# Build
cmake --build build-win -j$(nproc)
```

### Windows (ARM64)

#### Cross-Compilation from Linux

```bash
# Install LLVM for Windows ARM64 cross-compilation
sudo apt-get install -y clang lld

# Configure for Windows ARM64
cmake -B build-win-arm64 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw64-arm64.cmake \
    -G Ninja

# Build
cmake --build build-win-arm64 -j$(nproc)
```

### Windows (ARM32)

#### Cross-Compilation from Linux

```bash
# Install ARM32 cross-compiler
sudo apt-get install -y gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

# Configure for Windows ARM32 (using MinGW)
cmake -B build-win-arm32 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-mingw32.cmake \
    -G Ninja

# Build
cmake --build build-win-arm32 -j$(nproc)
```

Note: Windows ARM32 builds are primarily for Windows on ARM tablets and the Windows 10X emulator. For most use cases, prefer x86_64 or ARM64 builds.

### macOS

#### Universal Binary (x86_64 + ARM64)

```bash
# Install Xcode command line tools
xcode-select --install

# Install Homebrew dependencies
brew install cmake ninja meson jinja2

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure with universal binary support
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-macos-universal.cmake \
    -DLIBREVMM_MACOS_ARCHS="modern-universal" \
    -G Ninja

# Build
cmake --build build -j$(nproc)
```

#### Legacy Intel Only (10.5+)

```bash
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-macos-universal.cmake \
    -DLIBREVMM_MACOS_ARCHS="legacy-intel" \
    -G Ninja
```

#### Apple Silicon Only (11.0+)

```bash
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain-macos-universal.cmake \
    -DLIBREVMM_MACOS_ARCHS="arm64" \
    -G Ninja
```

### BSD

LibreVMM supports FreeBSD, OpenBSD, and NetBSD as host platforms.

#### FreeBSD

```bash
# Install dependencies
sudo pkg update
sudo pkg install -y \
    cmake \
    ninja \
    meson \
    python3 \
    git \
    llvm-toolchain

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

#### OpenBSD

```bash
# Install dependencies
doas pkg_add cmake ninja meson python3

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

#### NetBSD

```bash
# Install dependencies
pkgin install cmake ninja meson python3

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

**Note:** BSD builds use the native clang toolchain. The TCG JIT may have limited functionality on BSD due to differences in executable memory allocation APIs. Use the interpreter mode (`EXEC_TCG_INTERP=ON`) for reliable operation.

### Haiku/BeOS

Haiku (the modern open source reimplementation of BeOS) can host LibreVMM for x86_64 guests.

```bash
# Install Haiku buildtools
# Download from https://www.haiku-os.org/guides/building

# Install dependencies via HaikuPorter
pkgman install cmake ninja meson python3

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure (Haiku uses clang)
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

**Note:**
- Haiku builds are experimental and require the Haiku nightly build environment
- Only the TCG interpreter mode is supported (no JIT due to memory constraints)
- Qt GUI is not available on Haiku; use headless or SDL-based output

### OpenIndiana

OpenIndiana (illumos-based open source Solaris derivative) can host LibreVMM.

```bash
# Install dependencies
sudo pkg install -y \
    developer/build/cmake \
    developer/ninja \
    runtime/python-3 \
    git

# Install Meson via pip
sudo pip3 install meson jinja2

# Clone and setup vendors
git clone https://github.com/librevmm/LibreVMM.git
cd LibreVMM
./scripts/setup-vendors.sh

# Configure
cmake -B build -G Ninja

# Build
cmake --build build -j$(nproc)
```

**Note:**
- OpenIndiana uses GCC or Clang depending on installation
- For best results, use the Illumos GCC:
  ```bash
  cmake -B build -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
  ```
- TCG JIT requires memory allocation from `/tmp` or a dedicated heap due to SMELF restrictions

---

## Multi-Build-System Environment

LibreVMM integrates multiple build systems:

| Component | Build System | Integration |
|----------|-------------|-------------|
| VirtualBox Core | kBuild | Wrapper via CMake |
| QEMU TCG | Meson | Subproject |
| 86Box | CMake | Subproject |
| Bochs | Autoconf | Subproject |
| SeaBIOS | Makefile/Kconfig | Subproject |
| OpenBIOS | Makefile/Kconfig | Subproject |
| SDL2 | CMake | Subproject |

### Build System Flow

```
CMake (main) → [kBuild wrapper] → VirtualBox
           → [Meson] → QEMU (TCG)
           → [CMake] → 86Box
           → [Autoconf] → Bochs
           → [Make/Kconfig] → SeaBIOS
           → [Make/Kconfig] → OpenBIOS
           → [CMake] → SDL2
```

### Subproject Integration

Each subproject is built as a static library producing:

- `libtcg_x86` - QEMU TCG engine (i386/x86_64)
- `lib86box_devices` - 86Box device modules
- `libbochs_devices` - Bochs device modules
- `bios-seabios.bin` - SeaBIOS firmware binary
- `bios-openbios.bin` - OpenBIOS firmware binary

---

## Vendor Build Integration

### QEMU (Meson)

The QEMU Meson build produces `libtcg_x86` for software emulation:

```bash
# Configure QEMU with TCG only (no other targets)
meson setup build-qemu \
    --wipe \
    -Dtcg=true \
    -Dkvm=false \
    -Dwhpx=disabled \
    -Dhv-fence=disabled \
    -Dxen=disabled \
    -Dslirp=disabled \
    -Dcurl=disabled \
    -Dcapstone=disabled \
    -Dfdt=disabled \
    -Dmodules=disabled \
    -Dpie=false \
    --default-library=static \
    --buildtype=release

# Build only TCG-related targets
ninja -C build-qemu libtcg_x86
```

### 86Box (CMake)

```bash
# Configure 86Box as subproject
cmake -B build-86box \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_QEMU=no \
    -DENABLE_VHD=no \
    -DENABLE_VNC=no \
    -DENABLE_WX=no \
    -DENABLE_NEW_DYNAREC=YES

# Build device library only
cmake --build build-86box --target lib86box_devices
```

### SeaBIOS (Makefile/Kconfig)

```bash
# Configure SeaBIOS
cd vendor/seabios
make menuconfig  # Or use defconfig

# Build
make -j$(nproc)

# Output: out/bios.bin
```

---

## Troubleshooting

### Common Issues

#### Missing Vendor Directory

```
Fatal error: Required vendor directory missing: virtualbox
```

**Solution**: Run `./scripts/setup-vendors.sh` to initialize vendor directories.

#### CMake Version Too Old

```
CMake Error: CMake version 3.21 required
```

**Solution**: Install a newer CMake version:
- Linux: `pip install cmake --upgrade` or use conda
- macOS: `brew install cmake`
- Windows: Download from cmake.org

#### Meson Build Fails

```
meson.build:1:0: ERROR: Unknown option 'kvm'
```

**Solution**: Use the correct Meson version. LibreVMM requires Meson 0.59.0+.

#### Cross-Compilation Toolchain Not Found

```
Could not find compiler
```

**Solution**: Install the required cross-compiler:
- Linux ARM64: `sudo apt-get install gcc-aarch64-linux-gnu`
- Windows ARM64: Install LLVM

#### Qt5 Not Found

```
Could NOT find Qt5 (missing: Qt5_DIR)
```

**Solution**: Install Qt5 development packages:
- Linux: `sudo apt-get install qtbase5-dev`
- macOS: `brew install qt@5`

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_TCG` | ON | Build TCG engine |
| `BUILD_QEMU_DEVICES` | ON | Import QEMU devices |
| `BUILD_86BOX_DEVICES` | ON | Import 86Box devices |
| `BUILD_BOCHS_DEVICES` | ON | Import Bochs devices |
| `BUILD_SEABIOS` | ON | Build SeaBIOS firmware |
| `BUILD_OPENBIOS` | ON | Build OpenBIOS firmware |
| `BUILD_SDL` | ON | Build with SDL2 support |
| `BUILD_QT` | ON | Build Qt GUI frontend |
| `BUILD_HEADLESS` | OFF | Build headless variant |
| `EXEC_HM` | ON | Enable hardware virtualization |
| `EXEC_TCG_JIT` | ON | Enable TCG JIT compilation |
| `EXEC_TCG_INTERP` | OFF | Enable TCG interpreter |

### Performance Tuning

- Use Ninja generator for faster builds: `-G Ninja`
- Enable parallel builds: `-j$(nproc)`
- Use ccache: `brew install ccache` or `sudo apt-get install ccache`

---

## Next Steps

- See [ARCHITECTURE.md](../ARCHITECTURE.md) for system architecture
- See [DEVICE_PORTING_GUIDE.md](../DEVICE_PORTING_GUIDE.md) for device import
- See [TODO.md](../TODO.md) for project roadmap
