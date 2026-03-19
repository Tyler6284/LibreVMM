/**
 * @file HMBackend.h
 * @brief Hardware Virtualization Backend for LibreVMM
 *
 * This file defines the HMBackend class that wraps VirtualBox's existing
 * hardware virtualization (VT-x/AMD-V) implementation.
 *
 * Key features:
 * - Wraps existing VirtualBox HM implementation
 * - Uses VT-x (Intel) or AMD-V (AMD) for hardware-assisted virtualization
 * - Supports nested paging and unrestricted guest mode
 *
 * @note Priority 2.1: Must report failure and halt on HM unavailability
 *       (NO silent fallback to TCG)
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 Oracle and/or its affiliates
 */

#ifndef HM_BACKEND_H
#define HM_BACKEND_H

#include "../../include/IVMMExecutionBackend.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name HM Backend Configuration
 * @brief Configuration options for the HM backend
 */
typedef struct HMBackendConfig {
    /** @brief Enable nested paging (EPT/RVI) */
    bool fNestedPaging;

    /** @brief Enable unrestricted guest mode */
    bool fUnrestrictedGuest;

    /** @brief Enable large pages */
    bool fLargePages;

    /** @brief Enable nested virtualization (VT-x/AMD-V) */
    bool fNestedVirtualization;

    /** @brief Enable AMD-V SEV (Secure Encrypted Virtualization) */
    bool fSEV;

    /** @brief Use large pages for secondary CPUs */
    bool fLargePagesForSecondaryCpus;

    /** @brief Enable VPID (Virtual Processor ID) */
    bool fVPID;

    /** @brief Enable XSAVE/XRSTOR */
    bool fXSAVE;

    /** @brief Enable invariant TSC */
    bool fInvariantTSC;

    /** @brief Force legacy PMU (Performance Monitoring Unit) */
    bool fForceLegacyPMU;
} HMBackendConfig;

/**
 * @name Default Configuration
 */
#define HM_DEFAULT_NESTED_PAGING true
#define HM_DEFAULT_UNRESTRICTED_GUEST true
#define HM_DEFAULT_LARGE_PAGES true
#define HM_DEFAULT_NESTED_VIRTUALIZATION false
#define HM_DEFAULT_SEV false

/**
 * @name HM Backend Opaque Structure
 */
typedef struct HMBackendPrivate HMBackendPrivate;

/**
 * @brief HM Backend for hardware virtualization
 *
 * The HMBackend wraps VirtualBox's existing hardware virtualization
 * implementation (HM - Hardware Mode) to provide near-native performance
 * using VT-x (Intel) or AMD-V (AMD) technology.
 */
typedef struct HMBackend {
    /** @brief Base execution backend interface */
    IVMMExecutionBackend base;

    /** @brief Backend configuration */
    HMBackendConfig Config;

    /** @brief Whether HM is initialized */
    bool fInitialized;

    /** @brief Whether hardware is available */
    bool fHardwareAvailable;

    /** @brief Opaque private data */
    HMBackendPrivate *pPrivate;
} HMBackend;

/**
 * @name Hardware Capability Flags
 * @brief Flags indicating which hardware features are available
 */
typedef enum {
    HM_HW_CAP_VTX = 0x0001,       /**< Intel VT-x available */
    HM_HW_CAP_AMDV = 0x0002,       /**< AMD-V available */
    HM_HW_CAP_NESTED_PAGING = 0x0004,  /**< EPT/RVI available */
    HM_HW_CAP_UNRESTRICTED_GUEST = 0x0008, /**< Unrestricted guest available */
    HM_HW_CAP_LARGE_PAGES = 0x0010, /**< Large pages available */
    HM_HW_CAP_VPID = 0x0020,       /**< VPID available */
    HM_HW_CAP_XSAVE = 0x0040,      /**< XSAVE/XRSTOR available */
    HM_HW_CAP_INVARIANT_TSC = 0x0080, /**< Invariant TSC available */
    HM_HW_CAP_NESTED_HWV = 0x0100, /**< Nested virtualization available */
    HM_HW_CAP_SEV = 0x0200         /**< AMD SEV available */
} HMHardwareCaps;

/**
 * @name HM Backend Functions
 * @brief Functions for creating and managing HM backend
 */

/**
 * @brief Create a new HM backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *HMBackendCreate(void);

/**
 * @brief Create an HM backend with custom configuration
 * @param pConfig Pointer to configuration structure
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *HMBackendCreateWithConfig(HMBackendConfig *pConfig);

/**
 * @brief Destroy an HM backend instance
 * @param pBackend Pointer to backend to destroy
 */
void HMBackendDestroy(IVMMExecutionBackend *pBackend);

/**
 * @name HM-Specific Functions
 * @brief Functions specific to HM backend operation
 */

/**
 * @brief Initialize HM hardware
 * @param pBackend Pointer to HM backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult HMBackendInitHardware(HMBackend *pBackend);

/**
 * @brief Enable a hardware feature
 * @param pBackend Pointer to HM backend
 * @param fFeature Feature flag to enable
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult HMBackendEnableFeature(HMBackend *pBackend, uint32_t fFeature);

/**
 * @brief Get hardware capabilities
 * @param pBackend Pointer to HM backend
 * @return Bitfield of available hardware capabilities
 */
uint32_t HMBackendGetHardwareCaps(HMBackend *pBackend);

/**
 * @brief Check if VT-x is available
 * @param pBackend Pointer to HM backend
 * @return true if VT-x is available
 */
bool HMBackendIsVTxAvailable(HMBackend *pBackend);

/**
 * @brief Check if AMD-V is available
 * @param pBackend Pointer to HM backend
 * @return true if AMD-V is available
 */
bool HMBackendIsAMDVAvailable(HMBackend *pBackend);

/**
 * @brief Check if nested paging is available
 * @param pBackend Pointer to HM backend
 * @return true if nested paging is available
 */
bool HMBackendIsNestedPagingAvailable(HMBackend *pBackend);

/**
 * @brief Check if unrestricted guest is available
 * @param pBackend Pointer to HM backend
 * @return true if unrestricted guest is available
 */
bool HMBackendIsUnrestrictedGuestAvailable(HMBackend *pBackend);

/**
 * @brief Handle VM exit from hardware
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 *
 * @note This is called when hardware triggers a VM exit, which must
 *       be handled appropriately (e.g., for I/O operations, interrupts)
 */
VBoxExecResult HMBackendHandleVMExit(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @name VM Exit Handling
 * @brief Functions for handling VM exits
 */

/**
 * @brief Handle external interrupt VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandleExtInterrupt(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle I/O instruction VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandleIOInstruction(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle CPUID VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandleCPUID(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle MSR read VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandleMSRRead(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle MSR write VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandleMSRWrite(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle page fault VM exit
 * @param pBackend Pointer to HM backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
VBoxExecResult HMBackendHandlePageFault(HMBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @name Host Capability Detection
 * @brief Functions to detect hardware capabilities
 */

/**
 * @brief Detect and cache hardware capabilities
 * @return Bitfield of available hardware capabilities
 */
uint32_t HMHardwareDetectCapabilities(void);

/**
 * @brief Check if hardware virtualization is available
 * @return true if VT-x or AMD-V is available
 */
bool HMHardwareIsVirtualizationAvailable(void);

/**
 * @brief Get CPU vendor (Intel/AMD)
 * @return CPU vendor identifier
 */
const char *HMHardwareGetCpuVendor(void);

/**
 * @brief Get CPU model name
 * @return CPU model name string
 */
const char *HMHardwareGetCpuModel(void);

/**
 * @name Configuration Functions
 * @brief Functions for managing HM configuration
 */

/**
 * @brief Get default HM configuration
 * @return Default configuration structure
 */
HMBackendConfig HMBackendGetDefaultConfig(void);

/**
 * @brief Apply HM configuration
 * @param pBackend Pointer to HM backend
 * @param pConfig Configuration to apply
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult HMBackendApplyConfig(HMBackend *pBackend, HMBackendConfig *pConfig);

#endif /* HM_BACKEND_H */
