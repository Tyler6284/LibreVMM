/**
 * @file IVBoxFrontend.h
 * @brief Stable backend API for LibreVMM UI implementations
 * 
 * This interface defines all VM management operations that ALL UI frontends
 * must use. No UI implementation should directly access the VMM or device layer.
 * 
 * This API is designed to mirror and extend the VirtualBox 5.2 Main API while
 * providing a cleaner, platform-agnostic interface.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef IVBOXFRONTEND_H
#define IVBOXFRONTEND_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <cstdint>
#include <functional>
#include <optional>
#include <variant>

/**
 * @brief Core execution backend types
 */
enum class ExecutionBackendType
{
    HM,           /**< Hardware virtualization (VT-x/AMD-V) */
    TCG_JIT,      /**< Tiny Code Generator with JIT compilation */
    TCG_Interpreter /**< TCG interpreter for restricted environments */
};

/**
 * @brief VM power state
 */
enum class VMPowerState
{
    PoweredOff,
    Saved,
    Running,
    Paused,
    Stuck,
    Starting,
    Stopping,
    Saving,
    Restoring
};

/**
 * @brief Machine state for snapshots
 */
enum class MachineState
{
    PoweredOff,
    Saved,
    Running,
    Paused,
    Stuck,
    Teleporting,
    TakingSnapshot,
    RestoringSnapshot,
    DeletingSnapshot,
    SettingUp
};

/**
 * @brief Chipset types
 */
enum class ChipsetType
{
    PIIX3,     /**< Intel i440FX + PIIX3 */
    Q35,       /**< Intel Q35 + ICH9 */
    VirtualBox /**< VirtualBox native chipset */
};

/**
 * @brief BIOS/UEFI firmware types
 */
enum class FirmwareType
{
    BIOS,           /**< SeaBIOS legacy BIOS */
    UEFI,           /**< UEFI with OVMF */
    UEFI32,         /**< 32-bit UEFI */
    UEFI64,         /**< 64-bit UEFI */
    BochsBIOS,      /**< Bochs BIOS */
    OpenBIOS        /**< OpenBIOS */
};

/**
 * @brief Storage controller types
 */
enum class StorageControllerType
{
    IDE,
    SATA,
    SCSI,
    SAS,
    NVMe,
    Floppy,
    USB,
    Virtio_SCSI,
    Virtio_Block,
    LSI_SAS,
    MegaRAID
};

/**
 * @brief Network adapter types
 */
enum class NetworkAdapterType
{
    NAT,
    Bridged,
    HostOnly,
    Internal,
    Generic,
    NATNetwork,
    Virtio_Net,
    E1000,
    E1000E,
    RTL8139,
    NE2000,
    PCNet
};

/**
 * @brief Audio backend types
 */
enum class AudioBackendType
{
    Null,
    DirectSound,
    CoreAudio,
    ALSA,
    PulseAudio,
    PipeWire,
    SDL,
    WASAPI,
    OBS
};

/**
 * @brief Video device types
 */
enum class VideoDeviceType
{
    VBoxVGA,
    VBoxSVGA,
    VMSVGA,
    QXL,
    Virtio_GPU,
    VGA,
    BochsDisplay,
    VMwareSVGA,
    Cirrus,
    VMDevice
};

/**
 * @brief USB controller types
 */
enum class USBControllerType
{
    OHCI,
    EHCI,
    XHCI,
    USB3,
    Virtio_USB
};

/**
 * @brief Generic device attachment information
 */
struct DeviceAttachment
{
    std::string deviceType;      /**< e.g., "HardDisk", "Network", "Audio" */
    std::string controllerName;  /**< Controller this device is attached to */
    int port;                     /**< Port number on controller */
    int slot;                     /**< Device slot (for PCI) */
    std::string imagePath;        /**< For storage: disk image path */
    std::map<std::string, std::string> settings; /**< Device-specific settings */
};

/**
 * @brief VM hardware configuration
 */
struct VMHardwareConfig
{
    uint64_t memoryMB;           /**< RAM in megabytes */
    uint32_t cpuCount;           /**< Number of virtual CPUs */
    uint64_t vramMB;             /**< Video RAM in megabytes */
    uint64_t accelerate3D;       /**< Enable 3D acceleration */
    uint64_t accelerate2D;       /**< Enable 2D acceleration */
    bool ioAPIC;                 /**< Enable I/O APIC */
    bool PAE;                    /**< Enable PAE */
    bool nestedPaging;           /**< Enable nested paging */
    bool longMode;               /**< Enable long mode (64-bit) */
    bool hardwareVirt;           /**< Enable hardware virtualization */
    bool EFI;                    /**< Use UEFI instead of BIOS */
    std::string firmwareType;    /**< Firmware type string */
    std::string chipsetType;     /**< Chipset type string */
    std::string ostype;          /**< Guest OS type */
    std::string cpuProfile;      /**< CPU profile name */
};

/**
 * @brief Storage controller configuration
 */
struct StorageControllerConfig
{
    std::string name;                        /**< Controller name (e.g., "SATA", "IDE") */
    StorageControllerType type;               /**< Controller type */
    std::vector<DeviceAttachment> devices;   /**< Attached devices */
    bool bootable;                            /**< Bootable flag */
    bool hotpluggable;                       /**< Hot-pluggable */
};

/**
 * @brief Network adapter configuration
 */
struct NetworkAdapterConfig
{
    std::string name;                        /**< Adapter name */
    NetworkAdapterType type;                 /**< Adapter type */
    std::string macAddress;                  /**< MAC address */
    bool enabled;                             /**< Enabled flag */
    std::string hostInterface;               /**< Host interface for bridged mode */
    std::string internalNetwork;             /**< Internal network name */
    std::string natNetwork;                  /**< NAT network name */
    std::map<std::string, std::string> settings; /**< Additional settings */
};

/**
 * @brief Audio configuration
 */
struct AudioConfig
{
    bool enabled;                             /**< Audio enabled */
    AudioBackendType backend;                 /**< Audio backend */
    std::string controller;                  /**< Audio controller type */
    std::map<std::string, std::string> settings; /**< Additional settings */
};

/**
 * @brief Video device configuration
 */
struct VideoDeviceConfig
{
    int id;                                   /**< Device ID */
    VideoDeviceType type;                     /**< Video device type */
    uint64_t vramMB;                         /**< VRAM in MB */
    bool accelerate3D;                        /**< Enable 3D acceleration */
    bool accelerate2D;                        /**< Enable 2D acceleration */
    std::map<std::string, std::string> settings; /**< Additional settings */
};

/**
 * @brief USB controller configuration
 */
struct USBControllerConfig
{
    std::string name;                        /**< Controller name */
    USBControllerType type;                 /**< Controller type */
    bool enabled;                            /**< Enabled flag */
    std::vector<std::string> deviceFilters; /**< USB device filters */
};

/**
 * @brief Complete VM configuration
 */
struct VMConfig
{
    std::string id;                          /**< VM unique ID */
    std::string name;                        /**< VM name */
    std::string ostype;                      /**< Guest OS type */
    std::string description;                 /**< VM description */
    std::string baseFolder;                  /**< VM base folder */
    
    VMHardwareConfig hardware;
    std::vector<StorageControllerConfig> storageControllers;
    std::vector<NetworkAdapterConfig> networkAdapters;
    std::vector<AudioConfig> audioDevices;
    std::vector<VideoDeviceConfig> videoDevices;
    std::vector<USBControllerConfig> usbControllers;
    
    std::map<std::string, std::string> metadata; /**< Custom metadata */
};

/**
 * @brief Snapshot information
 */
struct SnapshotInfo
{
    std::string id;                          /**< Snapshot ID */
    std::string name;                        /**< Snapshot name */
    std::string description;                 /**< Snapshot description */
    std::string timeStamp;                   /**< Creation timestamp */
    std::string parentId;                    /**< Parent snapshot ID */
    VMConfig state;                          /**< VM state at snapshot */
};

/**
 * @brief VM runtime information
 */
struct VMRuntimeInfo
{
    std::string vmId;                        /**< VM ID */
    VMPowerState state;                      /**< Current power state */
    uint64_t uptime;                         /**< VM uptime in milliseconds */
    uint64_t memoryUsed;                     /**< Memory currently used */
    double cpuLoad;                          /**< CPU load percentage */
    double guestCPULoad;                     /**< Guest CPU load */
    uint64_t networkTransmitted;             /**< Network bytes sent */
    uint64_t networkReceived;                /**< Network bytes received */
    uint64_t diskRead;                       /**< Disk read bytes */
    uint64_t diskWritten;                    /**< Disk written bytes */
    std::string currentSnapshot;            /**< Current snapshot name */
};

/**
 * @brief Display information for framebuffer access
 */
struct DisplayInfo
{
    int displayNumber;                       /**< Display number (0-based) */
    uint32_t width;                          /**< Framebuffer width */
    uint32_t height;                         /**< Framebuffer height */
    uint32_t bitsPerPixel;                   /**< Bits per pixel */
    uint32_t pixelFormat;                    /**< Pixel format */
    std::vector<uint8_t> framebuffer;       /**< Framebuffer data */
    bool enabled;                            /**< Display enabled */
};

/**
 * @brief Progress operation information
 */
struct ProgressInfo
{
    int id;                                  /**< Progress ID */
    std::string description;                /**< Operation description */
    double percent;                         /**< Completion percentage (0-100) */
    bool canceled;                           /**< Cancel requested */
    std::string result;                      /**< Result string */
};

/**
 * @brief Event types for frontend callbacks
 */
enum class FrontendEventType
{
    VMStateChanged,
    VMProgress,
    DeviceChanged,
    MediumChanged,
    NetworkChanged,
    USBChanged,
    DisplayChanged,
    RuntimeError,
    CanShowWindow,
    ShowWindow
};

/**
 * @brief Event data for frontend callbacks
 */
struct FrontendEvent
{
    FrontendEventType type;
    std::string vmId;
    std::variant<std::string, int, double, bool, ProgressInfo> data;
};

/**
 * @brief Callback type for frontend events
 */
using FrontendEventCallback = std::function<void(const FrontendEvent&)>;

/**
 * @brief Virtual Machine descriptor for listing
 */
struct VMDescriptor
{
    std::string id;
    std::string name;
    std::string ostype;
    std::string state;
    std::string sessionName;
    std::string sessionPID;
};

/**
 * @brief Medium (disk) information
 */
struct MediumInfo
{
    std::string id;
    std::string name;
    std::string location;
    std::string type;           /**< HardDisk, DVD, Floppy */
    std::string format;         /**< VDI, VMDK, QCOW2, RAW */
    uint64_t size;              /**< Logical size */
    uint64_t actualSize;       /**< Actual size on disk */
    bool readOnly;
    bool encrypted;
};

/**
 * @brief Network adapter descriptor
 */
struct NetworkAdapterDescriptor
{
    std::string name;
    std::string description;
    bool available;
    std::string type;
};

/**
 * @brief Host interface descriptor for bridged networking
 */
struct HostInterfaceDescriptor
{
    std::string name;
    std::string address;
    std::string networkName;
    bool wireless;
};

/**
 * @brief System properties for host information
 */
struct SystemProperties
{
    uint64_t memorySize;           /**< Total host memory */
    uint32_t cpuCount;              /**< Number of CPUs */
    uint64_t freeMemory;           /**< Free memory */
    std::string osVersion;          /**< Host OS version */
    std::string osType;             /**< Host OS type */
    bool hardwareVirt;              /**< Hardware virtualization available */
    bool nestedPaging;              /**< Nested paging available */
    std::vector<std::string> ostypes; /**< Available OS types */
};

/**
 * @brief IVBoxFrontend - Stable backend API interface
 * 
 * This is the ONLY API that UI frontends should use. All VM management,
 * device configuration, and execution control goes through this interface.
 */
class IVBoxFrontend
{
public:
    virtual ~IVBoxFrontend() = default;

    // ===== VM Lifecycle Management =====

    /**
     * @brief Create a new virtual machine
     * @param config VM configuration
     * @return VM ID on success, empty string on failure
     */
    virtual std::string createVM(const VMConfig& config) = 0;

    /**
     * @brief Register an existing VM from a folder
     * @param folder Path to VM folder
     * @return VM ID on success
     */
    virtual std::string registerVM(const std::string& folder) = 0;

    /**
     * @brief Unregister a virtual machine
     * @param vmId VM ID
     * @param deleteFiles Whether to delete VM files
     */
    virtual void unregisterVM(const std::string& vmId, bool deleteFiles) = 0;

    /**
     * @brief Start a VM session
     * @param vmId VM ID
     * @param sessionType Session type (gui, headless, vrdp)
     * @return Session ID on success
     */
    virtual std::string startVM(const std::string& vmId, const std::string& sessionType) = 0;

    /**
     * @brief Power off a running VM
     * @param vmId VM ID
     */
    virtual void powerOffVM(const std::string& vmId) = 0;

    /**
     * @brief Save VM state (hibernate)
     * @param vmId VM ID
     */
    virtual void saveVMState(const std::string& vmId) = 0;

    /**
     * @brief Pause a running VM
     * @param vmId VM ID
     */
    virtual void pauseVM(const std::string& vmId) = 0;

    /**
     * @brief Resume a paused VM
     * @param vmId VM ID
     */
    virtual void resumeVM(const std::string& vmId) = 0;

    /**
     * @brief Reset a running VM
     * @param vmId VM ID
     */
    virtual void resetVM(const std::string& vmId) = 0;

    /**
     * @brief Get the current state of a VM
     * @param vmId VM ID
     * @return VM power state
     */
    virtual VMPowerState getVMState(const std::string& vmId) = 0;

    /**
     * @brief Get VM runtime information
     * @param vmId VM ID
     * @return Runtime information
     */
    virtual VMRuntimeInfo getVMRuntimeInfo(const std::string& vmId) = 0;

    // ===== VM Configuration =====

    /**
     * @brief Get VM configuration
     * @param vmId VM ID
     * @return VM configuration
     */
    virtual VMConfig getVMConfig(const std::string& vmId) = 0;

    /**
     * @brief Update VM configuration
     * @param vmId VM ID
     * @param config New configuration
     */
    virtual void updateVMConfig(const std::string& vmId, const VMConfig& config) = 0;

    /**
     * @brief Get available OS types
     * @return List of OS type identifiers
     */
    virtual std::vector<std::string> getOSTypes() = 0;

    /**
     * @brief Get system properties
     * @return System properties
     */
    virtual SystemProperties getSystemProperties() = 0;

    // ===== Storage Controllers =====

    /**
     * @brief Add a storage controller to a VM
     * @param vmId VM ID
     * @param controller Controller configuration
     */
    virtual void addStorageController(const std::string& vmId, const StorageControllerConfig& controller) = 0;

    /**
     * @brief Remove a storage controller
     * @param vmId VM ID
     * @param controllerName Controller name
     */
    virtual void removeStorageController(const std::string& vmId, const std::string& controllerName) = 0;

    /**
     * @brief Attach a medium to a storage controller
     * @param vmId VM ID
     * @param controllerName Controller name
     * @param port Port number
     * @param device Device number
     * @param mediumId Medium ID to attach
     */
    virtual void attachMedium(const std::string& vmId, const std::string& controllerName,
                              int port, int device, const std::string& mediumId) = 0;

    /**
     * @brief Detach a medium from a storage controller
     * @param vmId VM ID
     * @param controllerName Controller name
     * @param port Port number
     * @param device Device number
     */
    virtual void detachMedium(const std::string& vmId, const std::string& controllerName,
                              int port, int device) = 0;

    // ===== Network Adapters =====

    /**
     * @brief Add a network adapter to a VM
     * @param vmId VM ID
     * @param adapter Network adapter configuration
     */
    virtual void addNetworkAdapter(const std::string& vmId, const NetworkAdapterConfig& adapter) = 0;

    /**
     * @brief Remove a network adapter
     * @param vmId VM ID
     * @param slot Slot number
     */
    virtual void removeNetworkAdapter(const std::string& vmId, int slot) = 0;

    /**
     * @brief Update network adapter configuration
     * @param vmId VM ID
     * @param slot Slot number
     * @param adapter New configuration
     */
    virtual void updateNetworkAdapter(const std::string& vmId, int slot, 
                                       const NetworkAdapterConfig& adapter) = 0;

    // ===== Audio =====

    /**
     * @brief Configure audio for a VM
     * @param vmId VM ID
     * @param audio Audio configuration
     */
    virtual void setAudioConfig(const std::string& vmId, const AudioConfig& audio) = 0;

    // ===== Video =====

    /**
     * @brief Add a video device to a VM
     * @param vmId VM ID
     * @param video Video device configuration
     */
    virtual void addVideoDevice(const std::string& vmId, const VideoDeviceConfig& video) = 0;

    /**
     * @brief Remove a video device
     * @param vmId VM ID
     * @param slot Slot number
     */
    virtual void removeVideoDevice(const std::string& vmId, int slot) = 0;

    /**
     * @brief Update video device configuration
     * @param vmId VM ID
     * @param slot Slot number
     * @param video New configuration
     */
    virtual void updateVideoDevice(const std::string& vmId, int slot, 
                                    const VideoDeviceConfig& video) = 0;

    // ===== USB =====

    /**
     * @brief Add a USB controller
     * @param vmId VM ID
     * @param controller USB controller configuration
     */
    virtual void addUSBController(const std::string& vmId, const USBControllerConfig& controller) = 0;

    /**
     * @brief Remove a USB controller
     * @param vmId VM ID
     * @param name Controller name
     */
    virtual void removeUSBController(const std::string& vmId, const std::string& name) = 0;

    // ===== Snapshots =====

    /**
     * @brief Take a snapshot of a VM
     * @param vmId VM ID
     * @param name Snapshot name
     * @param description Snapshot description
     * @return Snapshot ID on success
     */
    virtual std::string takeSnapshot(const std::string& vmId, const std::string& name,
                                      const std::string& description) = 0;

    /**
     * @brief Restore a snapshot
     * @param vmId VM ID
     * @param snapshotId Snapshot ID
     */
    virtual void restoreSnapshot(const std::string& vmId, const std::string& snapshotId) = 0;

    /**
     * @brief Delete a snapshot
     * @param vmId VM ID
     * @param snapshotId Snapshot ID
     */
    virtual void deleteSnapshot(const std::string& vmId, const std::string& snapshotId) = 0;

    /**
     * @brief Get snapshots for a VM
     * @param vmId VM ID
     * @return List of snapshots
     */
    virtual std::vector<SnapshotInfo> getSnapshots(const std::string& vmId) = 0;

    // ===== Medium Management =====

    /**
     * @brief Create a new medium (disk image)
     * @param format Format type (VDI, VMDK, QCOW2, etc.)
     * @param location File location
     * @param size Size in bytes
     * @param mediumType Medium type (HardDisk, DVD, Floppy)
     * @return Medium ID on success
     */
    virtual std::string createMedium(const std::string& format, const std::string& location,
                                     uint64_t size, const std::string& mediumType) = 0;

    /**
     * @brief Get medium information
     * @param mediumId Medium ID
     * @return Medium information
     */
    virtual MediumInfo getMediumInfo(const std::string& mediumId) = 0;

    /**
     * @brief Get all registered media
     * @return List of media
     */
    virtual std::vector<MediumInfo> getMedia() = 0;

    /**
     * @brief Close/release a medium
     * @param mediumId Medium ID
     */
    virtual void closeMedium(const std::string& mediumId) = 0;

    // ===== Host Networking =====

    /**
     * @brief Get available host network interfaces
     * @return List of host interfaces
     */
    virtual std::vector<HostInterfaceDescriptor> getHostInterfaces() = 0;

    /**
     * @brief Get available network adapters
     * @return List of network adapters
     */
    virtual std::vector<NetworkAdapterDescriptor> getNetworkAdapters() = 0;

    // ===== VM Listing =====

    /**
     * @brief Get list of all VMs
     * @return List of VM descriptors
     */
    virtual std::vector<VMDescriptor> listVMs() = 0;

    /**
     * @brief Find a VM by name
     * @param name VM name
     * @return VM ID if found, empty string otherwise
     */
    virtual std::string findVMByName(const std::string& name) = 0;

    // ===== Execution Control =====

    /**
     * @brief Set the execution backend for a VM
     * @param vmId VM ID
     * @param backend Execution backend type
     */
    virtual void setExecutionBackend(const std::string& vmId, ExecutionBackendType backend) = 0;

    /**
     * @brief Get the current execution backend
     * @param vmId VM ID
     * @return Execution backend type
     */
    virtual ExecutionBackendType getExecutionBackend(const std::string& vmId) = 0;

    // ===== Display/Framebuffer =====

    /**
     * @brief Get display information for a VM
     * @param vmId VM ID
     * @param display Display number
     * @return Display information
     */
    virtual DisplayInfo getDisplayInfo(const std::string& vmId, int display) = 0;

    // ===== Progress Operations =====

    /**
     * @brief Cancel a running progress operation
     * @param progressId Progress ID
     */
    virtual void cancelProgress(int progressId) = 0;

    /**
     * @brief Wait for a progress operation to complete
     * @param progressId Progress ID
     * @return true if completed successfully
     */
    virtual bool waitForProgress(int progressId) = 0;

    // ===== Event Handling =====

    /**
     * @brief Register an event callback
     * @param callback Event callback function
     * @return Callback ID
     */
    virtual int registerEventCallback(FrontendEventCallback callback) = 0;

    /**
     * @brief Unregister an event callback
     * @param callbackId Callback ID
     */
    virtual void unregisterEventCallback(int callbackId) = 0;

    // ===== Guest Control =====

    /**
     * @brief Execute a program in the guest
     * @param vmId VM ID
     * @param executable Path to executable
     * @param args Arguments
     * @param env Environment variables
     * @return Exit code
     */
    virtual int guestExecute(const std::string& vmId, const std::string& executable,
                             const std::vector<std::string>& args,
                             const std::map<std::string, std::string>& env) = 0;

    // ===== Shared Folders =====

    /**
     * @brief Create a shared folder
     * @param vmId VM ID
     * @param name Share name
     * @param hostPath Host path
     * @param writable Writable flag
     */
    virtual void createSharedFolder(const std::string& vmId, const std::string& name,
                                     const std::string& hostPath, bool writable) = 0;

    /**
     * @brief Remove a shared folder
     * @param vmId VM ID
     * @param name Share name
     */
    virtual void removeSharedFolder(const std::string& vmId, const std::string& name) = 0;

    // ===== Remote Desktop (VRDP) =====

    /**
     * @brief Enable or disable VRDP
     * @param vmId VM ID
     * @param enabled Enable flag
     * @param port Port number (0 for auto)
     */
    virtual void setVRDPEnabled(const std::string& vmId, bool enabled, int port) = 0;

    // ===== Statistics =====

    /**
     * @brief Get performance statistics for a VM
     * @param vmId VM ID
     * @param names Statistics names
     * @return Statistics values
     */
    virtual std::map<std::string, double> getPerformanceStatistics(
        const std::string& vmId, const std::vector<std::string>& names) = 0;
};

/**
 * @brief Factory function to create a frontend instance
 * @param backendType Backend type hint (may be ignored)
 * @return Frontend instance
 */
std::unique_ptr<IVBoxFrontend> CreateVBoxFrontend(ExecutionBackendType backendType = ExecutionBackendType::TCG_JIT);

#endif /* IVBOXFRONTEND_H */
