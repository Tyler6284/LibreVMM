/**
 * @file VBoxExtDevice.h
 * @brief Device Abstraction Layer - Unified IVBoxExtDevice Interface
 *
 * This header defines the core interface that all imported devices
 * (QEMU, Bochs, 86Box, native VBox) must implement.
 * 
 * Warn user to provide 86Box roms from the 86Box ROMs repository!
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef VBOX_EXT_DEVICE_H
#define VBOX_EXT_DEVICE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name Device Capability Flags
 * @brief Flags for device capabilities used in UI filtering and runtime validation
 */
#define DEVICE_CAP_NONE           0x00000000U
#define DEVICE_CAP_3D             0x00000001U  /**< Device supports 3D acceleration */
#define DEVICE_CAP_LEGACY_ISA     0x00000002U  /**< Device is a legacy ISA device */
#define DEVICE_CAP_USB3           0x00000004U  /**< Device supports USB 3.0 */
#define DEVICE_CAP_MCA            0x00000008U  /**< Device supports MCA bus */
#define DEVICE_CAP_VIRTIO         0x00000010U  /**< Device is a virtio device */
#define DEVICE_CAP_PCI_EXPRESS    0x00000020U  /**< Device requires PCIe */
#define DEVICE_CAP_ACPI           0x00000040U  /**< Device requires ACPI */
#define DEVICE_CAP_MULTIFUNCTION  0x00000080U  /**< Device is a multi-function PCI device */
#define DEVICE_CAP_HOTPLUG        0x00000100U  /**< Device supports hot-plugging */
#define DEVICE_CAP_MSIX           0x00000200U  /**< Device supports MSI-X */
#define DEVICE_CAP_DMA            0x00000400U  /**< Device uses DMA */
#define DEVICE_CAP_INTERRUPT      0x00000800U  /**< Device uses interrupts */
#define DEVICE_CAP_MMIO           0x00001000U  /**< Device uses MMIO */
#define DEVICE_CAP_PORTIO         0x00002000U  /**< Device uses port I/O */
#define DEVICE_CAP_SAVESTATE      0x00004000U  /**< Device supports save/restore state */
#define DEVICE_CAP_PAUSE          0x00008000U  /**< Device supports pause/resume */
#define DEVICE_CAP_RESET          0x00010000U  /**< Device supports reset */
#define DEVICE_CAP_SHAREABLE      0x00020000U  /**< Device can be shared between VMs */
#define DEVICE_CAP_NETWORK        0x00040000U  /**< Device is a network device */
#define DEVICE_CAP_STORAGE        0x00080000U  /**< Device is a storage device */
#define DEVICE_CAP_AUDIO          0x00100000U  /**< Device is an audio device */
#define DEVICE_CAP_VIDEO          0x00200000U  /**< Device is a video/graphics device */
#define DEVICE_CAP_INPUT          0x00400000U  /**< Device is an input device */
#define DEVICE_CAP_ROM            0x00800000U  /**< Device has ROM */
#define DEVICE_CAP_PM             0x01000000U  /**< Device supports power management */
#define DEVICE_CAP_WATCHDOG       0x02000000U  /**< Device is a watchdog */
#define DEVICE_CAP_TPM            0x04000000U  /**< Device is a TPM */
#define DEVICE_CAP_RNG            0x08000000U  /**< Device is an RNG */

/**
 * @name Bus Types
 * @brief Standard bus attachment points
 */
typedef enum VBoxExtBusType
{
    VBOX_EXT_BUS_NONE = 0,
    VBOX_EXT_BUS_ISA,        /**< Industry Standard Architecture */
    VBOX_EXT_BUS_PCI,        /**< Peripheral Component Interconnect */
    VBOX_EXT_BUS_PCIE,       /**< PCI Express */
    VBOX_EXT_BUS_LPC,        /**< Low Pin Count (bridge) */
    VBOX_EXT_BUS_USB,        /**< Universal Serial Bus */
    VBOX_EXT_BUS_AGP,        /**< Accelerated Graphics Port (legacy) */
    VBOX_EXT_BUS_MCA,        /**< Micro Channel Architecture */
    VBOX_EXT_BUS_I2C,        /**< Inter-Integrated Circuit */
    VBOX_EXT_BUS_SPI,        /**< Serial Peripheral Interface */
    VBOX_EXT_BUS_VIRTUAL,    /**< Virtual bus (virtio) */
    VBOX_EXT_BUS_MAX
} VBoxExtBusType;

/**
 * @name Device State
 * @brief Lifecycle states for devices
 */
typedef enum VBoxExtDeviceState
{
    VBOX_EXT_DEV_STATE_NONE = 0,
    VBOX_EXT_DEV_STATE_CONSTRUCTED,
    VBOX_EXT_DEV_STATE_INITIALIZED,
    VBOX_EXT_DEV_STATE_RESETTING,
    VBOX_EXT_DEV_STATE_READY,
    VBOX_EXT_DEV_STATE_RUNNING,
    VBOX_EXT_DEV_STATE_PAUSED,
    VBOX_EXT_DEV_STATE_STOPPING,
    VBOX_EXT_DEV_STATE_DESTRUCTING,
    VBOX_EXT_DEV_STATE_MAX
} VBoxExtDeviceState;

/**
 * @name I/O Resource Types
 * @brief Types of I/O resources that can be registered
 */
typedef enum VBoxExtIoType
{
    VBOX_EXT_IO_NONE = 0,
    VBOX_EXT_IO_PORT,        /**< Port I/O (x86 in/out instructions) */
    VBOX_EXT_IO_MEMORY,      /**< Memory-mapped I/O (MMIO) */
    VBOX_EXT_IO_IRQ,         /**< Interrupt Request line */
    VBOX_EXT_IO_DMA,         /**< Direct Memory Access channel */
    VBOX_EXT_IO_MAX
} VBoxExtIoType;

/**
 * @name Device Type Classification
 * @brief High-level device type categories
 */
typedef enum VBoxExtDeviceType
{
    VBOX_EXT_DEV_TYPE_NONE = 0,
    VBOX_EXT_DEV_TYPE_CHIPSET,
    VBOX_EXT_DEV_TYPE_CPU,
    VBOX_EXT_DEV_TYPE_MEMORY_CONTROLLER,
    VBOX_EXT_DEV_TYPE_STORAGE_CONTROLLER,
    VBOX_EXT_DEV_TYPE_NETWORK_CONTROLLER,
    VBOX_EXT_DEV_TYPE_AUDIO_CONTROLLER,
    VBOX_EXT_DEV_TYPE_VIDEO_CONTROLLER,
    VBOX_EXT_DEV_TYPE_INPUT_CONTROLLER,
    VBOX_EXT_DEV_TYPE_USB_CONTROLLER,
    VBOX_EXT_DEV_TYPE_SERIAL_CONTROLLER,
    VBOX_EXT_DEV_TYPE_PARALLEL_CONTROLLER,
    VBOX_EXT_DEV_TYPE_ISA_DEVICE,
    VBOX_EXT_DEV_TYPE_PCI_DEVICE,
    VBOX_EXT_DEV_TYPE_VIRTIO_DEVICE,
    VBOX_EXT_DEV_TYPE_UNKNOWN
} VBoxExtDeviceType;

/**
 * @name I/O Resource Descriptor
 * @brief Describes an I/O resource allocated to a device
 */
typedef struct VBoxExtIoResource
{
    VBoxExtIoType  enmType;      /**< Type of I/O resource */
    uint64_t       u64Base;      /**< Base address (port number for port I/O, physical address for MMIO) */
    uint32_t       u32Size;      /**< Size of the region in bytes */
    uint8_t        u8Irq;       /**< IRQ line number (for IRQ resources) */
    uint8_t        u8Dma;        /**< DMA channel (for DMA resources) */
    bool           fReadable;    /**< Region is readable */
    bool           fWritable;    /**< Region is writable */
    bool           fExecutable;  /**< Region is executable (code) */
    const char    *pcszOwner;    /**< Owner/consumer of this resource */
} VBoxExtIoResource;

/**
 * @name Device Configuration Structure
 * @brief Configuration data passed to device during construction
 */
typedef struct VBoxExtDeviceConfig
{
    const char   *pcszDeviceName;    /**< Device name (e.g., "e1000", "VGA") */
    const char   *pcszDeviceId;     /**< Unique device instance identifier */
    const char   *pcszInstanceId;    /**< VM instance identifier */
    uint32_t      u32InstanceNum;    /**< Instance number of this device type */
    VBoxExtBusType enmBus;           /**< Bus type to attach to */
    uint8_t       u8Bus;             /**< Bus number/address */
    uint8_t       u8Dev;             /**< Device number (PCI) */
    uint8_t       u8Fun;             /**< Function number (PCI) */
    uint32_t      u32Caps;           /**< Capability flags */
    void         *pvUserData;        /**< User-defined data pointer */
    uint32_t      cbUserData;       /**< Size of user data */
} VBoxExtDeviceConfig;

/**
 * @name PCI Configuration Space Access
 * @brief Callbacks for PCI configuration space access
 */
typedef struct VBoxExtPciCallbacks
{
    /**
     * @brief Read from PCI configuration space
     * @param pvContext    Context pointer
     * @param u8Bus        PCI bus number
     * @param u8Dev        PCI device number
     * @param u8Fun        PCI function number
     * @param u16Offset    Register offset (0-255)
     * @param pu32Value    Output value (4 bytes)
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPciRead)(void *pvContext, uint8_t u8Bus, uint8_t u8Dev, uint8_t u8Fun,
                      uint16_t u16Offset, uint32_t *pu32Value);

    /**
     * @brief Write to PCI configuration space
     * @param pvContext    Context pointer
     * @param u8Bus        PCI bus number
     * @param u8Dev        PCI device number
     * @param u8Fun        PCI function number
     * @param u16Offset    Register offset (0-255)
     * @param u32Value     Value to write
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPciWrite)(void *pvContext, uint8_t u8Bus, uint8_t u8Dev, uint8_t u8Fun,
                       uint16_t u16Offset, uint32_t u32Value);

    /**
     * @brief Enable/disable PCI interrupt
     * @param pvContext    Context pointer
     * @param u8Bus        PCI bus number
     * @param u8Dev        PCI device number
     * @param u8Fun        PCI function number
     * @param fEnable      True to enable, false to disable
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPciSetIrq)(void *pvContext, uint8_t u8Bus, uint8_t u8Dev, uint8_t u8Fun, bool fEnable);
} VBoxExtPciCallbacks;

/**
 * @name I/O Registration Callbacks
 * @brief Callbacks for I/O resource registration
 */
typedef struct VBoxExtIoCallbacks
{
    /**
     * @brief Register port I/O range
     * @param pvContext    Context pointer
     * @param pszName      Name for debugging
     * @param uPortStart   Starting port
     * @param cPorts       Number of ports
     * @param pvCallback   Callback data
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterIoPort)(void *pvContext, const char *pszName,
                             uint16_t uPortStart, uint16_t cPorts, void *pvCallback);

    /**
     * @brief Unregister port I/O range
     * @param pvContext    Context pointer
     * @param uPortStart   Starting port
     * @param cPorts       Number of ports
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnUnregisterIoPort)(void *pvContext, uint16_t uPortStart, uint16_t cPorts);

    /**
     * @brief Register MMIO region
     * @param pvContext    Context pointer
     * @param pszName      Name for debugging
     * @param uAddress     Physical address
     * @param cbSize       Size in bytes
     * @param fFlags       Flags (read/write/execute)
     * @param pvCallback   Callback data
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterMmio)(void *pvContext, const char *pszName,
                           uint64_t uAddress, uint32_t cbSize,
                           uint32_t fFlags, void *pvCallback);

    /**
     * @brief Unregister MMIO region
     * @param pvContext    Context pointer
     * @param uAddress     Physical address
     * @param cbSize       Size in bytes
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnUnregisterMmio)(void *pvContext, uint64_t uAddress, uint32_t cbSize);

    /**
     * @brief Register IRQ handler
     * @param pvContext    Context pointer
     * @param u8Irq        IRQ line
     * @param fLevel       Level-triggered if true, edge-triggered if false
     * @param pvCallback   Callback data
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterIrq)(void *pvContext, uint8_t u8Irq, bool fLevel, void *pvCallback);

    /**
     * @brief Unregister IRQ handler
     * @param pvContext    Context pointer
     * @param u8Irq        IRQ line
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnUnregisterIrq)(void *pvContext, uint8_t u8Irq);

    /**
     * @brief Register DMA channel
     * @param pvContext    Context pointer
     * @param u8Channel    DMA channel (0-7)
     * @param pvCallback   Callback data
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterDma)(void *pvContext, uint8_t u8Channel, void *pvCallback);

    /**
     * @brief Unregister DMA channel
     * @param pvContext    Context pointer
     * @param u8Channel    DMA channel (0-7)
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnUnregisterDma)(void *pvContext, uint8_t u8Channel);
} VBoxExtIoCallbacks;

/**
 * @name IVBoxExtDevice Interface
 * @brief Unified interface that all imported devices must implement
 *
 * This is the core interface that all devices (QEMU, Bochs, 86Box, native VBox)
 * must implement to be compatible with the LibreVMM device abstraction layer.
 */
typedef struct IVBoxExtDevice
{
    /**
     * @brief Get the device name
     * @return Static device name string
     */
    const char *(*pfnGetName)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Get the device type
     * @return Device type enumeration
     */
    VBoxExtDeviceType (*pfnGetType)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Get device capability flags
     * @return Bitmask of DEVICE_CAP_* flags
     */
    uint32_t (*pfnGetCapabilities)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Get the supported bus type
     * @return Primary bus type this device attaches to
     */
    VBoxExtBusType (*pfnGetBusType)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Get current device state
     * @return Current device state
     */
    VBoxExtDeviceState (*pfnGetState)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Construct the device
     * @param pDevice      Device instance
     * @param pConfig      Configuration data
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnConstruct)(struct IVBoxExtDevice *pDevice, const VBoxExtDeviceConfig *pConfig);

    /**
     * @brief Destruct the device
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnDestruct)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Initialize the device
     * @param pDevice      Device instance
     * @param pIoCallbacks I/O registration callbacks
     * @param pvIoContext  Context for I/O callbacks
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnInitialize)(struct IVBoxExtDevice *pDevice,
                         const VBoxExtIoCallbacks *pIoCallbacks, void *pvIoContext);

    /**
     * @brief Reset the device
     * @param pDevice      Device instance
     * @param fWarm        True for warm reset, false for cold reset
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnReset)(struct IVBoxExtDevice *pDevice, bool fWarm);

    /**
     * @brief Power on the device
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPowerOn)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Power off the device
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPowerOff)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Attach device to a bus
     * @param pDevice      Device instance
     * @param enmBus       Bus type to attach to
     * @param uBus         Bus number
     * @param uDev         Device number (PCI)
     * @param uFun         Function number (PCI)
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnAttach)(struct IVBoxExtDevice *pDevice, VBoxExtBusType enmBus,
                     uint8_t uBus, uint8_t uDev, uint8_t uFun);

    /**
     * @brief Detach device from a bus
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnDetach)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Save device state
     * @param pDevice      Device instance
     * @param pvBuffer     Output buffer
     * @param cbBuffer     Buffer size
     * @param pcbSaved     Bytes written
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnSaveState)(struct IVBoxExtDevice *pDevice, void *pvBuffer,
                        size_t cbBuffer, size_t *pcbSaved);

    /**
     * @brief Restore device state
     * @param pDevice      Device instance
     * @param pvBuffer     Input buffer
     * @param cbBuffer     Buffer size
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnLoadState)(struct IVBoxExtDevice *pDevice, const void *pvBuffer,
                        size_t cbBuffer);

    /**
     * @brief Pause device execution
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnPause)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Resume device execution
     * @param pDevice      Device instance
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnResume)(struct IVBoxExtDevice *pDevice);

    /**
     * @brief Register PCI callbacks (for PCI devices)
     * @param pDevice      Device instance
     * @param pCallbacks   PCI callbacks
     * @param pvContext    Context for PCI callbacks
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnSetPciCallbacks)(struct IVBoxExtDevice *pDevice,
                              const VBoxExtPciCallbacks *pCallbacks, void *pvContext);

    /**
     * @brief Query device for a specific interface
     * @param pDevice      Device instance
     * @param pszIfaceId   Interface identifier
     * @param ppvIface     Output interface pointer
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnQueryInterface)(struct IVBoxExtDevice *pDevice, const char *pszIfaceId,
                             void **ppvIface);

    /**
     * @brief Device-specific I/O port read handler
     * @param pDevice      Device instance
     * @param uPort        Port address
     * @param pu32Value    Output value
     * @return VBOX_SUCCESS on success
     */
    int (*pfnIoPortRead)(struct IVBoxExtDevice *pDevice, uint16_t uPort, uint32_t *pu32Value);

    /**
     * @brief Device-specific I/O port write handler
     * @param pDevice      Device instance
     * @param uPort        Port address
     * @param u32Value     Value to write
     * @return VBOX_SUCCESS on success
     */
    int (*pfnIoPortWrite)(struct IVBoxExtDevice *pDevice, uint16_t uPort, uint32_t u32Value);

    /**
     * @brief Device-specific MMIO read handler
     * @param pDevice      Device instance
     * @param uAddress     Memory address
     * @param cbRead       Bytes to read (1,2,4,8)
     * @param pu64Value    Output value
     * @return VBOX_SUCCESS on success
     */
    int (*pfnMmioRead)(struct IVBoxExtDevice *pDevice, uint64_t uAddress,
                       uint32_t cbRead, uint64_t *pu64Value);

    /**
     * @brief Device-specific MMIO write handler
     * @param pDevice      Device instance
     * @param uAddress     Memory address
     * @param cbWrite      Bytes to write (1,2,4,8)
     * @param u64Value     Value to write
     * @return VBOX_SUCCESS on success
     */
    int (*pfnMmioWrite)(struct IVBoxExtDevice *pDevice, uint64_t uAddress,
                        uint32_t cbWrite, uint64_t u64Value);

    /**
     * @brief Device-specific IRQ handler
     * @param pDevice      Device instance
     * @param u8Irq        IRQ line
     * @return VBOX_SUCCESS on success
     */
    int (*pfnIrqHandler)(struct IVBoxExtDevice *pDevice, uint8_t u8Irq);

    /**
     * @brief Device-specific DMA handler
     * @param pDevice      Device instance
     * @param u8Channel    DMA channel
     * @return VBOX_SUCCESS on success
     */
    int (*pfnDmaHandler)(struct IVBoxExtDevice *pDevice, uint8_t u8Channel);

} IVBoxExtDevice;

/**
 * @name Helper Macros for Interface Implementation
 * @brief Macros to simplify IVBoxExtDevice implementation
 */
#define VBOX_EXT_DEVICE_INITIALIZER \
    { \
        NULL, /* pfnGetName */ \
        NULL, /* pfnGetType */ \
        NULL, /* pfnGetCapabilities */ \
        NULL, /* pfnGetBusType */ \
        NULL, /* pfnGetState */ \
        NULL, /* pfnConstruct */ \
        NULL, /* pfnDestruct */ \
        NULL, /* pfnInitialize */ \
        NULL, /* pfnReset */ \
        NULL, /* pfnPowerOn */ \
        NULL, /* pfnPowerOff */ \
        NULL, /* pfnAttach */ \
        NULL, /* pfnDetach */ \
        NULL, /* pfnSaveState */ \
        NULL, /* pfnLoadState */ \
        NULL, /* pfnPause */ \
        NULL, /* pfnResume */ \
        NULL, /* pfnSetPciCallbacks */ \
        NULL, /* pfnQueryInterface */ \
        NULL, /* pfnIoPortRead */ \
        NULL, /* pfnIoPortWrite */ \
        NULL, /* pfnMmioRead */ \
        NULL, /* pfnMmioWrite */ \
        NULL, /* pfnIrqHandler */ \
        NULL  /* pfnDmaHandler */ \
    }

/* Virtual function table declaration helper */
#define VBOX_EXT_DEVICE_VTBL_DECLARE(name) static const IVBoxExtDevice name##_vtbl

/* Virtual function table definition helper */
#define VBOX_EXT_DEVICE_VTBL_BEGIN(name) static const IVBoxExtDevice name##_vtbl =
#define VBOX_EXT_DEVICE_VTBL_END(name) ;

/**
 * @name Return Codes
 * @brief Standard return codes for device operations
 */
#define VBOX_SUCCESS                     0
#define VBOXERR_GENERIC_FAILURE          (-1)
#define VBOXERR_INVALID_PARAM            (-2)
#define VBOXERR_INVALID_STATE            (-3)
#define VBOXERR_NOT_SUPPORTED            (-4)
#define VBOXERR_NO_MEMORY                (-5)
#define VBOXERR_NO_RESOURCES             (-6)
#define VBOXERR_CONFLICTING_RESOURCES    (-7)
#define VBOXERR_DEVICE_NOT_FOUND         (-8)
#define VBOXERR_ALREADY_EXISTS           (-9)
#define VBOXERR_TIMEOUT                  (-10)

/**
 * @name Bus Type String Conversion
 * @brief Convert bus type to/from string
 */
const char *VBoxExtBusTypeToString(VBoxExtBusType enmBus);
VBoxExtBusType VBoxExtBusTypeFromString(const char *pszBus);

/**
 * @name Device Type String Conversion
 * @brief Convert device type to/from string
 */
const char *VBoxExtDeviceTypeToString(VBoxExtDeviceType enmType);
VBoxExtDeviceType VBoxExtDeviceTypeFromString(const char *pszType);

/**
 * @name Device Capability String Conversion
 * @brief Convert capability flags to/from string
 */
const char *VBoxExtCapabilityToString(uint32_t fCaps);
uint32_t VBoxExtCapabilityFromString(const char *pszCap);

#endif /* VBOX_EXT_DEVICE_H */
