/**
 * @file VBoxExtDevices.h
 * @brief Device Abstraction Layer - Central Device Catalog
 *
 * This header provides the central device catalog that tracks all available
 * devices at runtime. It includes headers for all device types, defines
 * device discovery functions, and implements device capability lookup.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef VBOX_EXT_DEVICES_H
#define VBOX_EXT_DEVICES_H

#include "../include/VBoxExtDevice.h"
#include "../include/VBoxExtDevReg.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name Device Catalog Initialization
 * @brief Initialize the device catalog
 *
 * Must be called before using any device catalog functions.
 */
int VBoxExtDevicesInit(void);

/**
 * @brief Shutdown the device catalog
 *
 * Frees all resources allocated by the catalog.
 */
void VBoxExtDevicesShutdown(void);

/**
 * @name Device Capability Lookup
 * @brief Query device capabilities
 */

/**
 * @brief Check if a device supports a specific capability
 * @param pszDeviceName   Device name
 * @param fCaps           Capability flags to check
 * @return true if device has ALL specified capabilities
 */
bool VBoxExtDevicesHasCapability(const char *pszDeviceName, uint32_t fCaps);

/**
 * @brief Get capability flags for a device
 * @param pszDeviceName   Device name
 * @return Capability flags, or DEVICE_CAP_NONE if device not found
 */
uint32_t VBoxExtDevicesGetCapabilities(const char *pszDeviceName);

/**
 * @brief Get device source
 * @param pszDeviceName   Device name
 * @return Device source, or VBOX_EXT_DEV_SOURCE_NONE if not found
 */
VBoxExtDevSource VBoxExtDevicesGetSource(const char *pszDeviceName);

/**
 * @name Device Information Retrieval
 * @brief Query device information
 */

/**
 * @brief Get device description
 * @param pszDeviceName   Device name
 * @return Device description, or NULL if not found
 */
const char *VBoxExtDevicesGetDescription(const char *pszDeviceName);

/**
 * @brief Get device type
 * @param pszDeviceName   Device name
 * @return Device type, or VBOX_EXT_DEV_TYPE_NONE if not found
 */
VBoxExtDeviceType VBoxExtDevicesGetType(const char *pszDeviceName);

/**
 * @brief Get supported bus types
 * @param pszDeviceName   Device name
 * @return Bitmask of supported bus types, or 0 if not found
 */
uint32_t VBoxExtDevicesGetSupportedBuses(const char *pszDeviceName);

/**
 * @brief Check if device requires ROM
 * @param pszDeviceName   Device name
 * @return true if device requires external ROM file
 */
bool VBoxExtDevicesRequiresRom(const char *pszDeviceName);

/**
 * @brief Get required ROM path
 * @param pszDeviceName   Device name
 * @return Required ROM path, or NULL if not required
 */
const char *VBoxExtDevicesGetRequiredRom(const char *pszDeviceName);

/**
 * @name Device Filtering
 * @brief Filter devices by various criteria
 */

/**
 * @brief Get all devices matching device type
 * @param enmType         Device type to filter
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetByType(VBoxExtDeviceType enmType, uint32_t *pcbCount);

/**
 * @brief Get all devices matching device source
 * @param enmSource       Device source to filter
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetBySource(VBoxExtDevSource enmSource, uint32_t *pcbCount);

/**
 * @brief Get all devices supporting a bus type
 * @param enmBus          Bus type to filter
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetByBus(VBoxExtBusType enmBus, uint32_t *pcbCount);

/**
 * @brief Get all devices supporting required capabilities
 * @param fCaps           Required capability flags
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetByCapabilities(uint32_t fCaps, uint32_t *pcbCount);

/**
 * @brief Get all devices that are NOT experimental
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetStable(uint32_t *pcbCount);

/**
 * @brief Get all devices that don't require hardware virtualization
 * @param pcbCount        Output: number of devices found
 * @return Array of device names (caller must free), or NULL on error
 */
char **VBoxExtDevicesGetSoftwareEmulated(uint32_t *pcbCount);

/**
 * @name Device Discovery Functions
 * @brief Functions for discovering devices at runtime
 */

/**
 * @brief Scan device directory and discover available devices
 * @param pszDevicesDir   Path to device directory (NULL for default)
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This function scans the specified directory for device modules
 * (.so on Linux, .dll on Windows) and registers any devices found.
 */
int VBoxExtDevicesDiscover(const char *pszDevicesDir);

/**
 * @brief Refresh device catalog
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * Re-scans the device directory for new or removed devices.
 */
int VBoxExtDevicesRefresh(void);

/**
 * @name Device Instance Management
 * @brief Manage device instances
 */

/**
 * @brief Create a device instance
 * @param pszDeviceName   Device name
 * @param pConfig         Device configuration
 * @param ppDevice        Output: created device instance
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDevicesCreateInstance(const char *pszDeviceName,
                                  const VBoxExtDeviceConfig *pConfig,
                                  IVBoxExtDevice **ppDevice);

/**
 * @brief Destroy a device instance
 * @param pDevice         Device instance to destroy
 * @return VBOX_SUCCESS on success, error code on failure
 */
int VBoxExtDevicesDestroyInstance(IVBoxExtDevice *pDevice);

/**
 * @name Device Enumeration
 * @brief Iterate over all available devices
 */

/**
 * @brief Callback for device enumeration
 * @param pszDeviceName   Device name
 * @param pvUser          User context
 * @return VBOX_SUCCESS to continue, non-VBOX_SUCCESS to stop
 */
typedef int (*PFNVBOXEXTDEVICESENUM)(const char *pszDeviceName, void *pvUser);

/**
 * @brief Enumerate all available devices
 * @param pfnCallback     Callback function
 * @param pvUser          User context
 * @return VBOX_SUCCESS on success
 */
int VBoxExtDevicesEnumerate(PFNVBOXEXTDEVICESENUM pfnCallback, void *pvUser);

/**
 * @name Device Statistics
 * @brief Query catalog statistics
 */

/**
 * @brief Get total number of available devices
 * @return Device count
 */
uint32_t VBoxExtDevicesGetCount(void);

/**
 * @brief Get number of devices by type
 * @param enmType         Device type
 * @return Count of devices of specified type
 */
uint32_t VBoxExtDevicesGetCountByType(VBoxExtDeviceType enmType);

/**
 * @brief Get number of devices by source
 * @param enmSource       Device source
 * @return Count of devices from specified source
 */
uint32_t VBoxExtDevicesGetCountBySource(VBoxExtDevSource enmSource);

/**
 * @name Device Validation
 * @brief Validate device configurations
 */

/**
 * @brief Check if two devices are compatible
 * @param pszDevice1      First device name
 * @param pszDevice2      Second device name
 * @param ppszReason      Output: reason if incompatible (caller must free)
 * @return VBOX_SUCCESS if compatible, VBOXERR_CONFLICTING_RESOURCES if incompatible
 */
int VBoxExtDevicesCheckCompatibility(const char *pszDevice1,
                                      const char *pszDevice2,
                                      char **ppszReason);

/**
 * @brief Get list of devices incompatible with a given device
 * @param pszDeviceName   Device name
 * @param pcbCount        Output: number of incompatible devices
 * @return Array of incompatible device names (caller must free)
 */
char **VBoxExtDevicesGetIncompatible(const char *pszDeviceName, uint32_t *pcbCount);

/**
 * @brief Get list of recommended companion devices
 * @param pszDeviceName   Device name
 * @param pcbCount        Output: number of recommended devices
 * @return Array of recommended device names (caller must free)
 */
char **VBoxExtDevicesGetCompanions(const char *pszDeviceName, uint32_t *pcbCount);

/**
 * @name Device Capability Groups
 * @brief Predefined device capability queries
 */

/**
 * @brief Get all 3D-capable video devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGet3DCapable(uint32_t *pcbCount);

/**
 * @brief Get all legacy ISA devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetLegacyISA(uint32_t *pcbCount);

/**
 * @brief Get all USB 3.0 capable devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetUSB3Capable(uint32_t *pcbCount);

/**
 * @brief Get all MCA devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetMCACapable(uint32_t *pcbCount);

/**
 * @brief Get all virtio devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetVirtioCapable(uint32_t *pcbCount);

/**
 * @brief Get all storage controller devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetStorageControllers(uint32_t *pcbCount);

/**
 * @brief Get all network controller devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetNetworkControllers(uint32_t *pcbCount);

/**
 * @brief Get all audio controller devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetAudioControllers(uint32_t *pcbCount);

/**
 * @brief Get all video controller devices
 * @param pcbCount        Output: number of devices
 * @return Array of device names (caller must free)
 */
char **VBoxExtDevicesGetVideoControllers(uint32_t *pcbCount);

/**
 * @name String Array Utilities
 * @brief Helper functions for working with device name arrays
 */

/**
 * @brief Free a device name array
 * @param papszDevices    Array to free
 * @param cDevices        Number of elements
 */
void VBoxExtDevicesFreeArray(char **papszDevices, uint32_t cDevices);

/**
 * @brief Duplicate a device name array
 * @param papszDevices    Array to duplicate
 * @param cDevices        Number of elements
 * @return New allocated array (caller must free)
 */
char **VBoxExtDevicesDuplicateArray(const char **papszDevices, uint32_t cDevices);

/**
 * @name Built-in Device Registration
 * @brief Register core devices without loading modules
 */

/**
 * @brief Register all built-in (static) devices
 * @return VBOX_SUCCESS on success, error code on failure
 *
 * This registers devices that are compiled into the main binary
 * rather than loaded from separate modules.
 */
int VBoxExtDevicesRegisterBuiltin(void);

/**
 * @name Default Device Directory
 * @brief Default location for device modules
 */

/**
 * @brief Get the default device directory
 * @return Default device directory path (do not free)
 */
const char *VBoxExtDevicesGetDefaultDir(void);

/**
 * @brief Set the default device directory
 * @param pszPath   New default path
 * @return VBOX_SUCCESS on success
 */
int VBoxExtDevicesSetDefaultDir(const char *pszPath);

#endif /* VBOX_EXT_DEVICES_H */
