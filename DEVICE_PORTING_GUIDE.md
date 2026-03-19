# LibreVMM — Device Porting Guide

This guide provides step-by-step instructions for importing a device from QEMU, 86Box, or Bochs into LibreVMM using the Device Abstraction Layer (DAL).

## Overview

The Device Abstraction Layer (DAL) is a unified interface that allows devices from various sources to be integrated into VirtualBox. It provides:

- Standard device lifecycle callbacks
- Bus attachment mechanisms
- I/O port, MMIO, IRQ, and DMA registration
- Device capability flags
- Runtime device discovery

## Pre-Requisites

1. Familiarity with C/C++ programming
2. Understanding of VirtualBox's PDM (Pluggable Device Manager) API
3. Knowledge of the device source code in its original project
4. Cloned repositories of the target project (QEMU, 86Box, or Bochs)

## Step 1: Create Device Directory Structure

Create a directory for your device in the appropriate location:

```
Projects/VirtualBox-Community-Edition/Devices/
├── [Source]/               # QEMU, 86Box, or Bochs
│   ├── [DeviceType]/       # e.g., audio, video, storage
│   │   ├── [DeviceName]/   # e.g., SB16, VGA, IDE
│   │   │   ├── [DeviceName].h     # Device interface definition
│   │   │   ├── [DeviceName].cpp   # Device implementation
│   │   │   └── Makefile.kmk       # Build configuration
│   │   └── ...
│   └── ...
└── ...
```

## Step 2: Define Device Interface

Create a device interface header file that defines the device's state structure and lifecycle callbacks:

```cpp
// [DeviceName].h
#ifndef VBOXEXT_DEV_[DEVICENAME]_H
#define VBOXEXT_DEV_[DEVICENAME]_H

#include <VBox/vmm/pdmdev.h>
#include <VBox/vmm/pdmifs.h>

// Device state structure
typedef struct SAMPLESTATE
{
    // Device instance data
    uint32_t uVersion;
    // ... additional device-specific state
} SAMPLESTATE;

typedef SAMPLESTATE *PSAMPLESTATE;

// Device registration structure
extern const PDMDEVREG g_Device[Devicename];

#endif // VBOXEXT_DEV_[DEVICENAME]_H
```

## Step 3: Implement Device Callbacks

Create the device implementation file that implements the lifecycle callbacks:

```cpp
// [DeviceName].cpp
#include "[DeviceName].h"
#include <VBox/vmm/pdmdev.h>
#include <VBox/log.h>
#include <iprt/errcore.h>

static DECLCALLBACK(int) devSampleConstruct(PPDMDEVINS pDevIns, int iInstance, PCFGMNODE pCfg)
{
    // Check device versions
    PDMDEV_CHECK_VERSIONS_RETURN(pDevIns);

    // Initialize device state
    PSAMPLESTATE pThis = PDMDEVINS_2_DATA(pDevIns, PSAMPLESTATE);
    pThis->uVersion = 1;

    // Validate and read configuration
    // ...

    return VINF_SUCCESS;
}

static DECLCALLBACK(int) devSampleDestruct(PPDMDEVINS pDevIns)
{
    PDMDEV_CHECK_VERSIONS_RETURN_QUIET(pDevIns);
    return VINF_SUCCESS;
}

// Additional callbacks: PowerOn, Reset, Suspend, Resume, etc.

// Device registration structure
static const PDMDEVREG g_DeviceSample =
{
    /* .u32Version = */             PDM_DEVREG_VERSION,
    /* .uReserved0 = */             0,
    /* .szName = */                 "sample",
    /* .fFlags = */                 PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE,
    /* .fClass = */                 PDM_DEVREG_CLASS_MISC,
    /* .cMaxInstances = */          1,
    /* .uSharedVersion = */         1,
    /* .cbInstanceShared = */       sizeof(SAMPLESTATE),
    /* .cbInstanceCC = */           0,
    /* .cbInstanceRC = */           0,
    /* .cMaxPciDevices = */         0,
    /* .cMaxMsixVectors = */        0,
    /* .pszDescription = */         "Sample device",
    /* ... callbacks ... */
};

// Register the device
extern "C" DECLEXPORT(int) VBoxDevicesRegister(PPDMDEVREGCB pCallbacks, uint32_t u32Version)
{
    LogFlow(("SampleDevice::VBoxDevicesRegister: u32Version=%#x pCallbacks->u32Version=%#x\n", u32Version, pCallbacks->u32Version));

    AssertLogRelMsgReturn(u32Version >= VBOX_VERSION,
                          ("VirtualBox version %#x, expected %#x or higher\n", u32Version, VBOX_VERSION),
                          VERR_VERSION_MISMATCH);
    AssertLogRelMsgReturn(pCallbacks->u32Version == PDM_DEVREG_CB_VERSION,
                          ("callback version %#x, expected %#x\n", pCallbacks->u32Version, PDM_DEVREG_CB_VERSION),
                          VERR_VERSION_MISMATCH);

    return pCallbacks->pfnRegister(pCallbacks, &g_DeviceSample);
}
```

## Step 4: Add Device to Build System

Create a Makefile.kmk file to integrate your device into the build system:

```makefile
# Makefile.kmk
ifndef VBOXEXT_DEV_[DEVICENAME]_MK
VBOXEXT_DEV_[DEVICENAME]_MK := 1

## @file
# [DeviceName] device.

VBOX_PATH_DEVICES ?= $(PATH_ROOT)/Projects/VirtualBox-Community-Edition/Devices

SUB_DEPTH = ../../../../..
include $(KBUILD_PATH)/subheader.kmk

VBOXDEVICEEXT += \
    VBoxExtDev[Devicename]

VBoxExtDev[Devicename]_TYPE := VBOXDEVICE
VBoxExtDev[Devicename]_PATH := $(VBOX_PATH_DEVICES)/[Source]/[DeviceType]/[DeviceName]
VBoxExtDev[Devicename]_SRCS := \
    [DeviceName].cpp
VBoxExtDev[Devicename]_DEFS := IN_RING3
VBoxExtDev[Devicename]_LIBS := \
    $(PATH_STAGE)/lib/VBoxRT$(VBOX_SUFF_LIB) \
    $(PATH_STAGE)/lib/VBoxVMM$(VBOX_SUFF_LIB)

include $(KBUILD_PATH)/subfooter.kmk

endif
```

## Step 5: Import Device Source Code

Copy or reference the device source code from the original project (QEMU, 86Box, or Bochs) and adapt it to use the DAL interface.

### QEMU Device Import

1. Locate the device source files in `vendor/qemu/hw/i386/`
2. Extract the device implementation from the QEMU codebase
3. Replace QEMU-specific APIs with VirtualBox's DAL interfaces
4. Adapt the device to use VirtualBox's memory and I/O systems

### 86Box Device Import

1. Locate the device source files in `vendor/86Box/src/`
2. Extract the device implementation from the 86Box codebase
3. Replace 86Box-specific APIs with VirtualBox's DAL interfaces
4. Adapt the device to use VirtualBox's memory and I/O systems
5. Use ROM images from `vendor/86Box/roms/` if needed

### Bochs Device Import

1. Locate the device source files in `vendor/Bochs/bochs/`
2. Extract the device implementation from the Bochs codebase
3. Replace Bochs-specific APIs with VirtualBox's DAL interfaces
4. Adapt the device to use VirtualBox's memory and I/O systems

### DOSBox-X Device Import

1. Locate the device source files in `vendor/DOSBox-X/src/`
2. Extract the device implementation from the 86Box codebase
3. Replace 86Box-specific APIs with VirtualBox's DAL interfaces
4. Adapt the device to use VirtualBox's memory and I/O systems

## Step 6: Implement Device Capability Flags

Add device capability flags to your device registration to allow UI filtering and runtime validation:

```cpp
// In [DeviceName].cpp
static const PDMDEVREG g_Device[Devicename] =
{
    /* ... */
    /* .fFlags = */                 PDM_DEVREG_FLAGS_DEFAULT_BITS | PDM_DEVREG_FLAGS_NEW_STYLE | DEVICE_CAP_LEGACY_ISA,
    /* ... */
};
```

Available capability flags:

- `DEVICE_CAP_3D`: Device supports 3D acceleration
- `DEVICE_CAP_LEGACY_ISA`: Device is an ISA device
- `DEVICE_CAP_USB3`: Device supports USB 3.0
- `DEVICE_CAP_MCA`: Device is an MCA device
- `DEVICE_CAP_VIRTIO`: Device is a virtio device

## Step 7: Test and Validate

1. Build LibreVMM
2. Create a test VM
3. Add your device to the VM configuration
4. Start the VM and test device functionality
5. Check for errors in the VirtualBox log
6. Run any available device-specific tests

## Step 8: Documentation

Update the project documentation with information about your device:

1. Add the device to `KNOWN_INCOMPATIBILITIES.md` if there are any known issues
2. Update `COMPONENT_LICENSES.md` with the device's license information
3. Add any device-specific documentation to the `Devices/[Source]/[DeviceType]/[DeviceName]/` directory

## Best Practices

1. **Keep it simple**: Focus on core functionality first
2. **Test early**: Test your device as soon as possible
3. **Document everything**: Write clear comments and documentation
4. **Follow VirtualBox coding style**: Use the same coding style as the rest of the VirtualBox codebase
5. **Preserve compatibility**: Ensure your device works with existing VirtualBox functionality

## Next Steps

After completing this guide, you should have a working device in LibreVMM. You can then:

1. Optimize your device's performance
2. Add additional features
3. Fix any bugs or compatibility issues
4. Contribute your device to the project