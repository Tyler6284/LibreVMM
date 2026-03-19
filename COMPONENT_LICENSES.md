# LibreVMM — Component Licenses

This document tracks the license information for all components used in LibreVMM.

## Core Components

### VirtualBox
- **License**: GPL-3.0-only or CDDL-1.0
- **Notes**: Dual-licensed; CDDL-licensed components are separated from GPL code
- **Source**: VirtualBox repository

### QEMU
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Different components may have different licenses
- **Source**: QEMU repository

### 86Box
- **License**: GPL-2.0
- **Notes**: Some device emulations may have separate attributions
- **Source**: 86Box repository

### Bochs
- **License**: LGPL-2.1
- **Notes**: Bundles SeaBIOS 1.13.0 binaries
- **Source**: Bochs repository

### SeaBIOS
- **License**: LGPL-3.0+ or BSD-3-Clause
- **Notes**: Dual-licensed
- **Source**: SeaBIOS repository

### OpenBIOS
- **License**: GPL-2.0
- **Notes**: 
- **Source**: OpenBIOS repository

### SDL2
- **License**: zlib
- **Notes**: Used for audio/video backends
- **Source**: SDL repository

## Device Drivers

### KVM Guest Drivers for Windows
- **License**: GPL-2.0 or LGPL-2.1
- **Notes**: Pre-built virtio driver binaries
- **Source**: KVM guest drivers for Windows repository

## ROM Images

### 86Box ROM Collection
- **License**: Various (check individual files)
- **Notes**: Contains controller BIOSes for 86Box device emulation
- **Source**: 86Box ROM repository

### Bochs VGABIOS
- **License**: LGPL-2.1 or custom
- **Notes**: VGA BIOS ROMs
- **Source**: Bochs repository

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