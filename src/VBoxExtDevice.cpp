/**
 * @file VBoxExtDevice.cpp
 * @brief Device Abstraction Layer - Base Device Implementation
 *
 * This file provides the base implementation of the IVBoxExtDevice interface,
 * including bus attachment helpers and I/O registration helpers.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "../include/VBoxExtDevice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @name VBoxExtDeviceBase - Base Device Implementation
 * @brief Default implementation of IVBoxExtDevice methods
 *
 * This structure provides default implementations that derived
 * devices can inherit or override.
 */
typedef struct VBoxExtDeviceBase
{
    /**< Interface vtable */
    IVBoxExtDevice       vtbl;

    /**< Device name */
    char                *pszName;

    /**< Device type */
    VBoxExtDeviceType    enmType;

    /**< Capability flags */
    uint32_t             fCaps;

    /**< Bus type */
    VBoxExtBusType       enmBusType;

    /**< Current state */
    VBoxExtDeviceState   enmState;

    /**< Device configuration */
    VBoxExtDeviceConfig  Config;

    /**< I/O callbacks (set during initialization) */
    VBoxExtIoCallbacks   IoCallbacks;
    void                *pvIoContext;

    /**< PCI callbacks (set during initialization) */
    VBoxExtPciCallbacks  PciCallbacks;
    void                *pvPciContext;

} VBoxExtDeviceBase;

/**
 * @name Default Device Name
 */
static const char *g_pszDefaultDeviceName = "VBoxExtDeviceBase";

/**
 * @brief Create a new base device instance
 * @param ppDevice   Output: created device instance
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceBaseCreate(IVBoxExtDevice **ppDevice)
{
    if (!ppDevice)
        return VBOXERR_INVALID_PARAM;

    VBoxExtDeviceBase *pDevice = (VBoxExtDeviceBase *)calloc(1, sizeof(VBoxExtDeviceBase));
    if (!pDevice)
        return VBOXERR_NO_MEMORY;

    pDevice->pszName = strdup(g_pszDefaultDeviceName);
    pDevice->enmType = VBOX_EXT_DEV_TYPE_NONE;
    pDevice->fCaps = DEVICE_CAP_NONE;
    pDevice->enmBusType = VBOX_EXT_BUS_NONE;
    pDevice->enmState = VBOX_EXT_DEV_STATE_NONE;

    *ppDevice = (IVBoxExtDevice *)pDevice;
    return VBOX_SUCCESS;
}

/**
 * @brief Destroy a base device instance
 * @param pDevice   Device to destroy
 * @return VBOX_SUCCESS on success
 */
int VBoxExtDeviceBaseDestroy(VBoxExtDeviceBase *pDevice)
{
    if (!pDevice)
        return VBOXERR_INVALID_PARAM;

    if (pDevice->pszName)
        free(pDevice->pszName);

    free(pDevice);
    return VBOX_SUCCESS;
}

/**
 * @name IVBoxExtDevice Virtual Function Implementations
 * @brief Default implementations that derived devices can override
 */

static const char *vbox_ext_device_get_name(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;
    return pBase->pszName ? pBase->pszName : g_pszDefaultDeviceName;
}

static VBoxExtDeviceType vbox_ext_device_get_type(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;
    return pBase->enmType;
}

static uint32_t vbox_ext_device_get_capabilities(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;
    return pBase->fCaps;
}

static VBoxExtBusType vbox_ext_device_get_bus_type(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;
    return pBase->enmBusType;
}

static VBoxExtDeviceState vbox_ext_device_get_state(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;
    return pBase->enmState;
}

static int vbox_ext_device_construct(IVBoxExtDevice *pDevice, const VBoxExtDeviceConfig *pConfig)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pConfig || !pConfig->pcszDeviceName)
        return VBOXERR_INVALID_PARAM;

    /* Copy configuration */
    memcpy(&pBase->Config, pConfig, sizeof(VBoxExtDeviceConfig));

    /* Update device name if different */
    if (pBase->pszName)
        free(pBase->pszName);
    pBase->pszName = strdup(pConfig->pcszDeviceName);

    pBase->enmState = VBOX_EXT_DEV_STATE_CONSTRUCTED;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_destruct(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    pBase->enmState = VBOX_EXT_DEV_STATE_DESTRUCTING;

    /* Unregister all I/O resources */
    VBoxExtDeviceBaseUnregisterAllResources(pDevice);

    pBase->enmState = VBOX_EXT_DEV_STATE_NONE;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_initialize(IVBoxExtDevice *pDevice,
                                       const VBoxExtIoCallbacks *pIoCallbacks,
                                       void *pvIoContext)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pIoCallbacks)
        return VBOXERR_INVALID_PARAM;

    /* Store callbacks */
    memcpy(&pBase->IoCallbacks, pIoCallbacks, sizeof(VBoxExtIoCallbacks));
    pBase->pvIoContext = pvIoContext;

    pBase->enmState = VBOX_EXT_DEV_STATE_INITIALIZED;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_reset(IVBoxExtDevice *pDevice, bool fWarm)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    pBase->enmState = VBOX_EXT_DEV_STATE_RESETTING;

    /* Default implementation: just change state */
    pBase->enmState = VBOX_EXT_DEV_STATE_READY;

    return VBOX_SUCCESS;
}

static int vbox_ext_device_power_on(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (pBase->enmState != VBOX_EXT_DEV_STATE_READY &&
        pBase->enmState != VBOX_EXT_DEV_STATE_INITIALIZED)
        return VBOXERR_INVALID_STATE;

    pBase->enmState = VBOX_EXT_DEV_STATE_RUNNING;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_power_off(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (pBase->enmState != VBOX_EXT_DEV_STATE_RUNNING &&
        pBase->enmState != VBOX_EXT_DEV_STATE_PAUSED)
        return VBOXERR_INVALID_STATE;

    pBase->enmState = VBOX_EXT_DEV_STATE_READY;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_attach(IVBoxExtDevice *pDevice, VBoxExtBusType enmBus,
                                   uint8_t uBus, uint8_t uDev, uint8_t uFun)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    pBase->Config.enmBus = enmBus;
    pBase->Config.u8Bus = uBus;
    pBase->Config.u8Dev = uDev;
    pBase->Config.u8Fun = uFun;

    return VBOX_SUCCESS;
}

static int vbox_ext_device_detach(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    pBase->Config.enmBus = VBOX_EXT_BUS_NONE;
    pBase->Config.u8Bus = 0;
    pBase->Config.u8Dev = 0;
    pBase->Config.u8Fun = 0;

    return VBOX_SUCCESS;
}

static int vbox_ext_device_save_state(IVBoxExtDevice *pDevice, void *pvBuffer,
                                       size_t cbBuffer, size_t *pcbSaved)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pvBuffer || !pcbSaved)
        return VBOXERR_INVALID_PARAM;

    /* Default: just save basic state */
    if (cbBuffer < sizeof(VBoxExtDeviceState))
        return VBOXERR_NO_RESOURCES;

    *(VBoxExtDeviceState *)pvBuffer = pBase->enmState;
    *pcbSaved = sizeof(VBoxExtDeviceState);

    return VBOX_SUCCESS;
}

static int vbox_ext_device_load_state(IVBoxExtDevice *pDevice, const void *pvBuffer,
                                       size_t cbBuffer)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pvBuffer || cbBuffer < sizeof(VBoxExtDeviceState))
        return VBOXERR_INVALID_PARAM;

    /* Default: restore basic state */
    pBase->enmState = *(const VBoxExtDeviceState *)pvBuffer;

    return VBOX_SUCCESS;
}

static int vbox_ext_device_pause(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (pBase->enmState != VBOX_EXT_DEV_STATE_RUNNING)
        return VBOXERR_INVALID_STATE;

    pBase->enmState = VBOX_EXT_DEV_STATE_PAUSED;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_resume(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (pBase->enmState != VBOX_EXT_DEV_STATE_PAUSED)
        return VBOXERR_INVALID_STATE;

    pBase->enmState = VBOX_EXT_DEV_STATE_RUNNING;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_set_pci_callbacks(IVBoxExtDevice *pDevice,
                                               const VBoxExtPciCallbacks *pCallbacks,
                                               void *pvContext)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (pCallbacks)
        memcpy(&pBase->PciCallbacks, pCallbacks, sizeof(VBoxExtPciCallbacks));

    pBase->pvPciContext = pvContext;
    return VBOX_SUCCESS;
}

static int vbox_ext_device_query_interface(IVBoxExtDevice *pDevice, const char *pszIfaceId,
                                            void **ppvIface)
{
    /* Default: no additional interfaces */
    if (ppvIface)
        *ppvIface = NULL;

    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_ioport_read(IVBoxExtDevice *pDevice, uint16_t uPort, uint32_t *pu32Value)
{
    /* Default: not implemented */
    if (pu32Value)
        *pu32Value = 0xFFFFFFFF;

    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_ioport_write(IVBoxExtDevice *pDevice, uint16_t uPort, uint32_t u32Value)
{
    /* Default: not implemented */
    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_mmio_read(IVBoxExtDevice *pDevice, uint64_t uAddress,
                                      uint32_t cbRead, uint64_t *pu64Value)
{
    /* Default: not implemented */
    if (pu64Value)
        *pu64Value = 0xFFFFFFFFFFFFFFFFULL;

    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_mmio_write(IVBoxExtDevice *pDevice, uint64_t uAddress,
                                       uint32_t cbWrite, uint64_t u64Value)
{
    /* Default: not implemented */
    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_irq_handler(IVBoxExtDevice *pDevice, uint8_t u8Irq)
{
    /* Default: not implemented */
    return VBOXERR_NOT_SUPPORTED;
}

static int vbox_ext_device_dma_handler(IVBoxExtDevice *pDevice, uint8_t u8Channel)
{
    /* Default: not implemented */
    return VBOXERR_NOT_SUPPORTED;
}

/**
 * @name Default VTable Initialization
 * @brief Initialize a device with the default vtable
 */
void VBoxExtDeviceBaseInitVtbl(VBoxExtDeviceBase *pDevice)
{
    pDevice->vtbl.pfnGetName = vbox_ext_device_get_name;
    pDevice->vtbl.pfnGetType = vbox_ext_device_get_type;
    pDevice->vtbl.pfnGetCapabilities = vbox_ext_device_get_capabilities;
    pDevice->vtbl.pfnGetBusType = vbox_ext_device_get_bus_type;
    pDevice->vtbl.pfnGetState = vbox_ext_device_get_state;
    pDevice->vtbl.pfnConstruct = vbox_ext_device_construct;
    pDevice->vtbl.pfnDestruct = vbox_ext_device_destruct;
    pDevice->vtbl.pfnInitialize = vbox_ext_device_initialize;
    pDevice->vtbl.pfnReset = vbox_ext_device_reset;
    pDevice->vtbl.pfnPowerOn = vbox_ext_device_power_on;
    pDevice->vtbl.pfnPowerOff = vbox_ext_device_power_off;
    pDevice->vtbl.pfnAttach = vbox_ext_device_attach;
    pDevice->vtbl.pfnDetach = vbox_ext_device_detach;
    pDevice->vtbl.pfnSaveState = vbox_ext_device_save_state;
    pDevice->vtbl.pfnLoadState = vbox_ext_device_load_state;
    pDevice->vtbl.pfnPause = vbox_ext_device_pause;
    pDevice->vtbl.pfnResume = vbox_ext_device_resume;
    pDevice->vtbl.pfnSetPciCallbacks = vbox_ext_device_set_pci_callbacks;
    pDevice->vtbl.pfnQueryInterface = vbox_ext_device_query_interface;
    pDevice->vtbl.pfnIoPortRead = vbox_ext_device_ioport_read;
    pDevice->vtbl.pfnIoPortWrite = vbox_ext_device_ioport_write;
    pDevice->vtbl.pfnMmioRead = vbox_ext_device_mmio_read;
    pDevice->vtbl.pfnMmioWrite = vbox_ext_device_mmio_write;
    pDevice->vtbl.pfnIrqHandler = vbox_ext_device_irq_handler;
    pDevice->vtbl.pfnDmaHandler = vbox_ext_device_dma_handler;
}

/**
 * @name Bus Attachment Helpers
 * @brief Helper functions for attaching devices to buses
 */

/**
 * @brief Attach device to ISA bus
 * @param pDevice      Device instance
 * @param u16IoBase    I/O port base address
 * @param u8Irq        IRQ line
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceAttachIsa(IVBoxExtDevice *pDevice, uint16_t u16IoBase, uint8_t u8Irq)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    /* Register port I/O */
    if (pBase->IoCallbacks.pfnRegisterIoPort && u16IoBase > 0)
    {
        int rc = pBase->IoCallbacks.pfnRegisterIoPort(
            pBase->pvIoContext,
            pBase->pszName,
            u16IoBase,
            8,  /* Standard ISA device uses 8 ports */
            NULL
        );
        if (rc != VBOX_SUCCESS)
            return rc;
    }

    /* Register IRQ */
    if (pBase->IoCallbacks.pfnRegisterIrq && u8Irq > 0)
    {
        int rc = pBase->IoCallbacks.pfnRegisterIrq(
            pBase->pvIoContext,
            u8Irq,
            false,  /* ISA edge-triggered by default */
            NULL
        );
        if (rc != VBOX_SUCCESS)
            return rc;
    }

    return pDevice->pfnAttach(pDevice, VBOX_EXT_BUS_ISA, 0, 0, 0);
}

/**
 * @brief Attach device to PCI bus
 * @param pDevice      Device instance
 * @param u8Bus        Bus number
 * @param u8Dev        Device number
 * @param u8Fun        Function number
 * @param u64MmioBase  MMIO base address (0 = none)
 * @param cbMmioSize   MMIO size
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceAttachPci(IVBoxExtDevice *pDevice, uint8_t u8Bus, uint8_t u8Dev,
                            uint8_t u8Fun, uint64_t u64MmioBase, uint32_t cbMmioSize)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    /* Register MMIO region if specified */
    if (pBase->IoCallbacks.pfnRegisterMmio && u64MmioBase > 0 && cbMmioSize > 0)
    {
        int rc = pBase->IoCallbacks.pfnRegisterMmio(
            pBase->pvIoContext,
            pBase->pszName,
            u64MmioBase,
            cbMmioSize,
            0x3,  /* Read/write */
            NULL
        );
        if (rc != VBOX_SUCCESS)
            return rc;
    }

    return pDevice->pfnAttach(pDevice, VBOX_EXT_BUS_PCI, u8Bus, u8Dev, u8Fun);
}

/**
 * @brief Attach device to PCIe bus
 * @param pDevice      Device instance
 * @param u8Bus        Bus number
 * @param u8Dev        Device number
 * @param u8Fun        Function number
 * @param u64MmioBase  MMIO base address
 * @param cbMmioSize   MMIO size
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceAttachPcie(IVBoxExtDevice *pDevice, uint8_t u8Bus, uint8_t u8Dev,
                             uint8_t u8Fun, uint64_t u64MmioBase, uint32_t cbMmioSize)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    /* PCIe requires MMIO, register it */
    if (pBase->IoCallbacks.pfnRegisterMmio && u64MmioBase > 0 && cbMmioSize > 0)
    {
        int rc = pBase->IoCallbacks.pfnRegisterMmio(
            pBase->pvIoContext,
            pBase->pszName,
            u64MmioBase,
            cbMmioSize,
            0x3,  /* Read/write */
            NULL
        );
        if (rc != VBOX_SUCCESS)
            return rc;
    }

    return pDevice->pfnAttach(pDevice, VBOX_EXT_BUS_PCIE, u8Bus, u8Dev, u8Fun);
}

/**
 * @brief Attach device to USB bus
 * @param pDevice      Device instance
 * @param u8Address    USB address
 * @param u8HubPort    Hub port number
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceAttachUsb(IVBoxExtDevice *pDevice, uint8_t u8Address, uint8_t u8HubPort)
{
    return pDevice->pfnAttach(pDevice, VBOX_EXT_BUS_USB, u8Address, u8HubPort, 0);
}

/**
 * @name I/O Registration Helpers
 * @brief Helper functions for registering I/O resources
 */

/**
 * @brief Register port I/O range
 * @param pDevice      Device instance
 * @param pszName      Resource name
 * @param uPortStart   Starting port
 * @param cPorts       Number of ports
 * @param pvCallback   Callback data
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceRegisterIoPort(IVBoxExtDevice *pDevice, const char *pszName,
                                 uint16_t uPortStart, uint16_t cPorts, void *pvCallback)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnRegisterIoPort)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnRegisterIoPort(
        pBase->pvIoContext,
        pszName ? pszName : pBase->pszName,
        uPortStart,
        cPorts,
        pvCallback
    );
}

/**
 * @brief Unregister port I/O range
 * @param pDevice      Device instance
 * @param uPortStart   Starting port
 * @param cPorts       Number of ports
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceUnregisterIoPort(IVBoxExtDevice *pDevice, uint16_t uPortStart, uint16_t cPorts)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnUnregisterIoPort)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnUnregisterIoPort(pBase->pvIoContext, uPortStart, cPorts);
}

/**
 * @brief Register MMIO region
 * @param pDevice      Device instance
 * @param pszName      Resource name
 * @param uAddress     Physical address
 * @param cbSize       Size in bytes
 * @param fFlags       Flags (read/write/execute)
 * @param pvCallback   Callback data
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceRegisterMmio(IVBoxExtDevice *pDevice, const char *pszName,
                               uint64_t uAddress, uint32_t cbSize,
                               uint32_t fFlags, void *pvCallback)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnRegisterMmio)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnRegisterMmio(
        pBase->pvIoContext,
        pszName ? pszName : pBase->pszName,
        uAddress,
        cbSize,
        fFlags,
        pvCallback
    );
}

/**
 * @brief Unregister MMIO region
 * @param pDevice      Device instance
 * @param uAddress     Physical address
 * @param cbSize       Size in bytes
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceUnregisterMmio(IVBoxExtDevice *pDevice, uint64_t uAddress, uint32_t cbSize)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnUnregisterMmio)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnUnregisterMmio(pBase->pvIoContext, uAddress, cbSize);
}

/**
 * @brief Register IRQ handler
 * @param pDevice      Device instance
 * @param u8Irq        IRQ line
 * @param fLevel       Level-triggered if true
 * @param pvCallback   Callback data
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceRegisterIrq(IVBoxExtDevice *pDevice, uint8_t u8Irq,
                              bool fLevel, void *pvCallback)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnRegisterIrq)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnRegisterIrq(pBase->pvIoContext, u8Irq, fLevel, pvCallback);
}

/**
 * @brief Unregister IRQ handler
 * @param pDevice      Device instance
 * @param u8Irq        IRQ line
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceUnregisterIrq(IVBoxExtDevice *pDevice, uint8_t u8Irq)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnUnregisterIrq)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnUnregisterIrq(pBase->pvIoContext, u8Irq);
}

/**
 * @brief Register DMA channel
 * @param pDevice      Device instance
 * @param u8Channel    DMA channel
 * @param pvCallback   Callback data
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceRegisterDma(IVBoxExtDevice *pDevice, uint8_t u8Channel, void *pvCallback)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnRegisterDma)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnRegisterDma(pBase->pvIoContext, u8Channel, pvCallback);
}

/**
 * @brief Unregister DMA channel
 * @param pDevice      Device instance
 * @param u8Channel    DMA channel
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDeviceUnregisterDma(IVBoxExtDevice *pDevice, uint8_t u8Channel)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    if (!pBase->IoCallbacks.pfnUnregisterDma)
        return VBOXERR_NOT_SUPPORTED;

    return pBase->IoCallbacks.pfnUnregisterDma(pBase->pvIoContext, u8Channel);
}

/**
 * @brief Unregister all I/O resources
 * @param pDevice      Device instance
 * @return VBOX_SUCCESS on success
 *
 * This is called during device destruction to clean up
 * all registered I/O resources.
 */
int VBoxExtDeviceBaseUnregisterAllResources(IVBoxExtDevice *pDevice)
{
    VBoxExtDeviceBase *pBase = (VBoxExtDeviceBase *)pDevice;

    /* Unregister all callbacks by clearing them */
    memset(&pBase->IoCallbacks, 0, sizeof(pBase->IoCallbacks));
    pBase->pvIoContext = NULL;

    return VBOX_SUCCESS;
}

/**
 * @name String Conversion Functions
 */

const char *VBoxExtBusTypeToString(VBoxExtBusType enmBus)
{
    static const char *apszBusNames[] = {
        "None",
        "ISA",
        "PCI",
        "PCIe",
        "LPC",
        "USB",
        "AGP",
        "MCA",
        "I2C",
        "SPI",
        "Virtual"
    };

    if (enmBus >= VBOX_EXT_BUS_NONE && enmBus < VBOX_EXT_BUS_MAX)
        return apszBusNames[enmBus];

    return "Unknown";
}

VBoxExtBusType VBoxExtBusTypeFromString(const char *pszBus)
{
    if (!pszBus)
        return VBOX_EXT_BUS_NONE;

    for (int i = 0; i < VBOX_EXT_BUS_MAX; i++)
    {
        VBoxExtBusType enmBus = (VBoxExtBusType)i;
        if (strcmp(pszBus, VBoxExtBusTypeToString(enmBus)) == 0)
            return enmBus;
    }

    return VBOX_EXT_BUS_NONE;
}

const char *VBoxExtDeviceTypeToString(VBoxExtDeviceType enmType)
{
    static const char *apszTypeNames[] = {
        "None",
        "Chipset",
        "CPU",
        "MemoryController",
        "StorageController",
        "NetworkController",
        "AudioController",
        "VideoController",
        "InputController",
        "USBController",
        "SerialController",
        "ParallelController",
        "ISADevice",
        "PCIDevice",
        "VirtioDevice",
        "Unknown"
    };

    if (enmType >= VBOX_EXT_DEV_TYPE_NONE && enmType < VBOX_EXT_DEV_TYPE_UNKNOWN)
        return apszTypeNames[enmType];

    return "Unknown";
}

VBoxExtDeviceType VBoxExtDeviceTypeFromString(const char *pszType)
{
    if (!pszType)
        return VBOX_EXT_DEV_TYPE_NONE;

    for (int i = 0; i < VBOX_EXT_DEV_TYPE_UNKNOWN; i++)
    {
        VBoxExtDeviceType enmType = (VBoxExtDeviceType)i;
        if (strcmp(pszType, VBoxExtDeviceTypeToString(enmType)) == 0)
            return enmType;
    }

    return VBOX_EXT_DEV_TYPE_NONE;
}

const char *VBoxExtCapabilityToString(uint32_t fCaps)
{
    static char szCaps[512] = {0};
    szCaps[0] = '\0';

    if (fCaps & DEVICE_CAP_3D)           strcat(szCaps, "3D ");
    if (fCaps & DEVICE_CAP_LEGACY_ISA)   strcat(szCaps, "LEGACY_ISA ");
    if (fCaps & DEVICE_CAP_USB3)         strcat(szCaps, "USB3 ");
    if (fCaps & DEVICE_CAP_MCA)          strcat(szCaps, "MCA ");
    if (fCaps & DEVICE_CAP_VIRTIO)       strcat(szCaps, "VIRTIO ");
    if (fCaps & DEVICE_CAP_PCI_EXPRESS)  strcat(szCaps, "PCI_EXPRESS ");
    if (fCaps & DEVICE_CAP_ACPI)         strcat(szCaps, "ACPI ");
    if (fCaps & DEVICE_CAP_MULTIFUNCTION) strcat(szCaps, "MULTIFUNCTION ");
    if (fCaps & DEVICE_CAP_HOTPLUG)      strcat(szCaps, "HOTPLUG ");
    if (fCaps & DEVICE_CAP_MSIX)         strcat(szCaps, "MSIX ");
    if (fCaps & DEVICE_CAP_DMA)          strcat(szCaps, "DMA ");
    if (fCaps & DEVICE_CAP_INTERRUPT)    strcat(szCaps, "INTERRUPT ");
    if (fCaps & DEVICE_CAP_MMIO)         strcat(szCaps, "MMIO ");
    if (fCaps & DEVICE_CAP_PORTIO)       strcat(szCaps, "PORTIO ");
    if (fCaps & DEVICE_CAP_SAVESTATE)    strcat(szCaps, "SAVESTATE ");
    if (fCaps & DEVICE_CAP_PAUSE)        strcat(szCaps, "PAUSE ");
    if (fCaps & DEVICE_CAP_RESET)        strcat(szCaps, "RESET ");
    if (fCaps & DEVICE_CAP_NETWORK)      strcat(szCaps, "NETWORK ");
    if (fCaps & DEVICE_CAP_STORAGE)      strcat(szCaps, "STORAGE ");
    if (fCaps & DEVICE_CAP_AUDIO)        strcat(szCaps, "AUDIO ");
    if (fCaps & DEVICE_CAP_VIDEO)        strcat(szCaps, "VIDEO ");
    if (fCaps & DEVICE_CAP_INPUT)        strcat(szCaps, "INPUT ");

    return szCaps[0] ? szCaps : "NONE";
}

uint32_t VBoxExtCapabilityFromString(const char *pszCap)
{
    uint32_t fCaps = DEVICE_CAP_NONE;

    if (!pszCap)
        return DEVICE_CAP_NONE;

    if (strstr(pszCap, "3D"))            fCaps |= DEVICE_CAP_3D;
    if (strstr(pszCap, "LEGACY_ISA"))    fCaps |= DEVICE_CAP_LEGACY_ISA;
    if (strstr(pszCap, "USB3"))          fCaps |= DEVICE_CAP_USB3;
    if (strstr(pszCap, "MCA"))           fCaps |= DEVICE_CAP_MCA;
    if (strstr(pszCap, "VIRTIO"))        fCaps |= DEVICE_CAP_VIRTIO;
    if (strstr(pszCap, "PCI_EXPRESS"))   fCaps |= DEVICE_CAP_PCI_EXPRESS;
    if (strstr(pszCap, "ACPI"))          fCaps |= DEVICE_CAP_ACPI;
    if (strstr(pszCap, "MULTIFUNCTION")) fCaps |= DEVICE_CAP_MULTIFUNCTION;
    if (strstr(pszCap, "HOTPLUG"))       fCaps |= DEVICE_CAP_HOTPLUG;
    if (strstr(pszCap, "MSIX"))          fCaps |= DEVICE_CAP_MSIX;
    if (strstr(pszCap, "DMA"))           fCaps |= DEVICE_CAP_DMA;
    if (strstr(pszCap, "INTERRUPT"))     fCaps |= DEVICE_CAP_INTERRUPT;
    if (strstr(pszCap, "MMIO"))          fCaps |= DEVICE_CAP_MMIO;
    if (strstr(pszCap, "PORTIO"))        fCaps |= DEVICE_CAP_PORTIO;
    if (strstr(pszCap, "SAVESTATE"))     fCaps |= DEVICE_CAP_SAVESTATE;
    if (strstr(pszCap, "PAUSE"))         fCaps |= DEVICE_CAP_PAUSE;
    if (strstr(pszCap, "RESET"))         fCaps |= DEVICE_CAP_RESET;
    if (strstr(pszCap, "NETWORK"))       fCaps |= DEVICE_CAP_NETWORK;
    if (strstr(pszCap, "STORAGE"))       fCaps |= DEVICE_CAP_STORAGE;
    if (strstr(pszCap, "AUDIO"))         fCaps |= DEVICE_CAP_AUDIO;
    if (strstr(pszCap, "VIDEO"))         fCaps |= DEVICE_CAP_VIDEO;
    if (strstr(pszCap, "INPUT"))         fCaps |= DEVICE_CAP_INPUT;

    return fCaps;
}
