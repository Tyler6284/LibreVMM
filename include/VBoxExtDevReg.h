/**
 * @file VBoxExtDevReg.h
 * @brief Device Abstraction Layer - Device Registry Interface
 *
 * This header defines the device registry interface for runtime device discovery.
 * Device modules should expose VBoxDevicesRegister() to register with the system.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef VBOX_EXT_DEV_REG_H
#define VBOX_EXT_DEV_REG_H

#include "VBoxExtDevice.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name Device Registration Magic
 * @brief Magic number to validate registration structures
 */
#define VBOX_EXT_DEVREG_MAGIC           0xDEADBEEFU
#define VBOX_EXT_DEVREG_VERSION_MAJOR   1
#define VBOX_EXT_DEVREG_VERSION_MINOR   0

/**
 * @name Device Registration Flags
 * @brief Flags for device registration behavior
 */
#define VBOX_EXT_DEVREG_F_NONE          0x00000000U
#define VBOX_EXT_DEVREG_F_EXPERIMENTAL  0x00000001U  /**< Device is experimental */
#define VBOX_EXT_DEVREG_F_HIDDEN        0x00000002U  /**< Device hidden from UI */
#define VBOX_EXT_DEVREG_F_REQUIRE_ROM   0x00000004U  /**< Device requires external ROM */
#define VBOX_EXT_DEVREG_F_BRIDGE        0x00000008U  /**< Device is a bus bridge */
#define VBOX_EXT_DEVREG_F_MULTI_INSTANCE 0x00000010U /**< Multiple instances allowed */
#define VBOX_EXT_DEVREG_F_REQUIRE_HARDWARE 0x00000020U /**< Requires hardware virtualization */
#define VBOX_EXT_DEVREG_F_NO_SAVE_STATE 0x00000040U  /**< Does not support save/restore */
#define VBOX_EXT_DEVREG_F_INJECTABLE    0x00000080U  /**< Device can inject interrupts */

/**
 * @name Device Source
 * @brief Identifies the source of the device implementation
 */
typedef enum VBoxExtDevSource
{
    VBOX_EXT_DEV_SOURCE_NONE = 0,
    VBOX_EXT_DEV_SOURCE_VBOX,      /**< Native VirtualBox device */
    VBOX_EXT_DEV_SOURCE_QEMU,      /**< Imported from QEMU */
    VBOX_EXT_DEV_SOURCE_BOCHS,     /**< Imported from Bochs */
    VBOX_EXT_DEV_SOURCE_86BOX,    /**< Imported from 86Box */
    VBOX_EXT_DEV_SOURCE_DOSBOX,    /**< Imported from DOSBox-X */
    VBOX_EXT_DEV_SOURCE_THIRD_PARTY, /**< Third-party device module */
    VBOX_EXT_DEV_SOURCE_MAX
} VBoxExtDevSource;

/**
 * @name Device Source String Conversion
 */
const char *VBoxExtDevSourceToString(VBoxExtDevSource enmSource);
VBoxExtDevSource VBoxExtDevSourceFromString(const char *pszSource);

/**
 * @name Device Registration Info
 * @brief Core device information for registration
 */
typedef struct VBoxExtDevRegInfo
{
    /**< Magic number for validation (VBOX_EXT_DEVREG_MAGIC) */
    uint32_t            u32Magic;

    /**< Major version of registration structure */
    uint16_t            u16VersionMajor;

    /**< Minor version of registration structure */
    uint16_t            u16VersionMinor;

    /**< Device name (e.g., "e1000", "VGA", "AHCI") */
    const char         *pszName;

    /**< Device description for UI display */
    const char         *pszDescription;

    /**< Device type classification */
    VBoxExtDeviceType   enmDeviceType;

    /**< Device source (VBOX, QEMU, BOCHS, etc.) */
    VBoxExtDevSource    enmSource;

    /**< Device capability flags */
    uint32_t            fCaps;

    /**< Registration flags */
    uint32_t            fFlags;

    /**< Primary bus type */
    VBoxExtBusType      enmBusType;

    /**< Supported bus types (bitmask of VBoxExtBusType values) */
    uint32_t            fSupportedBuses;

    /**< Vendor ID (for PCI devices, 0xFFFFFFFF if N/A) */
    uint32_t            u32VendorId;

    /**< Device ID (for PCI devices, 0xFFFFFFFF if N/A) */
    uint32_t            u32DeviceId;

    /**< Sub-system Vendor ID (for PCI devices, 0xFFFFFFFF if N/A) */
    uint32_t            u32SubVendorId;

    /**< Sub-system Device ID (for PCI devices, 0xFFFFFFFF if N/A) */
    uint32_t            u32SubDeviceId;

    /**< Revision ID (for PCI devices, 0xFF if N/A) */
    uint8_t             u8Revision;

    /**< Class code (for PCI devices, 0xFFFFFF if N/A) */
    uint32_t            u32ClassCode;

    /**< Required ROM file path (optional, NULL if not required) */
    const char         *pszRequiredRom;

    /**< List of required configuration properties (NULL terminated) */
    const char         **papszRequiredProps;

    /**< List of optional configuration properties (NULL terminated) */
    const char         **papszOptionalProps;

    /**< Default configuration values (NULL terminated) */
    const char         **papszDefaultValues;

    /**< Reserved for future use */
    void                *pvReserved1;
    void                *pvReserved2;
    void                *pvReserved3;

} VBoxExtDevRegInfo;

/**
 * @name PDMDEVREGCB - Device Registration Callback
 * @brief Callback structure provided by the VM to devices during registration
 *
 * This is the callback structure that the device registration function
 * receives. It provides functions to register the device and its drivers.
 */
typedef struct PDMDEVREGCB
{
    /**
     * @brief Version of the callback structure
     * @todo: Define proper version scheme
     */
    uint32_t            u32Version;

    /**
     * @brief Register a device
     * @param pCallbacks      This callback structure
     * @param pDevRegInfo     Device registration info
     * @param ppDevice        Output pointer to device instance (optional)
     * @return VBOX_SUCCESS on success, error code on failure
     *
     * The device implementation should call this to register itself
     * with the VM's device registry.
     */
    int (*pfnRegisterDevice)(struct PDMDEVREGCB *pCallbacks,
                             VBoxExtDevRegInfo *pDevRegInfo,
                             IVBoxExtDevice **ppDevice);

    /**
     * @brief Register a driver for the device
     * @param pCallbacks      This callback structure
     * @param pszDriverName  Driver name
     * @param pszDeviceName  Associated device name
     * @param u32Major       Driver major version
     * @param u32Minor       Driver minor version
     * @return VBOX_SUCCESS on success, error code on failure
     *
     * Drivers provide the actual implementation for a device.
     */
    int (*pfnRegisterDriver)(struct PDMDEVREGCB *pCallbacks,
                              const char *pszDriverName,
                              const char *pszDeviceName,
                              uint32_t u32Major,
                              uint32_t u32Minor);

    /**
     * @brief Register a LCD driver (driver for character cell display)
     * @param pCallbacks      This callback structure
     * @param pszDriverName   Driver name
     * @param pszDeviceName   Associated device name
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterLdr)(struct PDMDEVREGCB *pCallbacks,
                          const char *pszDriverName,
                          const char *pszDeviceName);

    /**
     * @brief Register a R0 driver (ring 0 / kernel mode driver)
     * @param pCallbacks      This callback structure
     * @param pszDriverName   Driver name
     * @param pszModuleName   Module name
     * @param u32Major       Driver major version
     * @param u32Minor       Driver minor version
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterR0Driver)(struct PDMDEVREGCB *pCallbacks,
                                const char *pszDriverName,
                                const char *pszModuleName,
                                uint32_t u32Major,
                                uint32_t u32Minor);

    /**
     * @brief Register a R3 driver (ring 3 / user mode driver)
     * @param pCallbacks      This callback structure
     * @param pszDriverName   Driver name
     * @param pszModuleName   Module name
     * @param u32Major       Driver major version
     * @param u32Minor       Driver minor version
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterR3Driver)(struct PDMDEVREGCB *pCallbacks,
                                const char *pszDriverName,
                                const char *pszModuleName,
                                uint32_t u32Major,
                                uint32_t u32Minor);

    /**
     * @brief Query the VM for configuration
     * @param pCallbacks      This callback structure
     * @param pszKey          Configuration key
     * @param pszValue       Output buffer for value
     * @param cbValue         Size of output buffer
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnQueryConfig)(struct PDMDEVREGCB *pCallbacks,
                          const char *pszKey,
                          char *pszValue,
                          size_t cbValue);

    /**
     * @brief Get the device configuration directory
     * @param pCallbacks      This callback structure
     * @return Path to device configuration directory
     *
     * Devices can store ROM files and other data here.
     */
    const char *(*pfnGetConfigDir)(struct PDMDEVREGCB *pCallbacks);

    /**
     * @brief Get the shared folder base path
     * @param pCallbacks      This callback structure
     * @return Path to shared folders directory
     */
    const char *(*pfnGetSharedFoldersDir)(struct PDMDEVREGCB *pCallbacks);

    /**
     * @brief Register an internal driver (built-in, not loadable)
     * @param pCallbacks      This callback structure
     * @param pszDriverName   Driver name
     * @param pszDeviceName   Associated device name
     * @return VBOX_SUCCESS on success, error code on failure
     */
    int (*pfnRegisterInternalDriver)(struct PDMDEVREGCB *pCallbacks,
                                     const char *pszDriverName,
                                     const char *pszDeviceName);

} PDMDEVREGCB;

/**
 * @name Device Registration Function Type
 * @brief Function type that device modules must export
 *
 * Every device module must export a function of this type which
 * will be called to register the device with the VM.
 *
 * @param pCallbacks   Callback structure for registration
 * @return VBOX_SUCCESS on success, error code on failure
 */
typedef int (*PFNVBOXEXTDEVICESREGISTER)(PDMDEVREGCB *pCallbacks);

/**
 * @name Module Entry Point
 * @brief Name of the exported registration function
 */
#define VBOX_EXT_DEVICES_REGISTER_NAME  "VBoxDevicesRegister"

/**
 * @name Device Registration Macro
 * @brief Helper macro to create device registration info
 *
 * Usage:
 *   VBOX_EXT_DEVREG_DECLARE(MyDevice,
 *       .pszName = "mydevice",
 *       .pszDescription = "My Custom Device",
 *       .enmDeviceType = VBOX_EXT_DEV_TYPE_PCI_DEVICE,
 *       ...
 *   );
 */
#define VBOX_EXT_DEVREG_DECLARE(name, ...) \
    static VBoxExtDevRegInfo name##_devreg_info = \
    { \
        .u32Magic = VBOX_EXT_DEVREG_MAGIC, \
        .u16VersionMajor = VBOX_EXT_DEVREG_VERSION_MAJOR, \
        .u16VersionMinor = VBOX_EXT_DEVREG_VERSION_MINOR, \
        __VA_ARGS__ \
    }

/**
 * @name Device Module Descriptor
 * @brief Descriptor for a dynamically loaded device module
 */
typedef struct VBoxExtModuleDescriptor
{
    /**< Module file path */
    const char         *pszPath;

    /**< Module handle (platform-specific) */
    void               *pvModule;

    /**< Registration function pointer */
    PFNVBOXEXTDEVICESREGISTER pfnRegister;

    /**< Number of devices registered by this module */
    uint32_t            cDevices;

    /**< Array of device registration info pointers */
    VBoxExtDevRegInfo **papDevRegInfo;

    /**< Load time in milliseconds */
    uint64_t            msLoadTime;

    /**< True if module is loaded */
    bool                fLoaded;

} VBoxExtModuleDescriptor;

/**
 * @name Device Conflict Entry
 * @brief Describes a conflict between two devices
 */
typedef struct VBoxExtDeviceConflict
{
    /**< First device name */
    const char         *pszDevice1;

    /**< Second device name */
    const char         *pszDevice2;

    /**< Conflict reason description */
    const char         *pszReason;

    /**< Conflict severity: 0=warning, 1=error (incompatible) */
    uint8_t            u8Severity;

} VBoxExtDeviceConflict;

/**
 * @name Device Compatibility Result
 * @brief Result of device combination validation
 */
typedef struct VBoxExtDeviceValidationResult
{
    /**< Number of warnings */
    uint32_t            cWarnings;

    /**< Number of errors (incompatible devices) */
    uint32_t            cErrors;

    /**< Array of conflict descriptions (cWarnings + cErrors total) */
    VBoxExtDeviceConflict *paConflicts;

    /**< True if combination is valid (no errors) */
    bool                fValid;

} VBoxExtDeviceValidationResult;

/**
 * @name Device Enumeration Callback
 * @brief Callback for enumerating registered devices
 *
 * @param pDevRegInfo   Device registration info
 * @param pvUser        User-provided context
 * @return VBOX_SUCCESS to continue enumeration, non-VBOX_SUCCESS to stop
 */
typedef int (*PFNVBOXEXTDEVICEENUM)(const VBoxExtDevRegInfo *pDevRegInfo, void *pvUser);

/**
 * @brief Initialize the device registry system
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This function must be called before any other registry functions.
 */
int VBoxExtDevRegInit(void);

/**
 * @brief Shutdown the device registry system
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This function unloads all device modules and frees resources.
 */
int VBoxExtDevRegShutdown(void);

/**
 * @brief Set the device directory path
 * @param pszPath   Path to directory containing device modules
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This sets the VBOX_EXT_DEVICES_DIR path from which device
 * modules are loaded at runtime.
 */
int VBoxExtDevRegSetDevicesDir(const char *pszPath);

/**
 * @brief Get the device directory path
 * @return Path to device directory, or NULL if not set
 */
const char *VBoxExtDevRegGetDevicesDir(void);

/**
 * @brief Register a device module from a shared library
 * @param pszPath   Path to the device module shared library
 * @param ppModule  Output module descriptor (optional)
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * Loads a device module (.so/.dll) and calls its VBoxDevicesRegister
 * entry point to register the device(s).
 */
int VBoxExtDevRegRegisterModule(const char *pszPath, VBoxExtModuleDescriptor **ppModule);

/**
 * @brief Unregister and unload a device module
 * @param pModule   Module descriptor
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDevRegUnregisterModule(VBoxExtModuleDescriptor *pModule);

/**
 * @brief Discover and load all device modules in the device directory
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * Scans the device directory for shared libraries and attempts
 * to load each one.
 */
int VBoxExtDevRegDiscoverDevices(void);

/**
 * @brief Register a device (internal use)
 * @param pDevRegInfo   Device registration info
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This is called internally by device modules through the
 * PDMDEVREGCB callback.
 */
int VBoxExtDevRegRegisterDevice(VBoxExtDevRegInfo *pDevRegInfo);

/**
 * @brief Unregister a device
 * @param pszDeviceName   Device name
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDevRegUnregisterDevice(const char *pszDeviceName);

/**
 * @brief Find a device by name
 * @param pszDeviceName   Device name
 * @return Device registration info, or NULL if not found
 */
const VBoxExtDevRegInfo *VBoxExtDevRegFindDevice(const char *pszDeviceName);

/**
 * @brief Enumerate all registered devices
 * @param pfnCallback   Callback function for each device
 * @param pvUser        User context passed to callback
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDevRegEnumerateDevices(PFNVBOXEXTDEVICEENUM pfnCallback, void *pvUser);

/**
 * @brief Get count of registered devices
 * @return Number of registered devices
 */
uint32_t VBoxExtDevRegGetDeviceCount(void);

/**
 * @brief Find devices by type
 * @param enmType        Device type to filter by
 * @param paDevRegInfo   Output array (pre-allocated by caller)
 * @param cMaxDevices    Maximum number of devices to return
 * @return Number of devices found and written to array
 */
uint32_t VBoxExtDevRegFindDevicesByType(VBoxExtDeviceType enmType,
                                         VBoxExtDevRegInfo *paDevRegInfo,
                                         uint32_t cMaxDevices);

/**
 * @brief Find devices by source
 * @param enmSource      Device source to filter by
 * @param paDevRegInfo   Output array (pre-allocated by caller)
 * @param cMaxDevices    Maximum number of devices to return
 * @return Number of devices found and written to array
 */
uint32_t VBoxExtDevRegFindDevicesBySource(VBoxExtDevSource enmSource,
                                           VBoxExtDevRegInfo *paDevRegInfo,
                                           uint32_t cMaxDevices);

/**
 * @brief Find devices by capability
 * @param fCaps         Required capability flags (all must match)
 * @param paDevRegInfo  Output array (pre-allocated by caller)
 * @param cMaxDevices   Maximum number of devices to return
 * @return Number of devices found and written to array
 */
uint32_t VBoxExtDevRegFindDevicesByCaps(uint32_t fCaps,
                                         VBoxExtDevRegInfo *paDevRegInfo,
                                         uint32_t cMaxDevices);

/**
 * @brief Find devices by bus type
 * @param enmBus        Bus type to filter by
 * @param paDevRegInfo  Output array (pre-allocated by caller)
 * @param cMaxDevices   Maximum number of devices to return
 * @return Number of devices found and written to array
 */
uint32_t VBoxExtDevRegFindDevicesByBus(VBoxExtBusType enmBus,
                                        VBoxExtDevRegInfo *paDevRegInfo,
                                        uint32_t cMaxDevices);

/**
 * @brief Validate device combination
 * @param pszDevices    Array of device names
 * @param cDevices      Number of devices
 * @param pResult       Output validation result (caller must free paConflicts)
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This validates a combination of devices without enforcing constraints.
 * It warns about potential conflicts but never auto-corrects.
 */
int VBoxExtDevRegValidateCombination(const char **pszDevices,
                                      uint32_t cDevices,
                                      VBoxExtDeviceValidationResult *pResult);

/**
 * @brief Free device validation result
 * @param pResult   Validation result to free
 */
void VBoxExtDevRegFreeValidationResult(VBoxExtDeviceValidationResult *pResult);

/**
 * @brief Register a device conflict rule
 * @param pszDevice1    First device name
 * @param pszDevice2    Second device name
 * @param pszReason     Conflict reason description
 * @param u8Severity    Severity (0=warning, 1=error)
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * Adds a conflict rule to the validation system.
 */
int VBoxExtDevRegAddConflictRule(const char *pszDevice1,
                                  const char *pszDevice2,
                                  const char *pszReason,
                                  uint8_t u8Severity);

#endif /* VBOX_EXT_DEV_REG_H */
