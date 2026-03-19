# UI Frontend Guide

> This guide explains how to implement a new UI frontend against the `IVBoxFrontend` API, including frontend lifecycle management and device configuration.

## Overview

LibreVMM's UI is designed as a thin, replaceable layer over a stable backend API. Multiple UI implementations can coexist and target different platforms without changes to the core.

The `IVBoxFrontend` interface ([`include/IVBoxFrontend.h`](../include/IVBoxFrontend.h)) provides all VM management, device configuration, and execution control operations.

---

## Architecture

### Frontend Structure

```
LibreVMM Frontend
    │
    ├── IVBoxFrontend (stable API)
    │   ├── VM lifecycle management
    │   ├── Device configuration
    │   ├── Snapshot operations
    │   └── Execution control
    │
    └── FrontendBase (optional base class)
        ├── Session tracking
        ├── Event handling
        └── Convenience methods
```

### Available Frontend Types

| Type | Platform | Location |
|------|----------|----------|
| CLI | All | [`src/CLI/`](../src/CLI/) |
| Qt | Desktop (Windows/Linux/macOS) | [`src/QtFrontend/`](../src/QtFrontend/) |
| WinRT | UWP/Windows | [`src/WinRTFrontend/`](../src/WinRTFrontend/) |
| Java | JVM platforms | (Future) |
| Android | Android | (Future) |

---

## Implementing a New Frontend

### Step 1: Implement IVBoxFrontend

Your frontend must implement the `IVBoxFrontend` interface. The complete interface is defined in [`include/IVBoxFrontend.h`](../include/IVBoxFrontend.h).

#### Required Methods

```cpp
class IVBoxFrontend
{
public:
    virtual ~IVBoxFrontend() = default;

    // ===== VM Lifecycle Management =====
    virtual std::string createVM(const VMConfig& config) = 0;
    virtual std::string registerVM(const std::string& folder) = 0;
    virtual void unregisterVM(const std::string& vmId, bool deleteFiles) = 0;
    virtual std::string startVM(const std::string& vmId, const std::string& sessionType) = 0;
    virtual void powerOffVM(const std::string& vmId) = 0;
    virtual void saveVMState(const std::string& vmId) = 0;
    virtual void pauseVM(const std::string& vmId) = 0;
    virtual void resumeVM(const std::string& vmId) = 0;
    virtual VMPowerState getVMState(const std::string& vmId) = 0;
    
    // ===== VM Configuration =====
    virtual void setVMConfig(const std::string& vmId, const VMConfig& config) = 0;
    virtual VMConfig getVMConfig(const std::string& vmId) = 0;
    
    // ===== Storage =====
    virtual std::string createMedium(const std::string& format,
                                     const std::string& path,
                                     uint64_t size,
                                     const std::string& type) = 0;
    virtual void attachMedium(const std::string& vmId,
                              const std::string& controllerName,
                              int port, int device,
                              const std::string& mediumId) = 0;
    virtual void detachMedium(const std::string& vmId,
                              const std::string& controllerName,
                              int port, int device) = 0;
    virtual std::vector<MediumInfo> getMedia() = 0;
    
    // ===== Snapshots =====
    virtual std::string takeSnapshot(const std::string& vmId,
                                      const std::string& name,
                                      const std::string& description) = 0;
    virtual void restoreSnapshot(const std::string& vmId,
                                  const std::string& snapshotId) = 0;
    virtual void deleteSnapshot(const std::string& vmId,
                                 const std::string& snapshotId) = 0;
    virtual std::vector<SnapshotInfo> getSnapshots(const std::string& vmId) = 0;
    
    // ===== Runtime =====
    virtual VMRuntimeInfo getVMRuntimeInfo(const std::string& vmId) = 0;
    
    // ===== System =====
    virtual SystemProperties getSystemProperties() = 0;
    virtual std::vector<std::string> getOSTypes() = 0;
    virtual std::vector<VMDescriptor> listVMs() = 0;
    virtual std::string findVMByName(const std::string& name) = 0;
    
    // ===== Display =====
    virtual std::vector<DisplayInfo> getDisplayInfo(const std::string& vmId) = 0;
    virtual void setDisplayFramebuffer(const std::string& vmId,
                                      int displayNumber,
                                      const std::vector<uint8_t>& framebuffer) = 0;
    
    // ===== Events =====
    virtual void registerEventCallback(FrontendEventCallback callback) = 0;
    virtual void unregisterEventCallback(int callbackId) = 0;
};
```

### Step 2: Use FrontendBase (Optional)

For convenience, you can inherit from `FrontendBase` ([`src/FrontendBase/FrontendBase.h`](../src/FrontendBase/FrontendBase.h)) which provides:

- Session tracking
- Event handling infrastructure
- Convenience methods for common operations

```cpp
#include "FrontendBase/FrontendBase.h"

class MyFrontend : public FrontendBase
{
public:
    MyFrontend() : FrontendBase(FrontendType::Custom) {}
    
    bool initialize() override;
    void shutdown() override;
    
    // ... implement platform-specific UI
};
```

---

## Example: Minimal Frontend Implementation

This example shows a minimal console-based frontend:

```cpp
#include <IVBoxFrontend.h>
#include <memory>
#include <iostream>

class MinimalFrontend : public IVBoxFrontend
{
public:
    MinimalFrontend() = default;
    virtual ~MinimalFrontend() = default;
    
    // ===== IVBoxFrontend Implementation =====
    
    std::string createVM(const VMConfig& config) override {
        std::cout << "Creating VM: " << config.name << std::endl;
        // Implementation: call backend API to create VM
        return "generated-uuid";
    }
    
    std::string registerVM(const std::string& folder) override {
        // Register existing VM
        return "";
    }
    
    void unregisterVM(const std::string& vmId, bool deleteFiles) override {
        std::cout << "Unregistering VM: " << vmId << std::endl;
    }
    
    std::string startVM(const std::string& vmId, const std::string& sessionType) override {
        std::cout << "Starting VM: " << vmId << " (" << sessionType << ")" << std::endl;
        return "session-id";
    }
    
    void powerOffVM(const std::string& vmId) override {
        std::cout << "Powering off VM: " << vmId << std::endl;
    }
    
    void saveVMState(const std::string& vmId) override {
        std::cout << "Saving VM state: " << vmId << std::endl;
    }
    
    void pauseVM(const std::string& vmId) override {
        std::cout << "Pausing VM: " << vmId << std::endl;
    }
    
    void resumeVM(const std::string& vmId) override {
        std::cout << "Resuming VM: " << vmId << std::endl;
    }
    
    VMPowerState getVMState(const std::string& vmId) override {
        return VMPowerState::PoweredOff;
    }
    
    void setVMConfig(const std::string& vmId, const VMConfig& config) override {
        std::cout << "Setting VM config: " << vmId << std::endl;
    }
    
    VMConfig getVMConfig(const std::string& vmId) override {
        return VMConfig{};
    }
    
    // ... implement all other methods
    
private:
    // Private implementation details
};

// Usage
int main() {
    auto frontend = std::make_unique<MinimalFrontend>();
    
    VMConfig config;
    config.name = "TestVM";
    config.ostype = "Linux";
    config.hardware.memoryMB = 1024;
    config.hardware.cpuCount = 1;
    
    std::string vmId = frontend->createVM(config);
    frontend->startVM(vmId, "headless");
    
    return 0;
}
```

---

## Frontend Lifecycle Management

### Initialization

```cpp
class MyFrontend : public FrontendBase
{
public:
    bool initialize() override {
        // Step 1: Initialize backend connection
        if (!initBackend()) {
            return false;
        }
        
        // Step 2: Load configuration
        if (!loadConfig()) {
            return false;
        }
        
        // Step 3: Discover available VMs
        auto vms = getAllVMs();
        
        // Step 4: Register event handlers
        registerEventHandler([this](const FrontendInternalEventData& event) {
            handleEvent(event);
        });
        
        m_running = true;
        return true;
    }
    
    void shutdown() override {
        // Step 1: Stop all running VMs
        for (const auto& vm : getAllVMs()) {
            if (isVMRunning(vm.id)) {
                stopVM(vm.id);
            }
        }
        
        // Step 2: Unregister event handlers
        // Step 3: Save configuration
        // Step 4: Cleanup backend connection
        
        m_running = false;
    }
};
```

### Session Management

```cpp
// Track active sessions
void trackSession(const std::string& sessionId, 
                  const std::string& vmId,
                  const std::string& sessionType) {
    VMSessionInfo info;
    info.sessionId = sessionId;
    info.vmId = vmId;
    info.sessionType = sessionType;
    info.startTime = std::chrono::steady_clock::now();
    info.isActive = true;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessions[sessionId] = info;
}

// Clean up on shutdown
void cleanupSessions() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& pair : m_sessions) {
        if (pair.second.isActive) {
            // Force stop active session
            try {
                powerOffVM(pair.second.vmId);
            } catch (...) {
                // Ignore errors during cleanup
            }
        }
    }
    m_sessions.clear();
}
```

### Event Handling

```cpp
// Register for events
int handlerId = registerEventHandler([](const FrontendInternalEventData& event) {
    switch (event.event) {
        case FrontendInternalEvent::VMStarted:
            std::cout << "VM started: " << event.vmId << std::endl;
            break;
        case FrontendInternalEvent::VMStopped:
            std::cout << "VM stopped: " << event.vmId << std::endl;
            break;
        case FrontendInternalEvent::Error:
            std::cerr << "Error: " << event.message << std::endl;
            break;
    }
});

// Unregister when done
unregisterEventHandler(handlerId);
```

---

## Device Configuration Flow

### Creating a VM with Devices

```cpp
std::string createVMWithDevices(IVBoxFrontend* frontend,
                                const std::string& name,
                                const std::string& ostype) {
    // Step 1: Basic VM configuration
    VMConfig config;
    config.name = name;
    config.ostype = ostype;
    config.hardware.memoryMB = 2048;
    config.hardware.cpuCount = 2;
    config.hardware.vramMB = 128;
    config.hardware.hardwareVirt = true;
    config.hardware.ioAPIC = true;
    
    // Step 2: Configure storage controller
    StorageControllerConfig sata;
    sata.name = "SATA";
    sata.type = StorageControllerType::SATA;
    sata.bootable = true;
    config.storageControllers.push_back(sata);
    
    // Step 3: Configure network
    NetworkAdapterConfig nat;
    nat.name = "NAT";
    nat.type = NetworkAdapterType::NAT;
    nat.enabled = true;
    config.networkAdapters.push_back(nat);
    
    // Step 4: Configure audio
    AudioConfig audio;
    audio.enabled = true;
    audio.backend = AudioBackendType::PulseAudio;
    config.audioDevices.push_back(audio);
    
    // Step 5: Configure video
    VideoDeviceConfig video;
    video.id = 0;
    video.type = VideoDeviceType::VMSVGA;
    video.vramMB = 128;
    video.accelerate3D = true;
    config.videoDevices.push_back(video);
    
    // Step 6: Create VM
    return frontend->createVM(config);
}
```

### Attaching Devices

```cpp
void attachDevices(IVBoxFrontend* frontend, const std::string& vmId) {
    // Create disk image
    std::string diskId = frontend->createMedium(
        "VDI",              // format
        "/path/to/disk.vdi",  // path
        20 * 1024 * 1024,  // 20 GB
        "HardDisk"          // type
    );
    
    // Attach disk to SATA controller
    frontend->attachMedium(
        vmId,
        "SATA",    // controller name
        0,         // port
        0,         // device
        diskId     // medium ID
    );
    
    // Add bridged network adapter
    NetworkAdapterConfig bridged;
    bridged.name = "Bridged";
    bridged.type = NetworkAdapterType::Bridged;
    bridged.enabled = true;
    bridged.hostInterface = "eth0";
    
    frontend->addNetworkAdapter(vmId, bridged);
}
```

### Device Types Reference

From [`include/IVBoxFrontend.h`](../include/IVBoxFrontend.h):

```cpp
// Storage Controller Types
enum class StorageControllerType {
    IDE, SATA, SCSI, SAS, NVMe, Floppy, USB,
    Virtio_SCSI, Virtio_Block, LSI_SAS, MegaRAID
};

// Network Adapter Types
enum class NetworkAdapterType {
    NAT, Bridged, HostOnly, Internal, Generic, NATNetwork,
    Virtio_Net, E1000, E1000E, RTL8139, NE2000, PCNet
};

// Video Device Types
enum class VideoDeviceType {
    VBoxVGA, VBoxSVGA, VMSVGA, QXL, Virtio_GPU,
    VGA, BochsDisplay, VMwareSVGA, Cirrus, VMDevice
};

// USB Controller Types
enum class USBControllerType {
    OHCI, EHCI, XHCI, USB3, Virtio_USB
};

// Audio Backend Types
enum class AudioBackendType {
    Null, DirectSound, CoreAudio, ALSA, PulseAudio,
    PipeWire, SDL, WASAPI, OBS
};
```

---

## Platform-Specific Considerations

### Windows (Win32/WinRT)

- Use native Windows APIs for display output
- Handle window messages for UI events
- Support system tray integration

### Linux

- Use X11 or Wayland for display
- Support system D-Bus for IPC
- Integrate with system audio (PulseAudio/PipeWire)

### macOS

- Use Cocoa/AppKit for UI
- Use CoreAudio for audio
- Support Retina displays

### Mobile (Android/iOS)

- Use platform UI frameworks
- Handle lifecycle carefully (pause/resume)
- Use SurfaceView/TextureView for display

---

## Testing Your Frontend

### Unit Tests

```cpp
// Test VM creation
void testCreateVM() {
    MinimalFrontend frontend;
    VMConfig config;
    config.name = "TestVM";
    config.hardware.memoryMB = 1024;
    
    std::string vmId = frontend.createVM(config);
    assert(!vmId.empty());
}
```

### Integration Tests

```cpp
// Test full VM lifecycle
void testVMLifecycle() {
    MinimalFrontend frontend;
    
    // Create
    std::string vmId = frontend.createVM(config);
    
    // Start
    std::string sessionId = frontend.startVM(vmId, "headless");
    assert(frontend.getVMState(vmId) == VMPowerState::Running);
    
    // Pause
    frontend.pauseVM(vmId);
    assert(frontend.getVMState(vmId) == VMPowerState::Paused);
    
    // Resume
    frontend.resumeVM(vmId);
    
    // Stop
    frontend.powerOffVM(vmId);
    assert(frontend.getVMState(vmId) == VMPowerState::PoweredOff);
}
```

---

## Related Documentation

- [Priority 5 — Platform-Agnostic UI](../TODO.md#priority-5---platform-agnostic-ui)
- [Priority 5.1 — Backend API](../TODO.md#51---backend-api-ui-agnostic-core)
- [`include/IVBoxFrontend.h`](../include/IVBoxFrontend.h)
- [`src/FrontendBase/FrontendBase.h`](../src/FrontendBase/FrontendBase.h)
- [BUILD_GUIDE.md](./BUILD_GUIDE.md)
