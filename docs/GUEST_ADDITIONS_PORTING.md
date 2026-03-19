# Guest Additions Porting Guide

> This guide provides instructions for porting LibreVMM Guest Additions to new guest operating systems using the minimal ABI, including virtio and SPICE driver porting.

## Overview

LibreVMM Guest Additions provide enhanced functionality for guest operating systems, including:

- **Display driver** - Graphics acceleration and resolution changes
- **Mouse integration** - Seamless pointer capture/release
- **Shared folders** - File sharing between host and guest
- **Clipboard** - Bidirectional clipboard sharing
- **Time synchronization** - Guest clock sync with host
- **Drag and drop** - File transfer between host and guest
- **virtio drivers** - High-performance paravirtualized devices
- **SPICE guest drivers** - Enhanced remote display capabilities

---

## Architecture

### Minimal Guest Additions ABI

The minimal ABI allows Guest Additions to work on legacy operating systems with limited driver support. It requires only:

1. **Communication channel** - Shared memory or I/O port communication
2. **Basic driver interface** - Loadable driver with initialization
3. **Heartbeat** - Simple liveness check

### Component Structure

```
Guest Additions/
├── core/           # Core communication library
├── display/        # Graphics drivers
├── mouse/          # Pointing device integration
├── sharedfolders/  # Host-guest file sharing
├── clipboard/     # Clipboard integration
├── timesync/      # Time synchronization
├── draganddrop/   # Drag and drop support
├── virtio/        # virtio drivers
└── spice/         # SPICE guest agents
```

---

## Porting Process

### Phase 1: Assessment

Before starting, evaluate:

1. **Target OS capabilities**
   - What driver model does it use?
   - Does it support loadable drivers?
   - What development tools are available?

2. **Required components**
   - Which Guest Additions features are needed?
   - Is full ABI or minimal ABI appropriate?

3. **Driver dependencies**
   - What kernel/DDK version needed?
   - Any hardware requirements?

### Phase 2: Minimal ABI Implementation

For operating systems with limited capabilities, implement the minimal ABI first:

#### Step 1: Create Communication Channel

Choose one based on target OS capabilities:

**I/O Port Communication (simplest):**
```c
// Example: Simple I/O port communication
#define VBOX_GUEST_PORT 0x5010

uint32_t vbox_read_uint32(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "dN"(port));
    return value;
}

void vbox_write_uint32(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "dN"(port));
}
```

**Shared Memory Communication:**
```c
// Create shared memory region
void *vbox_create_shm(const char *name, size_t size) {
    // Platform-specific implementation
    // Returns pointer to shared memory or NULL on failure
}
```

#### Step 2: Implement Heartbeat

The heartbeat provides basic liveness detection:

```c
typedef struct VBoxGuestHeartbeat {
    uint32_t magic;           // VBOX_GUEST_MAGIC
    uint32_t version;         // Protocol version
    uint64_t timestamp;       // Last update time
    uint32_t flags;           // Capability flags
} VBoxGuestHeartbeat;

#define VBOX_GUEST_MAGIC 0x56424F58  // "VBX"
```

#### Step 3: Basic Driver Entry Point

```c
// Minimal driver initialization
int vbox_guest_init(void) {
    // Verify communication channel
    if (!vbox_check_channel()) {
        return -1;
    }
    
    // Initialize heartbeat
    vbox_heartbeat_init();
    
    // Report capabilities
    vbox_report_caps(VBOX_GUEST_CAP_HEARTBEAT);
    
    return 0;
}
```

### Phase 3: Display Driver Porting

#### Windows Display Driver

For Windows guests, implement a display driver:

1. **Create INF file** for driver installation
2. **Implement Miniport driver** using DDK
3. **Handle HGM (Hardware Graphics Manager)** calls

```c
// Windows display driver skeleton
#include <dderror.h>
#include <devioctl.h>
#include <miniport.h>

VP_STATUS NTAPI
VBoxMiniportQueryAdapterInfo(
    PVOIDHwDeviceExtension,
    PQUERY_ADAPTER_INFORMATIONQueryInfo
);

VP_STATUS NTAPI
VBoxMiniportStartIO(
    PVOIDHwDeviceExtension,
    PVIDEO_REQUEST_PACKETVideoRequestPacket
);

// ... additional Miniport functions
```

#### Linux Display Driver

For Linux guests, implement a DRM/KMS driver:

```c
// Linux DRM driver skeleton
#include <drm/drm_device.h>
#include <drm/drm_driver.h>

static struct drm_driver vbox_driver = {
    .name = "vboxvideo",
    .desc = "VirtualBox Graphics Adapter",
    .major = 1,
    .minor = 0,
    .driver_features = DRIVER_MODESET,
    .fops = &vbox_fops,
    .dumb_create = vbox_dumb_create,
    // ... additional callbacks
};

static int __init vbox_init(void) {
    return drm_platform_init(&vbox_driver);
}
```

### Phase 4: Mouse Integration

#### Absolute Pointer (USB)

```c
// Absolute mouse reporting
typedef struct VBoxMouseEvent {
    uint8_t  report_id;
    uint8_t  status;
    int16_t  x;
    int16_t  y;
    int8_t   z;  // wheel
} VBoxMouseEvent;

void vbox_mouse_report(int x, int y, int z, uint8_t buttons) {
    VBoxMouseEvent event = {
        .report_id = 1,
        .status = 0,
        .x = x,
        .y = y,
        .z = z
    };
    vbox_send_event(VBOX_EVENT_MOUSE, &event, sizeof(event));
}
```

#### Relative Pointer (PS/2)

```c
// PS/2 mouse relative movement
void vbox_ps2_mouse_report(int dx, int dy, int dz, uint8_t buttons) {
    uint8_t packet[3];
    packet[0] = 0x08 | ((dx < 0) ? 0x10 : 0) | ((dy < 0) ? 0x20 : 0) | 
                (buttons & 0x07);
    packet[1] = (uint8_t)dx;
    packet[2] = (uint8_t)dy;
    vbox_send_ps2(packet, 3);
}
```

---

## virtio Driver Porting

LibreVMM bundles virtio drivers from the Linux KVM project (`vendor/kvm-guest-drivers-windows/`).

### virtio-blk (Block Device)

```c
// virtio-blk driver port
#include <virtio.h>

typedef struct virtio_blk_config {
    uint64_t capacity;
    uint32_t size_max;
    uint32_t seg_max;
    uint16_t cylinders;
    uint8_t heads;
    uint8_t sectors;
    uint32_t blk_size;
    // ... additional fields
} virtio_blk_config;

typedef struct virtio_blk_req {
    uint32_t type;      // VIRTIO_BLK_T_IN, OUT, GET_ID
    uint32_t reserved;
    uint64_t sector;
    uint8_t data[];
    uint8_t status;
} virtio_blk_req;

// Initialize virtio-blk
int vbox_virtio_blk_init(void *bar0) {
    // Discover virtio capability
    // Configure PCI MSI-X
    // Setup virtqueue
    
    return 0;
}
```

### virtio-net (Network)

```c
// virtio-net driver port
#include <virtio.h>

typedef struct virtio_net_config {
    uint8_t  mac[6];
    uint16_t status;
    uint16_t max_virtqueue_pairs;
    uint16_t mtu;
    // ... additional fields
} virtio_net_config;

// Network packet transmission
int vbox_virtio_net_xmit(struct sk_buff *skb) {
    // Add header descriptor
    // Add data descriptors
    // Add status descriptor
    // Notify device
    
    return 0;
}
```

### virtio-scsi (SCSI)

```c
// virtio-scsi driver port
#include <virtio.h>

typedef struct virtio_scsi_config {
    uint32_t num_queues;
    uint32_t seg_max;
    uint32_t max_sectors;
    uint32_t cmd_per_lun;
    uint32_t event_info_size;
    uint32_t sense_size;
    uint32_t cdb_size;
    uint16_t max_channel;
    uint16_t max_target;
    uint32_t max_lun;
} virtio_scsi_config;
```

---

## SPICE Guest Driver Porting

SPICE provides enhanced remote display capabilities.

### SPICE QXL Driver

```c
// SPICE QXL display driver
#include <spice/qxl_dev.h>

typedef struct QXLDevice {
    uint32_t id;
    uint64_t ram_size;
    uint32_t vram_size;
    uint32_t surface_count;
    QXLInterface *spice_interface;
} QXLDevice;

// QXL command processing
int qxl_process_command(QXLDevice *dev, QXLCommand *cmd) {
    switch (cmd->type) {
        case QXL_CMD_DRAW:
            return qxl_handle_draw(dev, cmd->u.draw);
        case QXL_CMD_UPDATE:
            return qxl_handle_update(dev, cmd->u.update);
        case QXL_CMD_CURSOR:
            return qxl_handle_cursor(dev, cmd->u.cursor);
        default:
            return -1;
    }
}
```

### SPICE Guest Agent

```c
// SPICE vdagent for Linux
#include <spice/vd_agent.h>

typedef struct VDAgentMessage {
    uint32_t type;
    uint32_t size;
    uint8_t  data[];
} VDAgentMessage;

// Handle clipboard
int vdagent_handle_clipboard(VDAgentMessage *msg) {
    switch (msg->type) {
        case VD_AGENT_CLIPBOARD:
            // Process clipboard data
            return vdagent_clipboard_copy(msg->data, msg->size);
        case VD_AGENT_CLIPBOARD_REQUEST:
            // Handle clipboard request
            return vdagent_clipboard_request(msg->data);
        default:
            return -1;
    }
}
```

---

## Minimal Guest Additions ABI Specification

For old OS builds that cannot implement full Guest Additions, the minimal ABI provides basic functionality:

### Required Structures

```c
// Minimal ABI version 1.0
#define VBOX_GUEST_ABI_VERSION 0x0100

// Magic number for communication
#define VBOX_GUEST_MAGIC 0x56424F58  // "VBX"

// Capabilities (minimal set)
#define VBOX_GUEST_CAP_HEARTBEAT    0x0001
#define VBOX_GUEST_CAP_DISPLAY      0x0002
#define VBOX_GUEST_CAP_MOUSE        0x0004
#define VBOX_GUEST_CAP_SHAREDFOLDERS 0x0008
#define VBOX_GUEST_CAP_CLIPBOARD    0x0010

// Communication structures
typedef struct VBoxGuestInfo {
    uint32_t version;
    uint32_t capabilities;
    uint64_t os_type;
} VBoxGuestInfo;

typedef struct VBoxGuestHeartbeat {
    uint32_t magic;
    uint32_t version;
    uint64_t last_update;
    uint32_t capabilities;
} VBoxGuestHeartbeat;
```

### Protocol

1. Guest writes `VBoxGuestInfo` to communication channel
2. Host acknowledges with capability bitmap
3. Guest enters normal operation, sending periodic heartbeats

---

## Build Instructions

### Windows (DDK)

```bash
# Build with Windows DDK
cd guestadditions/windows
build -cZ
```

### Linux (Kernel Module)

```bash
# Build Linux kernel module
cd guestadditions/linux/sharedfolders
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
```

### Testing

1. **Unit tests:** Test each component independently
2. **Integration tests:** Test in VM with minimal Guest Additions
3. **Compatibility tests:** Test on target OS versions

---

## Troubleshooting

### Guest Additions Not Loading

1. Check kernel/driver compatibility
2. Verify communication channel (I/O port or shared memory)
3. Enable debug logging

### Performance Issues

1. Use virtio drivers when available
2. Enable 2D/3D acceleration if supported
3. Allocate sufficient video memory

### Driver Signing (Windows)

1. For test signing: `bcdedit /set testsigning on`
2. For production: Obtain code signing certificate

---

## References

- [Priority 3 — Guest Additions Expansion](../TODO.md#priority-3--guest-additions-expansion)
- [virtio Specification](https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.html)
- [SPICE Protocol](https://www.spice-space.org/documentation.html)
- [`vendor/kvm-guest-drivers-windows/`](../vendor/kvm-guest-drivers-windows/)
