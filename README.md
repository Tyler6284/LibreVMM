# LibreVMM
### 📄 [Vibe-Code Transparency Statement](./TRANSPARENCY.md)
### ⚖️ [Licenses](./LICENSES/)

## What is it?
It's a virtual machine manager based upon the VirtualBox OSE codebase!

## What is it for?
It's meant to be a well-rounded x86 and x86-64 virtualizer/emulator for a multitude of platforms.

## What features are there?
Aside from the baseline VirtualBox features, I am importing the features and basic mechanics from QEMU, 86Box, DOSBox-X, and Bochs. Down to their emulated/virtual hardware for virtual machines.

## What OS platforms are you targeting?
I'm hoping to target all, but not limited to; Windows, macOS, Linux, Android, iOS, Windows Mobile, Xbox One.

## What CPU archiecture are you going to support.
As many as I possibly can; x86-64 and x86 hosts is where this software will definitely shine, especially for use with hardware-assisted virtualization.
However, it is my intention to have this software available on ARM64, RISC-V, ARM32, SPARC, MIPS, and PowerPC as well.

## Will it function the same as VirtualBox?
It is my goal to have it functionally the same, but I must stress THIS IS NOT A SUCCESSOR TO ORACLE'S SOFTWARE. I have no authority on the project, therefore I must stress, if you have issues with this software, you submit issues, discussions, and trackers here, as opposed to unnecessarily bothering the developers behind VirtualBox.

While it will definitely function the same as VirtualBox, it's my goal to specifically target the original VirtualBox 5.2 UI, and have the design language be mirrored across multiple different UI platforms, starting with QT as the base, and doing my best to mirror the UI across other languages, like XUL, HTML5, Win32 GDI, JNI, and other such platforms.

In order to NOT infringe on Oracle's trade dress, I will be changing the icons, look-and-feel, and theme of LibreVMM, by utilizing the Humanity icons from Cannonical to fill in the gaps where necessary.

## Why are you doing this?
To be blunt, there is no consistency for virtualization/emulation for x86-64 and i386 across multiple platforms, this project aims to fix that, by creating a cohesive unit that can run on just about any platform, any hardware, on any ISA or CPU architecture.

## Final Notes
This isn't going to be perfect, but, I for one can't fathom why anyone won't run wild with what we're given, to make something universal for as many platforms as we possibly can. I get there isn't incentive, and some incentives won't match for other people, but I for one am excited for this. I want this software to unlock possibilities for everyone with any hardware, that wants the ease and use of VirtualBox, the granular, modular control, and portability of QEMU, the speed of DOSBox-X, and the accuracy of 86Box.

This isn't going to be done in a day, and with my limited hardware and limited knowledge, it's definitely going to look like a train wreck at first... but, if you believe you can contribute, if you believe there is merit to my idea, and would also love to have a cohesive, universal solution for most platforms, I encourage you to help me where you think you can.

#### This project is a fork of the VirtualBox® base package. VirtualBox is a registered trademark of Oracle Corporation. This project is not affiliated with, endorsed by, or sponsored by Oracle Corporation.

# Authors of the projects used/referenced
VirtualBox - Oracle - https://www.virtualbox.org/ - https://github.com/VirtualBox/virtualbox
DOSBox-X - Jonathon Campbell - https://dosbox-x.com/ - https://github.com/joncampbell123/dosbox-x
Bochs - https://sourceforge.net/p/bochs/_members/ - https://bochs.sourceforge.io/ - https://sourceforge.net/projects/bochs/
86Box - Miran Grča - https://86box.net/ - https://github.com/86Box/86Box
QEMU - Fabrice Bellard - https://www.qemu.org/ - https://github.com/qemu/qemu
SDL -	Sam Lantinga - https://www.libsdl.org/ - https://github.com/libsdl-org/SDL
