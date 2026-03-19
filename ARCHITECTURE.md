# LibreVMM — Architecture Overview

## Project Goals

LibreVMM aims to build off the core VirtualBox virtualization platform with additional features and compatibility from other open-source projects, including:

- **QEMU**: TCG execution engine and modern device models
- **86Box**: Legacy device emulation for retro computing
- **DOSBox-X**: Dynamic CPU cycle scaling and additional legacy hardware
- **Bochs**: Interpreter-based execution for restricted environments
- **SeaBIOS/OpenBIOS/Bochs BIOS**: Multiple firmware options for compatibility

## Core Architecture

The project preserves VirtualBox's existing architecture while adding new extensibility layers:

### 1. Device Abstraction Layer (DAL)

The DAL provides a unified interface for all device implementations, regardless of origin. It ensures that:

- All devices (QEMU, 86Box, Bochs, native VBox) are treated uniformly
- Devices are dynamically discoverable and loadable
- Standard bus attachment points are defined (ISA, PCI, PCIe, LPC, USB, AGP, MCA)
- IRQ, DMA, MMIO, and port I/O registration APIs are host-agnostic

### 2. Execution Backend Abstraction

The execution backend abstraction allows pluggable CPU execution engines:

- **Native VMM**: VirtualBox's existing VT-x/AMD-V hardware virtualization
- **QEMU TCG**: Software JIT compilation for systems without VT-x/AMD-V
- **QEMU TCG Interpreter**: No-JIT fallback for restricted environments like UWP

### 3. Firmware Selection

Users can select from multiple firmware options per VM:

- **OVMF**: Existing UEFI support
- **SeaBIOS**: Legacy BIOS option
- **Bochs BIOS**: Compatibility-focused BIOS
- **OpenBIOS**: Open-source firmware with advanced features

### 4. Build System

The project uses a unified build system that integrates multiple subprojects:

- **VirtualBox**: kBuild
- **QEMU**: Meson
- **86Box**: CMake + vcpkg
- **Bochs**: Autoconf

## Directory Structure

```
LibreVMM/
├── .gitignore                          # Git ignore rules
├── LICENSE                             # Project license (GPL-3.0)
├── README.md                           # Project overview and author credits
├── TRANSPARENCY.md                     # Vibe-code transparency statement
├── CODE_OF_CONDUCT.md                  # Contributor code of conduct
├── ARCHITECTURE.md                     # This file
├── DEVICE_PORTING_GUIDE.md             # Guide for importing devices
├── BUILD_GUIDE.md                      # Build instructions
├── EXECUTION_BACKENDS.md               # Execution backend documentation
├── GUEST_ADDITIONS_PORTING.md          # Guest Additions porting guide
├── UI_FRONTEND_GUIDE.md                # UI frontend development guide
├── KNOWN_INCOMPATIBILITIES.md          # Known device/chipset/BIOS incompatibilities
├── COMPONENT_LICENSES.md               # License information for all components
├── LICENSES/                           # Directory containing license text files
└── TODO.md                             # Project TODO list
```

## Implementation Plan

The project is divided into 9 priorities:

1. **Priority 1**: Expanded Device Layer
2. **Priority 2**: Full Software Emulation Path
3. **Priority 3**: Guest Additions Expansion
4. **Priority 4**: Remove VM Execution Restrictions
5. **Priority 5**: Platform-Agnostic UI
6. **Priority 6**: Firmware & BIOS Options
7. **Priority 7**: Build System Unification
8. **Priority 8**: License & Compliance Tracking
9. **Priority 9**: Documentation & Developer Onboarding

Each priority contains specific goals and tasks that must be completed in order.

## Guiding Principles

The project adheres to the following guiding principles:

- **Full modularity**: Every subsystem should be replaceable or disableable independently.
- **Total user sovereignty**: No silent overrides, no forced defaults, no hidden compatibility guards.
- **Platform agnosticism**: The core must never assume a specific host OS, host architecture, or privilege level.
- **Additive, never destructive**: All upstream VirtualBox functionality is preserved as a baseline.

## Next Steps

1. Begin Priority 1: Expanded Device Layer - Design Device Abstraction Layer (DAL)
2. Implement device registry/catalog system
3. Define standard bus attachment points and I/O APIs
4. Create device capability flags system
