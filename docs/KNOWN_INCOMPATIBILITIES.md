# Known Incompatibilities

> This document lists device, chipset, and BIOS combinations that are known to misbehave in LibreVMM. These are documented for reference—not restrictions. Users are free to use any combination but should be aware of potential issues.

**Note:** LibreVMM's philosophy is to warn, never block. If a combination is unusual or known problematic, a warning is displayed, but the configuration is allowed.

---

## How to Contribute

Community contributions are welcome! To add a new incompatibility:

1. **Identify the issue:** Document the exact combination that causes problems
2. **Describe the symptoms:** What happens when this combination is used?
3. **Explain the cause:** Why does this combination not work?
4. **Provide workarounds:** Are there alternative configurations that work?

Submit contributions via GitHub pull requests or issue reports.

---

## Format for Documenting Issues

Use the following format for each incompatibility entry:

```markdown
### Issue Title

**Affected Components:**
- Component A (version X.Y)
- Component B (version X.Y)

**Symptoms:**
- Description of the problem

**Cause:**
- Technical explanation

**Workaround:**
- Alternative configuration or fix
```

---

## Current Known Incompatibilities

### USB OHCI Controller + Windows 98

**Affected Components:**
- USB Controller: OHCI
- Guest OS: Windows 98/98 SE

**Symptoms:**
- USB devices not detected
- System hangs during USB enumeration
- Blue screen on USB device insertion

**Cause:**
- Windows 98 OHCI driver has timing issues with the emulated USB controller
- The guest driver expects specific timing that the emulation does not provide

**Workaround:**
- Use USB 1.1 (UHCI) controller instead
- Or upgrade to Windows ME which has fixed OHCI support

---

### QXL Device + DOS Guests

**Affected Components:**
- Video Device: QXL (SPICE)
- Guest OS: DOS

**Symptoms:**
- No display output
- Application hangs immediately after launch

**Cause:**
- QXL requires guest drivers that are not available for DOS
- DOS cannot run the QXL driver

**Workaround:**
- Use standard VGA or VESA display adapter for DOS guests

---

### virtio-blk + Windows XP

**Affected Components:**
- Storage Controller: virtio-blk
- Guest OS: Windows XP

**Symptoms:**
- Blue screen (BSOD) during boot
- "STOP: 0x0000007B" (INACCESSIBLE_BOOT_DEVICE)

**Cause:**
- Windows XP does not include virtio drivers natively
- The virtio driver needs to be injected via install media, but XP has driver signing issues

**Workaround:**
- Use IDE or SATA controller for Windows XP storage
- Or use the Red Hat virtio drivers with proper installation procedure

---

### Bochs BIOS + Q35 Chipset

**Affected Components:**
- BIOS: Bochs BIOS
- Chipset: Q35 (Intel ICH9)

**Symptoms:**
- Guest fails to boot
- BIOS hangs at "Booting from Hard Disk"

**Cause:**
- Bochs BIOS does not fully support Q35/ICH9 MMIO layout
- Some required PCI configuration space is not properly initialized

**Workaround:**
- Use SeaBIOS or OpenBIOS with Q35 chipset
- Use i440FX (PIIX3) chipset with Bochs BIOS

---

### SeaBIOS + Windows 3.11

**Affected Components:**
- BIOS: SeaBIOS
- Guest OS: Windows 3.11

**Symptoms:**
- Guest hangs during boot
- No display output after BIOS POST

**Cause:**
- SeaBIOS has compatibility issues with very old Windows versions
- BIOSint13 hooks are not properly set up for Windows 3.x

**Workaround:**
- Use Bochs BIOS for Windows 3.x guests

---

### E1000 + Windows 2008 R2 (Limited)

**Affected Components:**
- Network Adapter: E1000
- Guest OS: Windows Server 2008 R2

**Symptoms:**
- Network connectivity issues under heavy load
- Packet loss observed

**Cause:**
- Known issue with the E1000 emulated NIC and Windows 2008 R2
- MSI-X interrupt handling issues in the guest driver

**Workaround:**
- Use E1000E instead of E1000
- Or use virtio-net for better performance

---

### Nested Paging + 32-bit Windows XP PAE

**Affected Components:**
- Execution Mode: HM (Hardware Virtualization)
- Feature: Nested Paging
- Guest OS: Windows XP with PAE

**Symptoms:**
- Guest crashes randomly
- Blue screens with memory management errors

**Cause:**
- Interaction between PAE and nested paging has known issues
- Some XP PAE drivers are not fully compatible

**Workaround:**
- Disable nested paging for XP PAE guests
- Or disable PAE in the guest

---

### RTL8139 + Linux 2.4.x

**Affected Components:**
- Network Adapter: RTL8139
- Guest OS: Linux 2.4.x

**Symptoms:**
- Network interface not detected
- No eth0 device

**Cause:**
- Linux 2.4.x RTL8139 driver has bugs with the emulated card
- Older driver version has incomplete PCI probe code

**Workaround:**
- Use NE2000 or PCNet network adapter for Linux 2.4.x guests

---

### USB XHCI + Windows 7 (Without SP1)

**Affected Components:**
- USB Controller: XHCI (USB 3.0)
- Guest OS: Windows 7 (pre-SP1)

**Symptoms:**
- USB 3.0 devices not recognized
- XHCI driver fails to load

**Cause:**
- Windows 7 RTM does not include XHCI driver
- USB 3.0 support was added in Windows 7 SP1

**Workaround:**
- Install Windows 7 SP1 or later
- Or use EHCI/OHCI controller for USB 2.0 devices

---

### VMSVGA + Windows 9x (3D Acceleration)

**Affected Components:**
- Video Device: VMSVGA (VMware SVGA II)
- Feature: 3D Acceleration
- Guest OS: Windows 95/98/Me

**Symptoms:**
- Guest crashes when 3D acceleration is enabled
- Blue screen during driver installation

**Cause:**
- The VMSVGA 3D acceleration is not compatible with Windows 9x
- Mesa-based software rendering does not work in this environment

**Workaround:**
- Disable 3D acceleration for Windows 9x guests
- Use VBoxVGA or standard VGA instead

---

### Large Pages + TCG Backend

**Affected Components:**
- Execution Mode: TCG (Software Emulation)
- Feature: Large Pages

**Symptoms:**
- VM fails to start
- Memory allocation errors

**Cause:**
- Large pages are an HM (hardware virtualization) feature
- TCG cannot use large pages for guest memory

**Workaround:**
- Disable large pages when using TCG backend
- This is automatically handled by LibreVMM

---

### IDE PIIX + Windows NT 4.0 (Boot Issues)

**Affected Components:**
- Storage Controller: IDE (PIIX)
- Guest OS: Windows NT 4.0

**Symptoms:**
- Guest fails to boot from IDE
- "INACCESSIBLE_BOOT_DEVICE" blue screen

**Cause:**
- NT 4.0 IDE driver has issues with the emulated PIIX controller
- The driver expects specific PCI configuration

**Workaround:**
- Use SCSI (LSI53C895A) controller for Windows NT 4.0
- Or use the Intel IDE driver update from Microsoft

---

### AC97 + DOS Guests

**Affected Components:**
- Audio: AC97 (Intel HD Audio)
- Guest OS: DOS

**Symptoms:**
- No audio output
- DOS applications crash when accessing audio

**Cause:**
- DOS cannot use AC97 - it requires a proper driver
- DOS audio is primarily through PC speaker or Sound Blaster

**Workaround:**
- Use Sound Blaster 16 or other ISA sound card for DOS audio
- Or use the PC speaker emulation

---

### OpenBIOS + macOS Guests

**Affected Components:**
- BIOS: OpenBIOS
- Guest OS: macOS (Darwin)

**Symptoms:**
- Guest fails to boot
- BIOS cannot find bootloader

**Cause:**
- OpenBIOS does not provide full Apple-specific firmware support
- macOS requires specific firmware interfaces not present in OpenBIOS

**Workaround:**
- Use OVMF (UEFI) firmware for macOS guests
- OpenBIOS is not recommended for macOS virtualization

---

## Placeholder for Community Contributions

> **Your contribution here!**

The following areas need more testing and documentation:

- [ ] ARM64 guest combinations
- [ ] RISC-V guest combinations
- [ ] More legacy OS configurations (OS/2, BeOS, etc.)
- [ ] Firmware combinations with new chipsets
- [ ] virtio combinations with various guest OS versions

---

## Testing New Combinations

If you encounter an issue not listed here:

1. **Document the exact versions:** Include LibreVMM version, guest OS version, and device versions
2. **Identify the trigger:** What causes the issue? Is it consistent?
3. **Check for existing reports:** Search the issue tracker
4. **Try workarounds:** Document what you tried and what worked
5. **Submit:** Create an issue or pull request to add to this list

---

## Related Documentation

- [Priority 1 — Expanded Device Layer](../TODO.md#priority-1--expanded-device-layer)
- [Priority 4 — Remove VM Execution Restrictions](../TODO.md#priority-4---remove-vm-execution-restrictions)
- [DEVICE_PORTING_GUIDE.md](./DEVICE_PORTING_GUIDE.md)
- [EXECUTION_BACKENDS.md](./EXECUTION_BACKENDS.md)
