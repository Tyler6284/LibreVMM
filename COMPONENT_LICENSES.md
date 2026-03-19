# VirtualBox Extended Edition — Component Licenses

This document tracks the license information for all components used in VirtualBox Extended Edition.

## Core Components

### VirtualBox
- **Location**: `Cloned-Repos/virtualbox/`
- **License**: GPL-3.0-only or CDDL-1.0
- **Notes**: Dual-licensed; CDDL-licensed components are separated from GPL code
- **Source**: VirtualBox repository

### QEMU
- **Location**: `Cloned-Repos/qemu/`
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Different components may have different licenses
- **Source**: QEMU repository

### 86Box
- **Location**: `Cloned-Repos/86Box/`
- **License**: GPL-2.0
- **Notes**: Some device emulations may have separate attributions
- **Source**: 86Box repository

### Bochs
- **Location**: `Cloned-Repos/Bochs/bochs/`
- **License**: LGPL-2.1
- **Notes**: Bundles SeaBIOS 1.13.0 binaries
- **Source**: Bochs repository

### SeaBIOS
- **Location**: `Cloned-Repos/seabios/`
- **License**: LGPL-3.0+ or BSD-3-Clause
- **Notes**: Dual-licensed
- **Source**: SeaBIOS repository

### OpenBIOS
- **Location**: `Cloned-Repos/openbios/`
- **License**: GPL-2.0
- **Notes**: 
- **Source**: OpenBIOS repository

### FEX-Emu
- **Location**: `Cloned-Repos/FEX/`
- **License**: MIT
- **Notes**: Linux ARM64-only
- **Source**: FEX-Emu repository

### SDL2
- **Location**: `Cloned-Repos/SDL/`
- **License**: zlib
- **Notes**: Used for audio/video backends
- **Source**: SDL repository

## Device Drivers

### KVM Guest Drivers for Windows
- **Location**: `Cloned-Repos/kvm-guest-drivers-windows/`
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Pre-built virtio driver binaries
- **Source**: KVM guest drivers for Windows repository

## ROM Images

### 86Box ROM Collection
- **Location**: `Cloned-Repos/roms/`
- **License**: Various (check individual files)
- **Notes**: Contains controller BIOSes for 86Box device emulation
- **Source**: 86Box ROM repository

### Bochs VGABIOS
- **Location**: `Cloned-Repos/Bochs/bochs/bios/VGABIOS-lgpl/` and `VGABIOS-elpin/`
- **License**: LGPL-2.1 or custom
- **Notes**: VGA BIOS ROMs
- **Source**: Bochs repository

## Reference Components

These components are used for design or driver model information only, not compiled directly into the project.

### Linux Kernel
- **Location**: `Cloned-Repos/linux/`
- **License**: GPL-2.0
- **Notes**: Reference for KVM interface and Linux guest drivers
- **Source**: Linux kernel repository

### ReactOS
- **Location**: `Cloned-Repos/reactos/`
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Reference for NT 3.x/4.x driver model
- **Source**: ReactOS repository

### One-Core-API-Source
- **Location**: `Cloned-Repos/One-Core-API-Source/`
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Reference for Win9x/NT 3.x/NT 4.0 API surface
- **Source**: One-Core-API-Source repository

### ntvdmx64
- **Location**: `Cloned-Repos/ntvdmx64/`
- **License**: Various (check individual files)
- **Notes**: Reference for DOS-mode execution in restricted environments
- **Source**: ntvdmx64 repository

### libvirt
- **Location**: `Cloned-Repos/libvirt/`
- **License**: LGPL-2.1
- **Notes**: Reference for VM management API design
- **Source**: libvirt repository

### virt-manager
- **Location**: `Cloned-Repos/virt-manager/`
- **License**: GPL-2.0
- **Notes**: Reference for Linux desktop UI design
- **Source**: virt-manager repository

### dosbox-x
- **Location**: `Cloned-Repos/dosbox-x/`
- **License**: GPL-2.0
- **Notes**: Reference for DOS guest support
- **Source**: DOSBox-X repository

### Vulkan
- **Location**: `Cloned-Repos/vulkan/`
- **License**: Apache-2.0
- **Notes**: Reference for Vulkan host rendering path
- **Source**: Vulkan repository

### OpenGL-Registry
- **Location**: `Cloned-Repos/OpenGL-Registry/`
- **License**: Khronos Group Open Source License
- **Notes**: Reference for OpenGL/OpenGL ES extension registry
- **Source**: OpenGL-Registry repository

## New Project Files

All new files created by this project will follow the REUSE specification with SPDX headers:

```cpp
/* SPDX-License-Identifier: GPL-3.0-only */
```

## Compatibility Matrix

| License | Compatible With |
|---------|-----------------|
| GPL-2.0 | GPL-2.0, LGPL-2.0, LGPL-2.1 |
| GPL-3.0 | GPL-3.0 |
| LGPL-2.1 | LGPL-2.1, GPL-2.0, GPL-3.0 |
| BSD-3-Clause | Most open-source licenses |
| MIT | Most open-source licenses |
| Apache-2.0 | Most open-source licenses |
| CDDL-1.0 | CDDL-1.0 (cannot be linked with GPL code) |

## License Resolution

### VirtualBox CDDL + GPL Split
- CDDL-licensed VBox components are kept in a separate module boundary
- GPL code is never statically linked into CDDL modules

### Bochs LGPL-2.1
- Bochs components are dynamically linked to maintain LGPL compliance

### SeaBIOS LGPL-3.0+
- SeaBIOS is distributed under LGPL-3.0+ with BSD-3-Clause options

## Audit Trail

All license information is regularly audited to ensure compliance with distribution requirements.

## Next Steps

1. Create `LICENSES/` directory and populate it with license text files
2. Add SPDX headers to all new source files
3. Audit existing device emulations for third-party code
4. Confirm license compatibility for all imported components