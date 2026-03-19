# LibreVMM — Project TODO

> **Guiding Principles:**
> - Full modularity: every subsystem should be replaceable or disableable independently.
> - Total user sovereignty: no silent overrides, no forced defaults, no hidden compatibility guards.
> - Platform agnosticism: the core must never assume a specific host OS, host architecture, or privilege level.
> - Additive, never destructive: all upstream VirtualBox functionality is preserved as a baseline.

---

## Repository Map

> This section maps each upstream repository to its role in the project. All listed repos are direct build inputs to LibreVMM.

| Repo | Role | Build System |
|---|---|---|
| `/vendor/virtualbox/` | Core foundation | kBuild |
| `/vendor/qemu/` | TCG engine + device import | Meson |
| `/vendor/86Box/` | Legacy device import | CMake + vcpkg |
| `/vendor/Bochs/bochs/` | BIOS + device import | Autoconf |
| `/vendor/seabios/` | Firmware option | Kconfig/Makefile |
| `/vendor/openbios/` | Firmware option | Makefile/Kconfig |
| `/vendor/kvm-guest-drivers-windows/` | virtio Windows guest drivers | Windows DDK (Windows only) |
| `/vendor/86Box/roms/` | ROM image library | N/A |
| `/vendor/SDL/` | SDL2 backend | CMake |
| `/vendor/dosbox-x/` | DOS cycle scaling "cycles=auto" + legacy hardware import | Autotools/CMake |
| `/vendor/jdk/` | Java runtime | OpenJDK build system |
| `/vendor/VirtualBox-5.2.44/` | UI design to copy from | kBuild |
| `/vendor/VirtualBoxSDK-5.2.44-139111/` | VirtualBox 5.2 SDK | N/A |
| `/vendor/VirtualBoxSDK-7.2.6-172322/` | VirtualBox 7.2 SDK | N/A |

### These will not be committed to the Git Repository!
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

- [ ] Isolate and extract QEMU's `hw/i386/` platform devices as standalone DAL-compatible modules.
- [ ] Import CPU device models: `qemu32`, `qemu64`, and all named i386/x86_64 CPU types from qemu/target/i386/cpu.c`.
- [ ] Import QEMU chipsets: i440FX + PIIX3/PIIX4, Q35 + ICH9.
- [ ] Import QEMU ISA devices: PIT (i8254), PIC (i8259), RTC (MC146818), DMA (i8237).
- [ ] Import QEMU sound devices: AC97, Intel HD Audio (HDA), ES1370, SB16, Adlib/OPL2/OPL3, GUS.
- [ ] Import QEMU storage controllers: AHCI, IDE (PIIX), NVMe, LSI53C895A, MegaRAID SAS, virtio-blk, virtio-scsi.
- [ ] Import QEMU network devices: e1000, e1000e, virtio-net, RTL8139, NE2000, pcnet.
- [ ] Import QEMU USB controllers: UHCI, OHCI, EHCI, XHCI.
- [ ] Import QEMU input devices: PS/2 i8042, virtio-input, USB HID.
- [ ] Import QEMU serial/parallel: 16550A UART, parallel port.
- [ ] Import QEMU audio backends from `qemu/audio/`: ALSA (`alsaaudio.c`), PulseAudio (`paaudio.c`), PipeWire (`pwaudio.c`), DirectSound (`dsoundaudio.c`), CoreAudio (`coreaudio.m`), SDL (`sdlaudio.c`), SPICE (`spiceaudio.c`), WAV capture (`wavaudio.c`/`wavcapture.c`) — wire these up as selectable host audio backends.
- [ ] For virtio Windows guest driver support, cross-reference `kvm-guest-drivers-windows/` — these are the shipping Windows guest drivers for virtio-net, virtio-blk, virtio-scsi, etc. Ensure imported QEMU virtio devices are compatible with these drivers.
- [ ] **Video devices — see 1.4.**

### 1.3 — 86Box Device Import

- [ ] Import 86Box legacy ISA/PCI video cards (see 1.4).
- [ ] Import 86Box ISA sound devices from `86Box/src/sound/`: Sound Blaster 1.0/2.0/Pro/16, WSS, MPU-401, Roland MT-32 (Munt), CMS/GameBlaster.
- [ ] Import 86Box ISA/MCA network devices from `86Box/src/network/`: 3Com 3C503, NE1000, NE2000 ISA, IBM Token Ring.
- [ ] Import 86Box FDC implementations from `86Box/src/floppy/` (NEC µPD765 and variants).
- [ ] Import 86Box IDE/MFM/RLL/ESDI storage controllers from `86Box/src/disk/`.
- [ ] Import 86Box chipsets from `86Box/src/chipset/`: the full set is already present (ALi, OPTi, SiS, VIA, UMC, Intel 420EX/4x0, NEAT, SCAT, CS8220/8230, Headland, Compaq 386, etc.).
- [ ] Import 86Box CPU models from `86Box/src/cpu/`: 8088, 8086, 286, 386SX/DX, 486SX/DX, early Pentium variants, plus `arch_detect.c` for runtime CPU detection.
- [ ] Import 86Box PS/2 / MCA machine type support from 86Box/src/mca.c`.
- [ ] Import 86Box joystick/gameport implementation from `86Box/src/game/`.
- [ ] Import 86Box CD-ROM support from `86Box/src/cdrom/` (includes image, VISO, Mitsumi, MKE implementations).
- [ ] Use `roms/` as the ROM image source for all 86Box controller BIOS needs (floppy controller BIOSes in `roms/floppy/`, HDD controller BIOSes in `roms/hdd/`).

### 1.4 — Video Device Expansion (Critical Sub-Priority)

- [ ] Design a video device abstraction layer with backend slots: `software-renderer`, `virgl` (QEMU), `VBoxSVGA`, `VMSVGA`, `legacy-vga`, `llvmpipe`.
- [ ] Import QEMU `virtio-gpu` with virgil3D (virgl) backend — provides OpenGL ES 2.0/3.x for modern guests without host GPU passthrough.
- [ ] Import QEMU `bochs-display` device (simple linear framebuffer for UEFI/BIOS guests).
- [ ] Import QEMU `VGA`, `VESA BIOS Extensions (VBE)`, and `cirrus-vga`.
- [ ] Import QEMU `vmware-svga` (SVGA II) as an alternative to VirtualBox's VMSVGA.
- [ ] Import QEMU `QXL` device for SPICE-capable guests.
- [ ] Import 86Box ISA/PCI video cards from `86Box/src/video/`: MDA, CGA, Hercules, EGA, VGA, SVGA (ET4000, Trident TVGA, Cirrus CL-GD5428/5434), S3 Trio/Virge, ATI Mach8/Mach32/Mach64.
- [ ] Restore VirtualBox 3D acceleration (`VBoxSVGA` + VMSVGA3D) for older guest OS targets (Windows 9x, NT 3.x/4.x, early Linux) — see Priority 2.3.
- [ ] Implement a software Mesa/LLVMpipe fallback for 3D acceleration on hosts without GPU acceleration. Use `OpenGL-Registry/` as the authoritative GL extension reference.
- [ ] Implement a 2D acceleration path (XVideo/DirectDraw/framebuffer blit) independent of 3D, for guests that only need 2D compositing.
- [ ] Ensure all video devices expose resolution, color depth, and VRAM amount as freely configurable fields — no silent clamping.
- [ ] Integrate VGABIOS ROMs from `Bochs/bochs/bios/VGABIOS-lgpl/` (Cirrus, banshee, standard, debug variants) and `VGABIOS-elpin/` as selectable VGA BIOS options.

### 1.5 — Bochs Device Import

- [ ] Import Bochs VGA/SVGA device (`bx_vgacore`) as a standalone DAL-compatible device option.
- [ ] Import Bochs NE2000 network implementation as a cross-check reference against the 86Box NE2000.
- [ ] Import Bochs floppy/IDE controller implementations from `Bochs/bochs/` where they differ meaningfully from 86Box.
- [ ] Import Bochs SB16 emulation as a cross-check reference for 86Box SB16 correctness.
- [ ] Note: Bochs already bundles SeaBIOS 1.13.0 binaries in `Bochs/bochs/bios/SeaBIOS/` — use these as a known-good baseline when integrating the full SeaBIOS build from `seabios/`.

---

## Priority 2 — Full Software Emulation Path

> **Goal:** Hardware virtualization (VT-x/AMD-V) must be fully, cleanly disableable. The project must run correctly at full feature parity on hosts with no VT-x/AMD-V, restricted privilege levels (UWP, sandboxed environments), and non-x86-64 host architectures. Target host architectures include: x86-64, x86, ARM64, ARM32, RISC-V, SPARC, MIPS, and PowerPC.

### 2.1 — TCG Integration (QEMU Tiny Code Generator)

- [ ] Extract `qemu/accel/tcg/` and `qemu/target/i386/` as a self-contained static library (`libtcg_x86`), stripping all QEMU main-loop, QOM, and non-TCG dependencies.
- [ ] Reference qemu/accel/stubs/tcg-stub.c` for the minimal stub interface — this defines the TCG/non-TCG boundary cleanly.
- [ ] Wire `libtcg_x86` into VirtualBox's VMM (`virtualbox/src/VBox/VMM/`) as a third execution mode alongside `HM` (hardware) and the removed `RawMode`.
- [ ] Implement a clean `IVMMExecutionBackend` interface with implementations for: `HMBackend` (VT-x/AMD-V), `TCGBackend` (software JIT), `TCGInterpreterBackend` (no-JIT, for `W^X`-restricted environments like UWP). Note that QEMU already has a single-step interpreter path in qemu/accel/tcg/tcg-accel-ops-rr.c`.
- [ ] Ensure TCG backend has no dependency on executable memory allocation when built in interpreter mode (`VBOX_NO_EXEC_ALLOC` flag) — critical for UWP/Xbox One.
- [ ] Validate TCG guest execution against known-good VirtualBox HM output for x86_64 and i386 guests.
- [ ] Verify TCG builds and produces correct output on ARM64 hosts — this is the primary non-x86 target.
- [ ] Evaluate TCG portability to RISC-V, ARM32, MIPS, SPARC, and PowerPC hosts — document any TCG backend limitations per architecture and track as separate sub-tasks once initial ARM64 support is stable.

### 2.2 — 3D and 2D Acceleration Controls

> **Goal:** Both 3D and 2D acceleration checkboxes are always visible and toggleable for any selected video device. When a device is capable of using host GPU acceleration, LibreVMM will attempt it. For emulated cards sourced from 86Box or DOSBox-X, the checkboxes remain available but a notice appears beneath the video device dropdown stating that it is unknown whether emulated cards will benefit from host GPU acceleration.

- [ ] Expose separate 3D acceleration and 2D acceleration checkboxes in the VM display settings for every selectable video device — no device should hide or disable these controls.
- [ ] Implement host GPU acceleration attempt logic: when 3D or 2D acceleration is enabled and the selected device supports it, LibreVMM attempts to use the host GPU via the appropriate backend (virgl, VBoxSVGA, VMSVGA3D, or LLVMpipe fallback).
- [ ] Implement the LLVMpipe software fallback path so that 3D/2D acceleration remains functional on hosts with no suitable GPU.
- [ ] For video devices imported from 86Box or DOSBox-X: keep both acceleration checkboxes enabled and user-controllable, but display a non-blocking informational notice beneath the video device dropdown: *"Acceleration behavior for emulated legacy cards is unknown. The host GPU may not be able to accelerate this device."*
- [ ] Restore the `VBoxSVGA` 3D path for legacy guest OS targets (Windows 9x, NT 3.x/4.x, early Linux), gated behind device selection rather than host detection.
- [ ] Add a legacy `VESA 2.0` accelerated blit path for guests that use non-standard framebuffer access patterns.

### 2.3 — Restricted Userspace / UWP / Xbox One Build Target
- [ ] Create a `VBOX_RESTRICTED_USERSPACE` compile-time flag that gates out all kernel driver dependencies across both `virtualbox/` and the imported QEMU/86Box modules.
- [ ] Audit and replace all `VirtualAlloc(MEM_COMMIT | PAGE_EXECUTE_READWRITE)` calls behind this flag.
- [ ] Replace JIT code generation with TCG interpreter fallback when `VBOX_NO_EXEC_ALLOC` is defined.
- [ ] Audit Win32 API usage across the VirtualBox source for UWP compatibility — replace or stub any APIs not in the UWP API surface.
- [ ] Create an Xbox One / Windows Mobile test build profile.
- [ ] Document RAM and storage constraints for these targets and set appropriate default VM configuration limits.

---

## Priority 3 — Guest Additions Expansion

> **Goal:** Guest Additions should be compiled and functional for the broadest possible range of guest operating systems, not just modern Windows NT and Linux. The Guest Additions installer should bundle virtio drivers (from `kvm-guest-drivers-windows/`) and SPICE guest drivers alongside the standard additions components, and all of these should be ported to every guest OS platform targeted by this project.

### 3.1 — Legacy Windows Guest Additions

- [ ] Port Guest Additions to Windows NT 3.1 / 3.5 / 3.51 (Win32s subsystem, no NDIS5, very limited DDK).
- [ ] Port Guest Additions to Windows NT 4.0 (NDIS4, DDK 4.0 display drivers).
- [ ] Port Guest Additions to Windows 9x (Win9x VxD model, VMM32, no NT kernel).
- [ ] Port Guest Additions to Windows Me (same VxD model as 9x with minor differences).
- [ ] Port Guest Additions to Windows 2000 (closest to current NT path, lowest-effort legacy port).
- [ ] Port virtio drivers from `kvm-guest-drivers-windows/` to each legacy Windows target where technically feasible.
- [ ] Port SPICE guest drivers to each legacy Windows target where technically feasible.
- [ ] Ensure SVGA/3D acceleration Guest Additions driver is functional on Windows 9x with the restored VBoxSVGA path from Priority 2.2.
- [ ] Provide a minimal "display only" Guest Additions installer for guests where full integration is not possible.

### 3.2 — Legacy and Alternative Linux/Unix Guest Additions

- [ ] Test and repair Guest Additions for Linux kernel versions 2.4.x and 2.6.x.
- [ ] Port or adapt Guest Additions for FreeBSD, NetBSD, OpenBSD.
- [ ] Port Guest Additions for OS/2 (eComStation / ArcaOS) — basic display and shared folders.
- [ ] Evaluate feasibility of minimal Guest Additions for DOS (VESA display, shared folder via INT, mouse integration via PS/2).
- [ ] Port virtio drivers and SPICE guest drivers to Linux, FreeBSD, and OS/2 targets where feasible.
- [ ] Evaluate ReactOS Guest Additions — bring to parity with NT 4.0 level.

### 3.3 — Guest Additions Architecture
- [ ] Decouple Guest Additions components so each (display driver, mouse, shared folders, clipboard, drag-and-drop, time sync) can be installed independently.
- [ ] Define a minimal Guest Additions ABI that old OS builds can implement with fewer dependencies.
- [ ] Provide source-level documentation of the Guest Additions protocol so third parties can implement it for unsupported guests.

---

## Priority 4 — Remove VM Execution Restrictions

> **Goal:** LibreVMM never silently modifies, overrides, or corrects user configuration. All guardrails become warnings. The user is always in full control.

### 4.1 — Configuration Enforcement Removal

- [ ] Audit all locations in `virtualbox/src/VBox/Main/` and `virtualbox/src/VBox/Frontends/` where the GUI or API silently overrides a user-selected setting — catalog every instance.
- [ ] Convert every silent override to a non-blocking warning dialog that requires explicit user acknowledgment but does not prevent the configuration from being saved.
- [ ] Remove any OS-type presets that forcibly change device selections when a guest OS type is chosen. OS type selection should only populate *defaults*, never enforce constraints.
- [ ] Remove the restriction that prevents incompatible video device + OS type combinations. Display a warning, proceed regardless.
- [ ] Remove any artificial limits on VRAM size, RAM allocation, CPU count, or disk size beyond what the host physically cannot provide.
- [ ] Remove automatic disabling of features (e.g., nested virtualization, PAE, NX) based on guest OS type — these should be user-controlled exclusively.

### 4.2 — Execution Guardrail Removal

- [ ] Remove forced fallback from HM to TCG without user consent — if HM fails, report the failure and halt, do not silently downgrade.
- [ ] Remove any guest execution watchdog that resets or pauses the VM without user instruction.
- [ ] Make all VM execution timeouts configurable, including setting them to zero (disabled).
- [ ] Expose raw MSR read/write capability as a VM option (currently heavily restricted in `virtualbox/src/VBox/VMM/`).
- [ ] Allow user-defined CPU feature flags to be set or cleared without OS-type validation.

### 4.3 — Device Configuration Freedom
- [ ] Allow any device to be attached to any bus, even if the combination is historically unusual — warn, do not block.
- [ ] Allow multiple instances of the same device type where technically possible (e.g., multiple VGA devices).
- [ ] Allow emulated PCI IDs to be overridden per device instance for passthrough spoofing and compatibility testing.
- [ ] Allow manual IRQ, DMA channel, and port I/O range assignment for ISA devices (currently auto-assigned only).

---

## Priority 5 — Platform-Agnostic UI

> **Goal:** The UI frontend is a thin, replaceable layer over a stable backend API. Multiple UI implementations can coexist and target different platforms without changes to the core. UI must replicate functionality of existing 5.2 VirtualBox frontend under /vendor/VirtualBox-5.2.44/.

### 5.1 — Backend API (UI-Agnostic Core)

- [ ] Define a complete, stable `IVBoxFrontend` API that exposes all VM management, device configuration, snapshot, and execution control operations.
- [ ] Ensure the existing VirtualBox Main API (`VBoxManage` / COM / XPCOM) either *is* this API or is a direct implementation of it.
- [ ] All UI implementations must use only this API — no direct access to VMM or device layer from UI code.
- [ ] Provide a CLI implementation (`vboxext-manage`) as the reference frontend and regression baseline.

### 5.2 — Qt Frontend (Primary Desktop UI)

- [ ] Preserve the existing Qt-based VirtualBox UI as the primary desktop frontend.
- [ ] Refactor Qt frontend to fully use the `IVBoxFrontend` API (remove any direct core access that may exist).
- [ ] Extend Qt UI with device selection panels for the new QEMU/86Box/Bochs device catalog.
- [ ] Add a "raw configuration" mode to the Qt UI that exposes all settings without OS-type filtering.
- [ ] Ensure Qt UI builds on Windows (x86, ARM64), Linux (x86_64, ARM64), and macOS (x86_64, Apple Silicon). Use `SDL/` for any supplemental display output needs where SDL is more practical than a native Qt surface.
- [ ] **Optional — Win32 GDI frontend:** Evaluate a native Win32 GDI implementation of the `IVBoxFrontend` API as a lightweight alternative to the Qt frontend on Windows hosts. This path would have no Qt dependency, lower memory overhead, and broader compatibility with stripped-down Windows environments (Server Core, IoT, legacy NT). Scope mirrors the Qt frontend: VM creation wizard, device configuration, start/stop/pause, display output, snapshot management.

### 5.3 — XUL / WinRT Frontend (UWP / Windows Mobile)
- [ ] Design a minimal XUL or XAML/WinRT UI that implements the `IVBoxFrontend` API.
- [ ] Scope: VM creation wizard, basic device configuration, start/stop/pause, display output, snapshot list.
- [ ] Ensure this frontend builds within the UWP sandbox (no elevated permissions assumed).
- [ ] Use platform-native display output (CoreWindow / SwapChain) for the guest framebuffer.

### 5.4 — Java Frontend (JVM Platforms)

- [ ] Design a Java frontend implementing the `IVBoxFrontend` API via JNI bridge to the native core, targeting any platform with a JVM (desktop Linux, older Android, embedded Java SE).
- [ ] Scope: VM creation wizard, basic device configuration, start/stop/pause, display output, snapshot list.
- [ ] Use Java AWT/Swing or JavaFX for the UI layer depending on target platform capabilities.
- [ ] Keep this frontend in a separate directory from the Kotlin/Jetpack Compose frontend to avoid build system conflicts.
- [ ] Handle ARM64 and x86_64 host paths transparently via the shared native core.

### 5.5 — Kotlin/Jetpack Compose Frontend (Android)

- [ ] Design a Kotlin/Jetpack Compose frontend implementing the `IVBoxFrontend` API via JNI bridge to the native core, targeting modern Android.
- [ ] Scope: same as Java frontend — creation, configuration, execution, display.
- [ ] Use Android SurfaceView or TextureView for guest framebuffer output.
- [ ] Handle ARM64 host (TCG only) and x86_64 host (TCG + potential HM via KVM) paths transparently.
- [ ] Keep this frontend in a separate directory from the Java frontend to avoid build system conflicts.

### 5.6 — iOS Frontend (Subject to Platform Restrictions)

- [ ] Evaluate feasibility of an iOS frontend — note that iOS prohibits JIT compilation via App Store rules, meaning only the TCG interpreter mode (`VBOX_NO_EXEC_ALLOC`) would be usable. Performance implications must be documented clearly.
- [ ] If feasible, implement using SwiftUI or UIKit against the `IVBoxFrontend` API via a C/C++ bridge.
- [ ] Scope: same as other mobile frontends — creation, configuration, execution, display.

### 5.7 — Web / Electron Frontend (Optional, Lower Priority)
- [ ] Evaluate a browser-based frontend using the `IVBoxFrontend` API over a local WebSocket/REST bridge.
- [ ] This enables remote VM management and potential thin-client display (VNC/SPICE fallback).

### 5.8 — Trade Dress & Visual Identity

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

- [ ] Integrate **SeaBIOS** as a selectable legacy BIOS option.
- [ ] Integrate **Bochs BIOS** (`BIOS-bochs-latest` and `BIOS-bochs-legacy`) as selectable legacy BIOS options. The bundled `bios.bin-1.13.0` SeaBIOS binary from Bochs serves as a pre-built SeaBIOS baseline.
- [ ] Integrate **OpenBIOS** as a selectable legacy BIOS option, targeting the amd64 architecture.
- [ ] Preserve existing **OVMF (UEFI)** support, extending it to work with the new chipset options (Q35, etc.).
- [ ] Allow completely custom BIOS ROM image loading from a user-provided file.
- [ ] Expose BIOS option ROM slots as configurable entries (for legacy ISA/PCI card BIOSes). Use the ROMs in `roms/` as the device option ROM library.
- [ ] Ensure BIOS selection is independent of chipset selection — all BIOS images should attempt to work with any chipset, with incompatibilities noted as warnings only.
- [ ] Integrate VGABIOS-lgpl and VGABIOS-elpin variants from Bochs as selectable VGA BIOS ROMs for legacy video devices.

---

## Priority 7 — Build System Unification

> **Goal:** A single build system entry point produces any combination of host platform, target platform, UI frontend, and feature set.

- [ ] Audit `virtualbox/` kBuild system (primary: `Config.kmk`, `Makefile.kmk`, `Version.kmk`) and evaluate whether to extend kBuild or migrate the project-level wrapper to CMake, given that 86Box (CMake+vcpkg), QEMU (Meson), Bochs (autoconf), and DOSBox-X (Autotools/CMake) all use different systems.
- [ ] Integrate `qemu/` Meson-based build as a subproject producing `libtcg_x86` only — strip all non-i386 targets and non-TCG accelerators during this extraction.
- [ ] Integrate `86Box/` CMake build (`CMakeLists.txt` + `CMakePresets.json`) as a subproject producing device modules only, using its existing `cmake/` toolchain files for cross-compilation.
- [ ] Integrate `Bochs/bochs/` autoconf build as a subproject producing device modules and BIOS only.
- [ ] Integrate `seabios/` Makefile/Kconfig build as a subproject producing BIOS binary only.
- [ ] Integrate `openbios/` Makefile/Kconfig build as a subproject producing BIOS binary (amd64 target) only.
- [ ] Integrate `SDL/` as a shared dependency for QEMU SDL backends and the platform UI layer.
- [ ] Create build profiles for: `desktop-full`, `desktop-nogui`, `uwp-restricted`, `android`, `ios`, `headless-server`.
- [ ] Define a clear `VBOX_EXT_DEVICES_DIR` that device modules are compiled into and loaded from at runtime.
- [ ] Ensure all imported subprojects can be built without their own main executables (library-mode builds only).
- [ ] Create CI pipeline definitions for: Windows x86_64, Linux x86_64, Linux ARM64, Windows ARM64. Reference the existing CI configs in `86Box/.ci/` (Jenkins + GitHub Actions) and `qemu/.gitlab-ci.d/` for platform-specific dependency lists.
- [ ] Define a minimal build (core + TCG + one BIOS + no GUI) as a smoke-test target.

---

## Priority 8 — License & Compliance Tracking

> **Goal:** Every component's license is tracked, conflicts are identified early, and distribution is legally clean. Recursively scan /vendor subdirectories for license and copying files, copy them to /LICENSES ensuring they are renamed accordingly to the content of the license/copying text.

- [ ] Create a `LICENSES/` directory and populate it with the license text of every component. Key source files: `virtualbox/COPYING` (GPL3) + `virtualbox/COPYING.CDDL`, `qemu/COPYING` (GPL2) + `COPYING.LIB`, `86Box/COPYING` (GPL2), `Bochs/bochs/LICENSE` (LGPL2.1), `seabios/COPYING` + `COPYING.LESSER` (LGPL3+BSD), `openbios/COPYING` (GPL2), `dosbox-x/COPYING` (GPL2), `SDL/` (zlib).
- [ ] Add author attributions to `COMPONENT_LICENSES.md` for: SeaBIOS (Kevin O'Connor) and OpenBIOS (The OpenBIOS Project, based on Open Firmware by Mitch Bradley).
- [ ] Create a `COMPONENT_LICENSES.md` table mapping every source directory to its license, origin, and any known compatibility concerns.
- [ ] Resolve the VirtualBox CDDL + GPL2 split (see virtualbox/COPYING.CDDL`): maintain CDDL-licensed VBox components in a separate module boundary; never statically link GPL code into CDDL modules.
- [ ] Confirm Bochs LGPL2.1 (Bochs/bochs/LICENSE`) dynamic vs. static linking strategy.
- [ ] Confirm SeaBIOS LGPL3 (seabios/COPYING.LESSER`) compatibility with the distribution model.
- [ ] Audit `86Box/src/` for embedded third-party code with separate licensing (some device emulations have specific attributions in source headers).
- [ ] Flag any QEMU device that pulls in GPL3 or proprietary dependencies and exclude or replace.
- [ ] Confirm kvm-guest-drivers-windows/LICENSE` allows redistribution of virtio driver binaries alongside this project.
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

- [ ] Write `ARCHITECTURE.md`: high-level diagram and description of every major subsystem and their interfaces.
- [ ] Write `DEVICE_PORTING_GUIDE.md`: step-by-step guide to importing a device from QEMU, 86Box, or Bochs using the DAL.
- [ ] Write `BUILD_GUIDE.md`: complete build instructions for every supported host platform and build profile, covering the multi-build-system environment (kBuild, CMake, Meson, autoconf, Kconfig/Makefile) across all subprojects.
- [ ] Write `EXECUTION_BACKENDS.md`: describes HM, TCG JIT, and TCG interpreter modes, when each is used, and how to extend them.
- [ ] Write `GUEST_ADDITIONS_PORTING.md`: guide for porting Guest Additions to a new guest OS using the minimal ABI, including the virtio and SPICE driver porting process.
- [ ] Write `UI_FRONTEND_GUIDE.md`: guide for implementing a new UI frontend against the `IVBoxFrontend` API.
- [ ] Maintain a `KNOWN_INCOMPATIBILITIES.md` that lists device/chipset/BIOS combinations that are known to misbehave and why — so users have a reference, not a restriction.
- [ ] Add inline documentation (Doxygen) to all new interfaces defined by this project.

---

## Backlog / Under Consideration

> Items that may belong in the project but need more evaluation before committing.

- [ ] **Storage format expansion**: Evaluate read/write support for QEMU's `qcow2` and raw `.img` formats natively (QEMU block layer is in qemu/block.c` and `blockdev.c`). Currently VirtualBox supports these in limited ways.
- [ ] **SPICE remote display**: Evaluate adding a SPICE server (from QEMU's qemu/audio/spiceaudio.c` and QXL display integration) to supplement VRDP, especially for the virgl/QXL 3D acceleration path.
- [ ] **Snapshot system review**: Ensure the snapshot system in `virtualbox/src/VBox/Storage/` is not coupled to a specific execution backend.
- [ ] **Headless / server mode**: Ensure a fully headless build (no Qt, no GUI dependency) is a supported first-class target.
- [ ] **Machine type library**: Define a "machine type" system (similar to QEMU's `-machine` flag) where a complete hardware configuration is saved as a named template — separate from individual VM instances.
- [ ] **DOSBox-X — Dynamic CPU cycle scaling (cycles=auto port)**: Port DOSBox-X's dynamic cycle scaling logic from `dosbox-x/src/cpu/` into LibreVMM's TCG execution path. This system automatically adjusts emulated CPU cycles to match the guest workload in real time, preventing both under- and over-provisioning of CPU time for DOS and legacy guests. Expose as a selectable execution throttle mode in the VM CPU settings (e.g., "Dynamic / Auto" alongside fixed cycle counts). Key reference: the `cycles=auto` implementation in DOSBox-X's `CPU_CycleAutoAdjust` logic.
- [ ] **DOSBox-X — Legacy hardware import**: Audit `dosbox-x/src/hardware/` for device implementations that offer value over or alongside 86Box equivalents. Priority targets: OPL2/OPL3 FM synthesis (`opl.cpp`), Sound Blaster (all variants), Tandy/PCjr audio, GUS (Gravis UltraSound), and MIDI/MPU-401. Wrap any selected devices behind the DAL (`IVBoxExtDevice`) consistent with the 86Box import strategy in Priority 1. Document in `COMPONENT_LICENSES.md` — DOSBox-X is licensed under GPL v2.
- [ ] **Networking backend expansion**: Evaluate adding QEMU's `slirp` user-mode NAT as an alternative to VirtualBox's existing NAT engine, particularly for the restricted userspace target where tap/bridge networking is unavailable.
- [ ] **SDL display output**: `SDL/` is already present as a QEMU dependency. Evaluate using SDL as a unified display output layer for non-Qt desktop platforms or the headless build.

---

*Last updated: see git log. Priorities and items should be revised as architecture decisions are finalized.*
