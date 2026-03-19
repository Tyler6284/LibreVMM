# LibreVMM (formerly VirtualBox Extended Edition) — Project TODO

> **Guiding Principles:**
> - Full modularity: every subsystem should be replaceable or disableable independently.
> - Total user sovereignty: no silent overrides, no forced defaults, no hidden compatibility guards.
> - Platform agnosticism: the core must never assume a specific host OS, host architecture, or privilege level.
> - Additive, never destructive: all upstream VirtualBox functionality is preserved as a baseline.

---

## Repository Map

> All source repositories live under `Cloned-Repos/`. This section maps each repo to its role in the project. Repos marked **[ACTIVE]** are direct build inputs. Repos marked **[REFERENCE]** are consulted for design or driver model information. Repos marked **[UNRELATED]** are present in the Code folder but are not part of this project.

| Repo | Role | Notes |
|---|---|---|
| `Cloned-Repos/virtualbox/` | **[ACTIVE]** Core foundation | kBuild-based; `src/` is the main source tree |
| `Cloned-Repos/qemu/` | **[ACTIVE]** TCG engine + device import | Meson-based; `accel/tcg/`, `hw/i386/`, `target/i386/`, `audio/` are primary targets |
| `Cloned-Repos/86Box/` | **[ACTIVE]** Legacy device import | CMake + vcpkg; `src/` contains all device subsystems |
| `Cloned-Repos/Bochs/bochs/` | **[ACTIVE]** BIOS + device reference | Autoconf; `bios/` contains `BIOS-bochs-latest`, VGABIOS ROMs, and bundled SeaBIOS 1.13.0 binaries. Has existing `Copilot's Plan.txt`. |
| `Cloned-Repos/seabios/` | **[ACTIVE]** Firmware option | Kconfig/Makefile; `src/` is firmware source. Has existing `VirtualBox Plan.txt`. |
| `Cloned-Repos/openbios/` | **[ACTIVE]** Firmware option | Makefile/Kconfig; `arch/amd64/` is the x86 target |
| `Cloned-Repos/FEX/` | **[ACTIVE]** ARM64 Linux host translation | CMake; `CodeEmitter/` is the ARM64 code emitter. Linux-only. |
| `Cloned-Repos/kvm-guest-drivers-windows/` | **[ACTIVE]** virtio Windows guest drivers | Pre-built reference for virtio device Windows guest support (virtio-net, virtio-blk, virtio-scsi, etc.) |
| `Cloned-Repos/roms/` | **[ACTIVE]** ROM image library | 86Box ROM collection; `floppy/`, `hdd/` contain controller BIOSes for 86Box device emulation |
| `Cloned-Repos/SDL/` | **[ACTIVE]** SDL2 backend | Used by QEMU's SDL audio/video backends (`qemu/audio/sdlaudio.c`); also relevant for cross-platform display output |
| `Cloned-Repos/OpenGL-Registry/` | **[ACTIVE]** GL spec reference | OpenGL/OpenGL ES extension registry; reference for 3D acceleration device implementation |
| `Cloned-Repos/linux/` | **[REFERENCE]** KVM interface + Linux guest drivers | Full kernel source; primary reference for KVM ioctl interface and Linux guest driver model |
| `Cloned-Repos/reactos/` | **[REFERENCE]** NT driver model reference | Full ReactOS source; reference for NT 3.x/4.x driver model and Guest Additions porting |
| `Cloned-Repos/One-Core-API-Source/` | **[REFERENCE]** Legacy Windows API shim | ReactOS-derived WinXP compatibility layer; reference for Win9x/NT 3.x/NT 4.0 API surface when porting Guest Additions |
| `Cloned-Repos/ntvdmx64/` | **[REFERENCE]** 64-bit NTVDM | HAXM-backed DOS on 64-bit Windows; reference for DOS-mode execution in restricted environments |
| `Cloned-Repos/libvirt/` | **[REFERENCE]** VM management API | Meson-based; reference for headless/server-mode VM management API design (Priority 5) |
| `Cloned-Repos/virt-manager/` | **[REFERENCE]** GTK VM management UI | Python/GTK; reference for Linux desktop UI design and libvirt API usage |
| `Cloned-Repos/dosbox-x/` | **[REFERENCE]** DOS emulation reference | Reference for DOS guest support and legacy hardware emulation depth |
| `Cloned-Repos/vulkan/` | **[REFERENCE]** Vulkan spec/bindings | Python Vulkan bindings; reference for Vulkan host rendering path in 3D acceleration |
| `Cloned-Repos/docs/` | **[REFERENCE]** Documentation | General project documentation |
| `Cloned-Repos/binary-waterfall/` | **[UNRELATED]** Audio visualization tool | Python/Qt; not part of this project |
| `Cloned-Repos/DawVert/` | **[UNRELATED]** DAW converter | Not part of this project |
| `Cloned-Repos/tiny11builder/` | **[UNRELATED]** Windows 11 debloat | Not part of this project |

> **Note on existing plan files:**
> - `Cloned-Repos/seabios/VirtualBox Plan.txt` — Review and reconcile with this TODO before starting Priority 6 work on SeaBIOS integration.
> - `Cloned-Repos/Bochs/Copilot's Plan.txt` — Review and reconcile with this TODO before starting Priority 1 Bochs device import work.

---

## Table of Contents

1. [Priority 1 — Expanded Device Layer](#priority-1--expanded-device-layer)
2. [Priority 2 — Full Software Emulation Path](#priority-2--full-software-emulation-path)
3. [Priority 3 — Guest Additions Expansion](#priority-3--guest-additions-expansion)
4. [Priority 4 — Remove VM Execution Restrictions](#priority-4--remove-vm-execution-restrictions)
5. [Priority 5 — Platform-Agnostic UI](#priority-5--platform-agnostic-ui)
6. [Priority 6 — Firmware & BIOS Options](#priority-6--firmware--bios-options)
7. [Priority 7 — Build System Unification](#priority-7--build-system-unification)
8. [Priority 8 — License & Compliance Tracking](#priority-8--license--compliance-tracking)
9. [Priority 9 — Documentation & Developer Onboarding](#priority-9--documentation--developer-onboarding)
10. [Backlog / Under Consideration](#backlog--under-consideration)

---

## Priority 1 — Expanded Device Layer

> **Goal:** Every device from QEMU (i386/x86_64 targets only), Bochs, and 86Box becomes a selectable option in the VM device configuration, alongside existing VirtualBox devices. No device should be hardcoded as mandatory.

### 1.1 — Device Abstraction Layer (DAL)
- [ ] Design a unified `IVBoxExtDevice` interface that all imported devices (QEMU, Bochs, 86Box, native VBox) must implement.
- [ ] Define standard bus attachment points: ISA, PCI, PCIe, LPC, USB, AGP (legacy), MCA.
- [ ] Define standard IRQ, DMA, MMIO, and port I/O registration APIs that are host-agnostic.
- [ ] Create a device registry/catalog system so devices are discovered at runtime, not compiled in. Device modules should load from a configurable `VBOX_EXT_DEVICES_DIR`.
- [ ] Implement a device capability flags system (e.g., `DEVICE_CAP_3D`, `DEVICE_CAP_LEGACY_ISA`, `DEVICE_CAP_USB3`, `DEVICE_CAP_MCA`) for UI filtering and runtime validation.
- [ ] Ensure device combinations can be validated without being enforced — warn, never auto-correct.

### 1.2 — QEMU Device Import (i386/x86_64 scope only)
> Source: `Cloned-Repos/qemu/hw/i386/`, `Cloned-Repos/qemu/target/i386/`

- [ ] Isolate and extract QEMU's `hw/i386/` platform devices as standalone DAL-compatible modules.
- [ ] Import CPU device models: `qemu32`, `qemu64`, and all named i386/x86_64 CPU types from `Cloned-Repos/qemu/target/i386/cpu.c`.
- [ ] Import QEMU chipsets: i440FX + PIIX3/PIIX4, Q35 + ICH9.
- [ ] Import QEMU ISA devices: PIT (i8254), PIC (i8259), RTC (MC146818), DMA (i8237).
- [ ] Import QEMU sound devices: AC97, Intel HD Audio (HDA), ES1370, SB16, Adlib/OPL2/OPL3, GUS.
- [ ] Import QEMU storage controllers: AHCI, IDE (PIIX), NVMe, LSI53C895A, MegaRAID SAS, virtio-blk, virtio-scsi.
- [ ] Import QEMU network devices: e1000, e1000e, virtio-net, RTL8139, NE2000, pcnet.
- [ ] Import QEMU USB controllers: UHCI, OHCI, EHCI, XHCI.
- [ ] Import QEMU input devices: PS/2 i8042, virtio-input, USB HID.
- [ ] Import QEMU serial/parallel: 16550A UART, parallel port.
- [ ] Import QEMU audio backends from `Cloned-Repos/qemu/audio/`: ALSA (`alsaaudio.c`), PulseAudio (`paaudio.c`), PipeWire (`pwaudio.c`), DirectSound (`dsoundaudio.c`), CoreAudio (`coreaudio.m`), SDL (`sdlaudio.c`), SPICE (`spiceaudio.c`), WAV capture (`wavaudio.c`/`wavcapture.c`) — wire these up as selectable host audio backends.
- [ ] For virtio Windows guest driver support, cross-reference `Cloned-Repos/kvm-guest-drivers-windows/` — these are the shipping Windows guest drivers for virtio-net, virtio-blk, virtio-scsi, etc. Ensure imported QEMU virtio devices are compatible with these drivers.
- [ ] **Video devices — see 1.4.**

### 1.3 — 86Box Device Import
> Source: `Cloned-Repos/86Box/src/`

- [ ] Import 86Box legacy ISA/PCI video cards (see 1.4).
- [ ] Import 86Box ISA sound devices from `Cloned-Repos/86Box/src/sound/`: Sound Blaster 1.0/2.0/Pro/16, WSS, MPU-401, Roland MT-32 (Munt), CMS/GameBlaster.
- [ ] Import 86Box ISA/MCA network devices from `Cloned-Repos/86Box/src/network/`: 3Com 3C503, NE1000, NE2000 ISA, IBM Token Ring.
- [ ] Import 86Box FDC implementations from `Cloned-Repos/86Box/src/floppy/` (NEC µPD765 and variants).
- [ ] Import 86Box IDE/MFM/RLL/ESDI storage controllers from `Cloned-Repos/86Box/src/disk/`.
- [ ] Import 86Box chipsets from `Cloned-Repos/86Box/src/chipset/`: the full set is already present (ALi, OPTi, SiS, VIA, UMC, Intel 420EX/4x0, NEAT, SCAT, CS8220/8230, Headland, Compaq 386, etc.).
- [ ] Import 86Box CPU models from `Cloned-Repos/86Box/src/cpu/`: 8088, 8086, 286, 386SX/DX, 486SX/DX, early Pentium variants, plus `arch_detect.c` for runtime CPU detection.
- [ ] Import 86Box PS/2 / MCA machine type support from `Cloned-Repos/86Box/src/mca.c`.
- [ ] Import 86Box joystick/gameport implementation from `Cloned-Repos/86Box/src/game/`.
- [ ] Import 86Box CD-ROM support from `Cloned-Repos/86Box/src/cdrom/` (includes image, VISO, Mitsumi, MKE implementations).
- [ ] Use `Cloned-Repos/roms/` as the ROM image source for all 86Box controller BIOS needs (floppy controller BIOSes in `roms/floppy/`, HDD controller BIOSes in `roms/hdd/`).

### 1.4 — Video Device Expansion (Critical Sub-Priority)
> Sources: `Cloned-Repos/qemu/hw/display/`, `Cloned-Repos/86Box/src/video/`, `Cloned-Repos/OpenGL-Registry/`, `Cloned-Repos/vulkan/`

- [ ] Design a video device abstraction layer with backend slots: `software-renderer`, `virgl` (QEMU), `VBoxSVGA`, `VMSVGA`, `legacy-vga`, `llvmpipe`.
- [ ] Import QEMU `virtio-gpu` with virgil3D (virgl) backend — provides OpenGL ES 2.0/3.x for modern guests without host GPU passthrough.
- [ ] Import QEMU `bochs-display` device (simple linear framebuffer for UEFI/BIOS guests).
- [ ] Import QEMU `VGA`, `VESA BIOS Extensions (VBE)`, and `cirrus-vga`.
- [ ] Import QEMU `vmware-svga` (SVGA II) as an alternative to VirtualBox's VMSVGA.
- [ ] Import QEMU `QXL` device for SPICE-capable guests.
- [ ] Import 86Box ISA/PCI video cards from `Cloned-Repos/86Box/src/video/`: MDA, CGA, Hercules, EGA, VGA, SVGA (ET4000, Trident TVGA, Cirrus CL-GD5428/5434), S3 Trio/Virge, ATI Mach8/Mach32/Mach64.
- [ ] Restore VirtualBox 3D acceleration (`VBoxSVGA` + VMSVGA3D) for older guest OS targets (Windows 9x, NT 3.x/4.x, early Linux) — see Priority 2.3.
- [ ] Implement a software Mesa/LLVMpipe fallback for 3D acceleration on hosts without GPU acceleration. Use `Cloned-Repos/OpenGL-Registry/` as the authoritative GL extension reference.
- [ ] Implement a 2D acceleration path (XVideo/DirectDraw/framebuffer blit) independent of 3D, for guests that only need 2D compositing.
- [ ] Ensure all video devices expose resolution, color depth, and VRAM amount as freely configurable fields — no silent clamping.
- [ ] Integrate VGABIOS ROMs from `Cloned-Repos/Bochs/bochs/bios/VGABIOS-lgpl/` (Cirrus, banshee, standard, debug variants) and `VGABIOS-elpin/` as selectable VGA BIOS options.

### 1.5 — Bochs Device Import
> Source: `Cloned-Repos/Bochs/bochs/`
> Note: Review `Cloned-Repos/Bochs/Copilot's Plan.txt` before starting this section.

- [ ] Import Bochs VGA/SVGA device (`bx_vgacore`) as a standalone DAL-compatible device option.
- [ ] Import Bochs NE2000 network implementation as a cross-check reference against the 86Box NE2000.
- [ ] Import Bochs floppy/IDE controller implementations from `Cloned-Repos/Bochs/bochs/` where they differ meaningfully from 86Box.
- [ ] Import Bochs SB16 emulation as a cross-check reference for 86Box SB16 correctness.
- [ ] Note: Bochs already bundles SeaBIOS 1.13.0 binaries in `Cloned-Repos/Bochs/bochs/bios/SeaBIOS/` — use these as a known-good baseline when integrating the full SeaBIOS build from `Cloned-Repos/seabios/`.

---

## Priority 2 — Full Software Emulation Path

> **Goal:** Hardware virtualization (VT-x/AMD-V) must be fully, cleanly disableable. The project must run correctly at full feature parity on hosts with no VT-x/AMD-V, restricted privilege levels (UWP, sandboxed environments), and non-x86-64 host architectures.

### 2.1 — TCG Integration (QEMU Tiny Code Generator)
> Source: `Cloned-Repos/qemu/accel/tcg/`, `Cloned-Repos/qemu/target/i386/`

- [ ] Extract `Cloned-Repos/qemu/accel/tcg/` and `Cloned-Repos/qemu/target/i386/` as a self-contained static library (`libtcg_x86`), stripping all QEMU main-loop, QOM, and non-TCG dependencies.
- [ ] Reference `Cloned-Repos/qemu/accel/stubs/tcg-stub.c` for the minimal stub interface — this defines the TCG/non-TCG boundary cleanly.
- [ ] Wire `libtcg_x86` into VirtualBox's VMM (`Cloned-Repos/virtualbox/src/VBox/VMM/`) as a third execution mode alongside `HM` (hardware) and the removed `RawMode`.
- [ ] Implement a clean `IVMMExecutionBackend` interface with implementations for: `HMBackend` (VT-x/AMD-V), `TCGBackend` (software JIT), `TCGInterpreterBackend` (no-JIT, for `W^X`-restricted environments like UWP). Note that QEMU already has a single-step interpreter path in `Cloned-Repos/qemu/accel/tcg/tcg-accel-ops-rr.c`.
- [ ] Ensure TCG backend has no dependency on executable memory allocation when built in interpreter mode (`VBOX_NO_EXEC_ALLOC` flag) — critical for UWP/Xbox One.
- [ ] Validate TCG guest execution against known-good VirtualBox HM output for x86_64 and i386 guests.
- [ ] Cross-reference `Cloned-Repos/linux/` KVM interface headers (`include/uapi/linux/kvm.h`) to understand the HM-to-KVM boundary on Linux hosts.

### 2.2 — FEX-Emu Integration (Non-x86 Linux Hosts)
> Source: `Cloned-Repos/FEX/`

- [ ] Evaluate FEX-Emu's `FEXCore` in `Cloned-Repos/FEX/` as a host-side x86 instruction translator for ARM64 Linux hosts.
- [ ] Study `Cloned-Repos/FEX/CodeEmitter/` (the ARM64 code emitter) to understand FEX's output interface — this is what bridges x86 guest instructions to ARM64 host code.
- [ ] Define a host capability detection routine: if host is ARM64 Linux and FEX is available, offer FEX as an alternative to TCG for the host emulation layer.
- [ ] Note: FEX is out of scope for Windows ARM hosts — TCG interpreter is the fallback there.

### 2.3 — Legacy 3D Acceleration Restoration
> Sources: `Cloned-Repos/virtualbox/src/VBox/Devices/Graphics/`, `Cloned-Repos/virtualbox/src/VBox/Additions/`

- [ ] Audit which VirtualBox source versions last supported 3D for Windows 9x / NT 4.0 / early Linux guests within `Cloned-Repos/virtualbox/`.
- [ ] Restore the `VBoxSVGA` 3D path for these legacy guests within `Cloned-Repos/virtualbox/src/VBox/Devices/Graphics/`, gated behind device selection (not host detection).
- [ ] Ensure the software Mesa/LLVMpipe fallback (from 1.4) covers 3D for legacy guests when the host has no suitable GPU.
- [ ] Add a legacy `VESA 2.0` accelerated blit path for guests that use non-standard framebuffer access patterns.

### 2.4 — Restricted Userspace / UWP / Xbox One Build Target
- [ ] Create a `VBOX_RESTRICTED_USERSPACE` compile-time flag that gates out all kernel driver dependencies across both `Cloned-Repos/virtualbox/` and the imported QEMU/86Box modules.
- [ ] Audit and replace all `VirtualAlloc(MEM_COMMIT | PAGE_EXECUTE_READWRITE)` calls behind this flag.
- [ ] Replace JIT code generation with TCG interpreter fallback when `VBOX_NO_EXEC_ALLOC` is defined.
- [ ] Audit Win32 API usage across `Cloned-Repos/virtualbox/src/` for UWP compatibility — replace or stub any APIs not in the UWP API surface.
- [ ] Note: `Cloned-Repos/Bochs/bochs/build/android/` provides a reference for building a C++ emulator core for a restricted mobile/embedded target — consult this when setting up the UWP and Android build targets.
- [ ] Create an Xbox One / Windows Mobile test build profile.
- [ ] Document RAM and storage constraints for these targets and set appropriate default VM configuration limits.

---

## Priority 3 — Guest Additions Expansion

> **Goal:** Guest Additions should be compiled and functional for the broadest possible range of guest operating systems, not just modern Windows NT and Linux.

### 3.1 — Legacy Windows Guest Additions
> Sources: `Cloned-Repos/virtualbox/src/VBox/Additions/`, `Cloned-Repos/reactos/`, `Cloned-Repos/One-Core-API-Source/`

- [ ] Port Guest Additions to Windows NT 3.1 / 3.5 / 3.51 — use `Cloned-Repos/reactos/` as the NT 3.x driver model reference (Win32s subsystem, no NDIS5, very limited DDK). Cross-reference `Cloned-Repos/One-Core-API-Source/` for the API surface available at this level.
- [ ] Port Guest Additions to Windows NT 4.0 (NDIS4, DDK 4.0 display drivers) — `Cloned-Repos/reactos/` is the primary driver model reference.
- [ ] Port Guest Additions to Windows 9x (Win9x VxD model, VMM32, no NT kernel) — `Cloned-Repos/One-Core-API-Source/` provides WinXP-era shim layer reference; consult its VxD-to-NT bridging for API compatibility patterns.
- [ ] Port Guest Additions to Windows Me (same VxD model as 9x with minor differences).
- [ ] Port Guest Additions to Windows 2000 (closest to current NT path, lowest-effort legacy port).
- [ ] Ensure SVGA/3D acceleration Guest Additions driver is functional on Windows 9x with the restored VBoxSVGA path from Priority 2.3.
- [ ] Provide a minimal "display only" Guest Additions installer for guests where full integration is not possible.

### 3.2 — Legacy and Alternative Linux/Unix Guest Additions
> Sources: `Cloned-Repos/virtualbox/src/VBox/Additions/`, `Cloned-Repos/linux/`

- [ ] Test and repair Guest Additions for Linux kernel versions 2.4.x and 2.6.x — use `Cloned-Repos/linux/` as the driver API reference for these kernel generations.
- [ ] Port or adapt Guest Additions for FreeBSD, NetBSD, OpenBSD.
- [ ] Port Guest Additions for OS/2 (eComStation / ArcaOS) — basic display and shared folders.
- [ ] Evaluate feasibility of minimal Guest Additions for DOS — consider `Cloned-Repos/ntvdmx64/` as a reference for DOS execution environment constraints (VESA display, shared folder via INT, mouse integration via PS/2).
- [ ] Evaluate ReactOS Guest Additions using `Cloned-Repos/reactos/` as the build/test environment — bring to parity with NT 4.0 level.

### 3.3 — Guest Additions Architecture
- [ ] Decouple Guest Additions components so each (display driver, mouse, shared folders, clipboard, drag-and-drop, time sync) can be installed independently.
- [ ] Define a minimal Guest Additions ABI that old OS builds can implement with fewer dependencies.
- [ ] Provide source-level documentation of the Guest Additions protocol so third parties can implement it for unsupported guests.

---

## Priority 4 — Remove VM Execution Restrictions

> **Goal:** VirtualBox Extended Edition never silently modifies, overrides, or corrects user configuration. All guardrails become warnings. The user is always in full control.

### 4.1 — Configuration Enforcement Removal
> Source: `Cloned-Repos/virtualbox/src/VBox/Main/`, `Cloned-Repos/virtualbox/src/VBox/Frontends/`

- [ ] Audit all locations in `Cloned-Repos/virtualbox/src/VBox/Main/` and `Cloned-Repos/virtualbox/src/VBox/Frontends/` where the GUI or API silently overrides a user-selected setting — catalog every instance.
- [ ] Convert every silent override to a non-blocking warning dialog that requires explicit user acknowledgment but does not prevent the configuration from being saved.
- [ ] Remove any OS-type presets that forcibly change device selections when a guest OS type is chosen. OS type selection should only populate *defaults*, never enforce constraints.
- [ ] Remove the restriction that prevents incompatible video device + OS type combinations. Display a warning, proceed regardless.
- [ ] Remove any artificial limits on VRAM size, RAM allocation, CPU count, or disk size beyond what the host physically cannot provide.
- [ ] Remove automatic disabling of features (e.g., nested virtualization, PAE, NX) based on guest OS type — these should be user-controlled exclusively.

### 4.2 — Execution Guardrail Removal
> Source: `Cloned-Repos/virtualbox/src/VBox/VMM/`

- [ ] Remove forced fallback from HM to TCG without user consent — if HM fails, report the failure and halt, do not silently downgrade.
- [ ] Remove any guest execution watchdog that resets or pauses the VM without user instruction.
- [ ] Make all VM execution timeouts configurable, including setting them to zero (disabled).
- [ ] Expose raw MSR read/write capability as a VM option (currently heavily restricted in `Cloned-Repos/virtualbox/src/VBox/VMM/`).
- [ ] Allow user-defined CPU feature flags to be set or cleared without OS-type validation.

### 4.3 — Device Configuration Freedom
- [ ] Allow any device to be attached to any bus, even if the combination is historically unusual — warn, do not block.
- [ ] Allow multiple instances of the same device type where technically possible (e.g., multiple VGA devices).
- [ ] Allow emulated PCI IDs to be overridden per device instance for passthrough spoofing and compatibility testing.
- [ ] Allow manual IRQ, DMA channel, and port I/O range assignment for ISA devices (currently auto-assigned only).

---

## Priority 5 — Platform-Agnostic UI

> **Goal:** The UI frontend is a thin, replaceable layer over a stable backend API. Multiple UI implementations can coexist and target different platforms without changes to the core.

### 5.1 — Backend API (UI-Agnostic Core)
> Source: `Cloned-Repos/virtualbox/src/VBox/Main/`
> Reference: `Cloned-Repos/libvirt/` (for alternative backend API design patterns)

- [ ] Define a complete, stable `IVBoxFrontend` API that exposes all VM management, device configuration, snapshot, and execution control operations.
- [ ] Ensure the existing VirtualBox Main API in `Cloned-Repos/virtualbox/src/VBox/Main/` (`VBoxManage` / COM / XPCOM) either *is* this API or is a direct implementation of it.
- [ ] Consult `Cloned-Repos/libvirt/` as a design reference for how a mature, stable, multi-frontend VM management API is structured — particularly its XML configuration model and driver abstraction layer. Do **not** take a runtime dependency on libvirt; use it as design reference only.
- [ ] All UI implementations must use only this API — no direct access to VMM or device layer from UI code.
- [ ] Provide a CLI implementation (`vboxext-manage`) as the reference frontend and regression baseline.

### 5.2 — Qt Frontend (Primary Desktop UI)
> Source: `Cloned-Repos/virtualbox/src/VBox/Frontends/`

- [ ] Preserve the existing Qt-based VirtualBox UI from `Cloned-Repos/virtualbox/src/VBox/Frontends/` as the primary desktop frontend.
- [ ] Refactor Qt frontend to fully use the `IVBoxFrontend` API (remove any direct core access that may exist).
- [ ] Extend Qt UI with device selection panels for the new QEMU/86Box/Bochs device catalog.
- [ ] Add a "raw configuration" mode to the Qt UI that exposes all settings without OS-type filtering.
- [ ] Ensure Qt UI builds on Windows (x86, ARM64), Linux (x86_64, ARM64), and macOS (x86_64, Apple Silicon). Use `Cloned-Repos/SDL/` for any supplemental display output needs where SDL is more practical than a native Qt surface.
- [ ] Consult `Cloned-Repos/virt-manager/` as a reference for VM UI layout patterns if a native GTK frontend for Linux is also desired.

### 5.3 — XUL / WinRT Frontend (UWP / Windows Mobile)
- [ ] Design a minimal XUL or XAML/WinRT UI that implements the `IVBoxFrontend` API.
- [ ] Scope: VM creation wizard, basic device configuration, start/stop/pause, display output, snapshot list.
- [ ] Ensure this frontend builds within the UWP sandbox (no elevated permissions assumed).
- [ ] Use platform-native display output (CoreWindow / SwapChain) for the guest framebuffer.

### 5.4 — Java / Android Frontend
> Reference: `Cloned-Repos/Bochs/bochs/build/android/` (existing Android build of a C++ emulator core)

- [ ] Study `Cloned-Repos/Bochs/bochs/build/android/` for the pattern of wrapping a C++ emulator core in an Android APK — this is the closest existing reference in the repo for this work.
- [ ] Design a Java (Android SDK) frontend implementing the `IVBoxFrontend` API via JNI bridge to the native core.
- [ ] Scope: same as XUL frontend — creation, configuration, execution, display.
- [ ] Use Android SurfaceView or TextureView for guest framebuffer output.
- [ ] Handle ARM64 host (TCG only) and x86_64 host (TCG + potential HM via KVM) paths transparently.

### 5.5 — Web / Electron Frontend (Optional, Lower Priority)
- [ ] Evaluate a browser-based frontend using the `IVBoxFrontend` API over a local WebSocket/REST bridge.
- [ ] This enables remote VM management and potential thin-client display (VNC/SPICE fallback).

### 5.6 — Trade Dress & Visual Identity

> **Legal context:** Under *Lotus Development Corp. v. Borland International* (1st Cir. 1995, aff'd by Supreme Court), UI button placement, field layout, wizard step order, and menu hierarchy are classified as a **"method of operation"** and are not protectable copyright. The *Oracle America, Inc. v. Google LLC* precedent further supports that the logical/structural organization of an interface is not owned by Oracle. This means the functional UI layout — identical to VirtualBox 6.x or 5.2 — is legally defensible to preserve for user familiarity. However, the **visual presentation** ("Trade Dress") must be clearly distinct to avoid consumer confusion claims.

- [ ] **Hard requirement — functional parity:** Preserve button placement, field positions, wizard step order (Name → RAM → Disk, etc.), and menu hierarchy from the upstream VirtualBox Qt UI. This layout is the user's "accrued talent" and is legally a method of operation, not copyrightable trade dress. Targeting VirtualBox 5.2 UI structure is an acceptable alternative baseline and further distances LibreVMM from Oracle's *current* trade dress.
- [ ] **Required — application icon:** Replace the VirtualBox cube logo with a distinct LibreVMM application icon before any public distribution. Source from human-made, appropriately licensed artwork only (see Priority 8 for Humanity Icon Theme licensing). No AI-generated imagery.
- [ ] **Required — color palette:** Create a global Qt Stylesheet (`.qss`) for LibreVMM that replaces all VirtualBox "Oracle Blue/Grey" colors (`#0F6EBD` range and associated greys) with a distinct palette. The stylesheet must override background colors, border styles, and accent colors while leaving every button and field at its original functional position. A palette derived from the Ubuntu Humanity "Aubergine/Orange" range, or a neutral community "Teal," achieves this distinction cleanly.
- [ ] **Required — typography:** Set a distinct UI font (e.g., Ubuntu Sans or Inter) in the global `.qss` to further differentiate the visual presentation from Oracle's default.
- [ ] **Required — branded icon audit and replacement:** Audit `src/VBox/Frontends/VirtualBox/images/` for any icon that contains the VirtualBox cube logo or the word "VirtualBox." Replace these before distribution. Generic functional icons (directional arrows, play/pause glyphs, hardware device icons) are covered by the GPLv3 and may be used as-is. (See also Priority 8 icon audit item.)
- [ ] **README and About dialog disclaimer:** Ensure the following text (or equivalent) appears in `README.md` and the About dialog at all times: *"LibreVMM is a fork of the VirtualBox® base package. VirtualBox is a registered trademark of Oracle Corporation. LibreVMM is not affiliated with, endorsed by, or sponsored by Oracle Corporation."*

---

## Priority 6 — Firmware & BIOS Options

> **Goal:** Users can select any supported firmware/BIOS per VM, independently of the chipset or device configuration.
> Note: Review `Cloned-Repos/seabios/VirtualBox Plan.txt` before starting this section — an integration plan already exists.

- [ ] Integrate **SeaBIOS** from `Cloned-Repos/seabios/src/` as a selectable legacy BIOS option. The existing `VirtualBox Plan.txt` in that repo defines intended integration steps — reconcile with this TODO before proceeding.
- [ ] Integrate **Bochs BIOS** (`Cloned-Repos/Bochs/bochs/bios/BIOS-bochs-latest` and `BIOS-bochs-legacy`) as selectable legacy BIOS options. The bundled `Bochs/bochs/bios/SeaBIOS/bios.bin-1.13.0` serves as a pre-built SeaBIOS baseline.
- [ ] Integrate **OpenBIOS** from `Cloned-Repos/openbios/` as a selectable legacy BIOS option, using the `arch/amd64/` target.
- [ ] Preserve existing **OVMF (UEFI)** support in `Cloned-Repos/virtualbox/`, extending it to work with the new chipset options (Q35, etc.).
- [ ] Allow completely custom BIOS ROM image loading from a user-provided file.
- [ ] Expose BIOS option ROM slots as configurable entries (for legacy ISA/PCI card BIOSes). Use the ROMs in `Cloned-Repos/roms/` as the device option ROM library.
- [ ] Ensure BIOS selection is independent of chipset selection — all BIOS images should attempt to work with any chipset, with incompatibilities noted as warnings only.
- [ ] Integrate the VGABIOS options from `Cloned-Repos/Bochs/bochs/bios/VGABIOS-lgpl/` and `VGABIOS-elpin/` as selectable VGA BIOS ROMs for legacy video devices.

---

## Priority 7 — Build System Unification

> **Goal:** A single build system entry point produces any combination of host platform, target platform, UI frontend, and feature set.

- [ ] Audit `Cloned-Repos/virtualbox/` kBuild system (primary: `Config.kmk`, `Makefile.kmk`, `Version.kmk`) and evaluate whether to extend kBuild or migrate the project-level wrapper to CMake, given that 86Box (CMake+vcpkg), QEMU (Meson), Bochs (autoconf), and FEX (CMake) all use different systems.
- [ ] Integrate `Cloned-Repos/qemu/` Meson-based build as a subproject producing `libtcg_x86` only — strip all non-i386 targets and non-TCG accelerators during this extraction.
- [ ] Integrate `Cloned-Repos/86Box/` CMake build (`CMakeLists.txt` + `CMakePresets.json`) as a subproject producing device modules only, using its existing `cmake/` toolchain files for cross-compilation.
- [ ] Integrate `Cloned-Repos/Bochs/bochs/` autoconf build as a subproject producing device modules and BIOS only.
- [ ] Integrate `Cloned-Repos/seabios/` Makefile/Kconfig build as a subproject producing BIOS binary only.
- [ ] Integrate `Cloned-Repos/openbios/` Makefile/Kconfig build as a subproject producing BIOS binary (amd64 target) only.
- [ ] Integrate `Cloned-Repos/SDL/` as a shared dependency for QEMU SDL backends and the platform UI layer.
- [ ] Integrate `Cloned-Repos/FEX/` CMake build as an optional subproject (Linux ARM64 hosts only), producing `libFEXCore` only.
- [ ] Create build profiles for: `desktop-full`, `desktop-nogui`, `uwp-restricted`, `android`, `headless-server`.
- [ ] Define a clear `VBOX_EXT_DEVICES_DIR` that device modules are compiled into and loaded from at runtime.
- [ ] Ensure all imported subprojects can be built without their own main executables (library-mode builds only).
- [ ] Create CI pipeline definitions for: Windows x86_64, Linux x86_64, Linux ARM64, Windows ARM64. Reference the existing CI configs in `Cloned-Repos/86Box/.ci/` (Jenkins + GitHub Actions) and `Cloned-Repos/qemu/.gitlab-ci.d/` for platform-specific dependency lists.
- [ ] Define a minimal build (core + TCG + one BIOS + no GUI) as a smoke-test target.

---

## Priority 8 — License & Compliance Tracking

> **Goal:** Every component's license is tracked, conflicts are identified early, and distribution is legally clean.

- [ ] Create a `LICENSES/` directory and populate it with the license text of every component. Key source files: `virtualbox/COPYING` (GPL2) + `virtualbox/COPYING.CDDL`, `qemu/COPYING` (GPL2) + `COPYING.LIB`, `86Box/COPYING` (GPL2), `Bochs/bochs/LICENSE` (LGPL2.1), `seabios/COPYING` + `COPYING.LESSER` (LGPL3+BSD), `openbios/COPYING` (GPL2), `FEX/LICENSE` (MIT), `SDL/` (zlib).
- [ ] Create a `COMPONENT_LICENSES.md` table mapping every source directory to its license, origin, and any known compatibility concerns.
- [ ] Resolve the VirtualBox CDDL + GPL2 split (see `Cloned-Repos/virtualbox/COPYING.CDDL`): maintain CDDL-licensed VBox components in a separate module boundary; never statically link GPL code into CDDL modules.
- [ ] Confirm Bochs LGPL2.1 (`Cloned-Repos/Bochs/bochs/LICENSE`) dynamic vs. static linking strategy.
- [ ] Confirm SeaBIOS LGPL3 (`Cloned-Repos/seabios/COPYING.LESSER`) compatibility with the distribution model.
- [ ] Audit `Cloned-Repos/86Box/src/` for embedded third-party code with separate licensing (some device emulations have specific attributions in source headers).
- [ ] Flag any QEMU device that pulls in GPL3 or proprietary dependencies and exclude or replace.
- [ ] Confirm `Cloned-Repos/kvm-guest-drivers-windows/LICENSE` allows redistribution of virtio driver binaries alongside this project.
- [ ] Confirm `Cloned-Repos/reactos/COPYING` and `Cloned-Repos/One-Core-API-Source/COPYING` (GPL2 + LGPL2.1) — code sourced from these repos used in Guest Additions must stay within that boundary.
- [ ] Set up a REUSE-compliant (SPDX header) policy for all new source files created by this project.

#### Oracle-Specific Legal Items

- [ ] **Oracle trademark compliance:** Never use the name "VirtualBox" or "Oracle VM VirtualBox" as the product name, application title, window title, installer name, or package name. All user-facing strings must use "LibreVMM." The exception is factual historical attribution (e.g., "forked from VirtualBox"). Cross-reference with Priority 5.6 for the mandatory disclaimer wording in README and About dialog.
- [ ] **PUEL scope boundary:** Confirm and document that the Oracle PUEL (*Personal Use and Evaluation License*) applies **only** to Oracle's proprietary Extension Pack binary blobs (e.g., `Oracle_VM_VirtualBox_Extension_Pack-*.vbox-extpack`). The GPL-licensed base package code that *implements* the Extension Pack loader (`src/VBox/Main/src-server/ExtPackManagerImpl.cpp`, `VBoxExtPackHelperApp.cpp`, etc.) is **not** subject to PUEL. Modifying or removing the loader, disabling Extension Pack signature verification, and removing PUEL license-agreement UI flows in the base package source are all legal modifications under GPLv3.
- [ ] **Proprietary Extension Pack content prohibition (hard line):** Never decompile, extract, incorporate, or redistribute code from Oracle's proprietary Extension Pack binaries (USB 3.0 `VBoxUSB`, VRDP server, disk encryption drivers, etc.). These are closed-source proprietary code. Replace their functionality exclusively with open-source equivalents: QEMU and 86Box USB/VRDP/storage implementations (see Priority 1). Document this boundary explicitly in `COMPONENT_LICENSES.md`.
- [ ] **Trade dress — legal basis:** Record in `ARCHITECTURE.md` (or a dedicated `LEGAL_NOTES.md`) the applicable precedents: (1) *Lotus Development Corp. v. Borland International* establishes UI layout/button placement as a non-copyrightable "method of operation"; (2) *Oracle America, Inc. v. Google LLC* establishes that the logical structure/organization of an interface is not protectable copyright. These precedents are the legal foundation for LibreVMM's functional UI parity strategy (Priority 5.6).
- [ ] **VirtualBox source icon audit:** Audit `src/VBox/Frontends/VirtualBox/images/` and flag any icon file that (a) depicts the VirtualBox cube logo, (b) contains the word "VirtualBox," or (c) is Oracle-specific branding. These flagged icons must be replaced before any public distribution. Generic functional icons (arrows, hardware device glyphs, play/pause status icons) are covered works under GPLv3 and may be used without replacement.
- [ ] **Humanity Icon Theme licensing:** If the Ubuntu Humanity Icon Theme is adopted as the replacement icon set, document it in `COMPONENT_LICENSES.md`. License: GPL v2. Compatibility: GPL v2 is compatible with this project's GPL v3 distribution model. Required actions: bundle `LICENSES/humanity-icons-GPL-2.0.txt`; add credit to the Ubuntu Artwork Team in `COMPONENT_LICENSES.md`. Note: Tango Desktop Project elements within the Humanity set are Public Domain. Note also: `src/VBox/Frontends/VirtualBox/images/` icons with the `virtualbox.svg` name from the Ubuntu source should be independently evaluated — confirm whether Ubuntu's version of that icon is visually distinct from Oracle's cube logo before use.

---

## Priority 9 — Documentation & Developer Onboarding

> **Goal:** Any competent C/C++ developer can understand the architecture and contribute to any subsystem without needing to ask basic questions.

- [ ] Write `ARCHITECTURE.md`: high-level diagram and description of every major subsystem and their interfaces, with explicit references to the source paths in `Cloned-Repos/`.
- [ ] Write `DEVICE_PORTING_GUIDE.md`: step-by-step guide to importing a device from `Cloned-Repos/qemu/`, `Cloned-Repos/86Box/`, or `Cloned-Repos/Bochs/` using the DAL.
- [ ] Write `BUILD_GUIDE.md`: complete build instructions for every supported host platform and build profile, covering the multi-build-system environment (kBuild, CMake, Meson, autoconf, Kconfig/Makefile) across all subprojects.
- [ ] Write `EXECUTION_BACKENDS.md`: describes HM, TCG JIT (`Cloned-Repos/qemu/accel/tcg/`), and TCG interpreter modes, when each is used, and how to extend them.
- [ ] Write `GUEST_ADDITIONS_PORTING.md`: guide for porting Guest Additions to a new guest OS using the minimal ABI, with NT driver model references to `Cloned-Repos/reactos/` and API compatibility references to `Cloned-Repos/One-Core-API-Source/`.
- [ ] Write `UI_FRONTEND_GUIDE.md`: guide for implementing a new UI frontend against the `IVBoxFrontend` API, with the Android reference in `Cloned-Repos/Bochs/bochs/build/android/` and the GTK reference in `Cloned-Repos/virt-manager/`.
- [ ] Maintain a `KNOWN_INCOMPATIBILITIES.md` that lists device/chipset/BIOS combinations that are known to misbehave and why — so users have a reference, not a restriction.
- [ ] Reconcile `Cloned-Repos/seabios/VirtualBox Plan.txt` and `Cloned-Repos/Bochs/Copilot's Plan.txt` into the canonical documentation — preserve any decisions already recorded in those files.
- [ ] Add inline documentation (Doxygen) to all new interfaces defined by this project.

---

## Backlog / Under Consideration

> Items that may belong in the project but need more evaluation before committing.

- [ ] **Storage format expansion**: Evaluate read/write support for QEMU's `qcow2` and raw `.img` formats natively (QEMU block layer is in `Cloned-Repos/qemu/block.c` and `blockdev.c`). Currently VirtualBox supports these in limited ways.
- [ ] **SPICE remote display**: Evaluate adding a SPICE server (from QEMU's `Cloned-Repos/qemu/audio/spiceaudio.c` and QXL display integration) to supplement VRDP, especially for the virgl/QXL 3D acceleration path.
- [ ] **Snapshot system review**: Ensure the snapshot system in `Cloned-Repos/virtualbox/src/VBox/Storage/` is not coupled to a specific execution backend.
- [ ] **Headless / server mode**: Ensure a fully headless build (no Qt, no GUI dependency) is a supported first-class target. Reference `Cloned-Repos/libvirt/` for headless management API patterns.
- [ ] **Machine type library**: Define a "machine type" system (similar to QEMU's `-machine` flag) where a complete hardware configuration is saved as a named template — separate from individual VM instances.
- [ ] **DOSBox-X integration evaluation**: `Cloned-Repos/dosbox-x/` is present. Evaluate whether any of its DOS-specific hardware emulation (particularly OPL2/3 and Sound Blaster implementations) offer value over the 86Box equivalents for DOS guest accuracy.
- [ ] **ntvdmx64 reference**: `Cloned-Repos/ntvdmx64/` demonstrates HAXM-backed 16-bit DOS execution on 64-bit Windows. Evaluate whether its approach to DOS-in-restricted-environment is applicable to the UWP target or DOS Guest Additions work.
- [ ] **Networking backend expansion**: Evaluate adding QEMU's `slirp` user-mode NAT (present in `Cloned-Repos/qemu/`) as an alternative to VirtualBox's existing NAT engine, particularly for the restricted userspace target where tap/bridge networking is unavailable.
- [ ] **SDL display output**: `Cloned-Repos/SDL/` is already present as a QEMU dependency. Evaluate using SDL as a unified display output layer for non-Qt desktop platforms or the headless build.

---

*Last updated: initial directory structure pass against `Cloned-Repos/`. Priorities and items should be revised as architecture decisions are finalized. Always verify the current state of a repo in `Cloned-Repos/` before starting a task — upstream repos may have changed.*
