# LibreVMM Vendor Directory

> **This directory contains upstream dependencies and is NOT committed to the Git repository.**
> 
> See [`.gitignore`](../.gitignore) - the `/vendor/` directory is explicitly excluded.

## Overview

This directory houses all upstream source code repositories that LibreVMM integrates or imports from. These are build inputs, not the project itself. Each subdirectory contains a specific component as documented in the [Repository Map](../TODO.md#repository-map).

## Directory Structure

```
vendor/
├── virtualbox/                  # Core foundation (kBuild)
├── qemu/                       # TCG engine + device import (Meson)
├── 86Box/                      # Legacy device import (CMake + vcpkg)
│   └── roms/                   # ROM image library
├── Bochs/bochs/               # BIOS + device import (Autoconf)
├── seabios/                    # Firmware option (Kconfig/Makefile)
├── openbios/                   # Firmware option (Makefile/Kconfig)
├── kvm-guest-drivers-windows/ # virtio Windows guest drivers
├── SDL/                        # SDL2 backend (CMake)
├── dosbox-x/                   # DOS cycle scaling + legacy hardware
├── jdk/                        # Java runtime
├── VirtualBox-5.2.44/         # UI design reference (kBuild)
├── VirtualBoxSDK-5.2.44-139111/ # VirtualBox 5.2 SDK
└── VirtualBoxSDK-7.2.6-172322/  # VirtualBox 7.2 SDK
```

## Setting Up Vendors

Run the setup script to clone all required repositories:

```bash
./scripts/setup-vendors.sh
```

For shallow clones (faster, less disk space):
```bash
./scripts/setup-vendors.sh --depth 1
```

To preview without making changes:
```bash
./scripts/setup-vendors.sh --dry-run
```

## Manual Setup Requirements

Some components require manual download due to licensing or binary distribution:

### VirtualBox 5.2.44 Source
1. Download: https://download.virtualbox.org/virtualbox/5.2.44/VirtualBox-5.2.44.tar.bz2
2. Extract to: `vendor/VirtualBox-5.2.44/`

### VirtualBox SDKs
- **5.2.44**: https://download.virtualbox.org/virtualbox/5.2.44/VirtualBoxSDK-5.2.44-139111.zip → `vendor/VirtualBoxSDK-5.2.44-139111/`
- **7.2.6**: https://download.virtualbox.org/virtualbox/7.2.6/VirtualBoxSDK-7.2.6-172322.zip → `vendor/VirtualBoxSDK-7.2.6-172322/`

### JDK 
Download from https://github.com/openjdk/jdk
- Extract to: `vendor/jdk/`

## Component Details

| Directory | Role | Build System | License |
|-----------|------|--------------|---------|
| [`virtualbox/`](virtualbox/) | Core foundation | kBuild | GPLv3 + CDDL |
| [`qemu/`](qemu/) | TCG engine + devices | Meson | GPLv2 |
| [`86Box/`](86Box/) | Legacy devices | CMake | GPLv2 |
| [`Bochs/bochs/`](Bochs/bochs/) | BIOS + devices | Autoconf | LGPLv2.1 |
| [`seabios/`](seabios/) | Firmware | Kconfig/Makefile | LGPLv3 |
| [`openbios/`](openbios/) | Firmware | Makefile/Kconfig | GPLv2 |
| [`kvm-guest-drivers-windows/`](kvm-guest-drivers-windows/) | virtio drivers | Windows DDK | GPLv2 |
| [`86Box/roms/`](86Box/roms/) | ROM images | N/A | Various |
| [`SDL/`](SDL/) | SDL2 backend | CMake | zlib |
| [`dosbox-x/`](dosbox-x/) | DOS emulation | Autotools/CMake | GPLv2 |
| [`jdk/`](jdk/) | Java runtime | OpenJDK build | GPLv2 + Classpath Exception |

## License Compliance

Each vendor component has its own license. See the [LICENSES](../LICENSES/) directory for aggregated license texts, and refer to each vendor's `COPYING`, `LICENSE`, or `COPYING.*` file for details.

Key license considerations:
- **VirtualBox**: Dual licensed under GPLv3 and CDDL. Components are separated accordingly.
- **QEMU**: GPLv2, with some components under COPYING.LIB (LGPLv2.1)
- **SeaBIOS**: LGPLv3 + BSD
- **86Box**: GPLv2
- **Bochs**: LGPLv2.1

## Build Integration

These vendor directories are integrated into the LibreVMM build system as documented in [BUILD_GUIDE.md](../BUILD_GUIDE.md). Each vendor component is built as a library or module, not as a standalone executable.

## Notes

- The `/vendor/` directory is excluded from version control per the TODO.md specification
- **DO NOT COMMIT VENDOR CODE TO LIBREVMM REPOSITORY!!!**
- Always respect upstream licenses when modifying or redistributing vendor components
- Run `scripts/setup-vendors.sh` after cloning the repository to populate vendor directories
