/**
 * @file HMBackend.cpp
 * @brief Hardware Virtualization Backend Implementation
 *
 * This file implements the HMBackend class that wraps VirtualBox's existing
 * hardware virtualization (VT-x/AMD-V) implementation.
 *
 * @note Priority 2.1: NO silent fallback - report failure and halt
 * @note Priority 2.1: Priority 4.2: Remove forced fallback from HM to TCG
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 Oracle and/or its affiliates
 */

#include "HMBackend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @name HM Backend Private Implementation
 * @brief Internal structures for HM backend
 */

/**
 * @brief HM runtime state
 */
typedef struct HMBackendPrivate {
    /** @brief HM context (would link to VirtualBox HM) */
    void *pHMContext;

    /** @brief Hardware capabilities cached at init */
    uint32_t fHardwareCaps;

    /** @brief Whether hardware has been validated */
    bool fHardwareValidated;

    /** @brief CPU vendor string */
    char szCpuVendor[16];

    /** @brief CPU model string */
    char szCpuModel[64];
} HMBackendPrivate;

/**
 * @name Forward Declarations of VirtualBox HM Functions
 * @brief These would link against VirtualBox's HM implementation
 */

extern "C" {

/**
 * @brief Initialize hardware virtualization
 * @return 0 on success, non-zero on failure
 */
int hm_init(void);

/**
 * @brief Shutdown hardware virtualization
 */
void hm_exit(void);

/**
 * @brief Check if hardware virtualization is available
 * @return 0 if available, non-zero otherwise
 */
int hm_is_available(void);

/**
 * @brief Get hardware capabilities
 * @return Bitfield of capabilities
 */
uint32_t hm_get_capabilities(void);

/**
 * @brief Create HM virtual CPU
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int hm_vcpu_create(int cpu_idx);

/**
 * @brief Destroy HM virtual CPU
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int hm_vcpu_destroy(int cpu_idx);

/**
 * @brief Run VM on hardware
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int hm_vcpu_run(int cpu_idx);

/**
 * @brief Handle VM exit
 * @param cpu_idx CPU index
 * @return Exit reason code
 */
int hm_vcpu_handle_exit(int cpu_idx);

/**
 * @brief Inject interrupt
 * @param cpu_idx CPU index
 * @param vector Interrupt vector
 * @return 0 on success, non-zero on failure
 */
int hm_inject_interrupt(int cpu_idx, int vector);

/**
 * @brief Read MSR
 * @param cpu_idx CPU index
 * @param msr MSR address
 * @param pValue Pointer to store value
 * @return 0 on success, non-zero on failure
 */
int hm_msr_read(int cpu_idx, uint32_t msr, uint64_t *pValue);

/**
 * @brief Write MSR
 * @param cpu_idx CPU index
 * @param msr MSR address
 * @param value Value to write
 * @return 0 on success, non-zero on failure
 */
int hm_msr_write(int cpu_idx, uint32_t msr, uint64_t value);

/**
 * @brief Get CPU vendor string
 * @param pBuffer Buffer to store vendor string
 * @param cbBuffer Size of buffer
 */
void hm_get_cpu_vendor(char *pBuffer, size_t cbBuffer);

/**
 * @brief Get CPU model string
 * @param pBuffer Buffer to store model string
 * @param cbBuffer Size of buffer
 */
void hm_get_cpu_model(char *pBuffer, size_t cbBuffer);

} /* extern "C" */

/**
 * @name Backend Interface Implementation
 * @brief Implementation of IVMMExecutionBackend interface
 */

static VBoxExecResult hmBackendInit(IVMMExecutionBackend *pBackend)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Allocate private data */
    pHM->pPrivate = (HMBackendPrivate *)calloc(1, sizeof(HMBackendPrivate));
    if (pHM->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_NO_MEMORY;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pHM->pPrivate;

    /* Initialize VirtualBox HM subsystem */
    int rc = hm_init();
    if (rc != 0) {
        free(pHM->pPrivate);
        pHM->pPrivate = NULL;
        return VBOX_EXEC_ERROR_HARDWARE_UNAVAILABLE;
    }

    /* Check if hardware is available */
    rc = hm_is_available();
    if (rc != 0) {
        hm_exit();
        free(pHM->pPrivate);
        pHM->pPrivate = NULL;
        return VBOX_EXEC_ERROR_HARDWARE_UNAVAILABLE;
    }

    /* Get hardware capabilities */
    pPriv->fHardwareCaps = hm_get_capabilities();

    /* Get CPU vendor and model */
    hm_get_cpu_vendor(pPriv->szCpuVendor, sizeof(pPriv->szCpuVendor));
    hm_get_cpu_model(pPriv->szCpuModel, sizeof(pPriv->szCpuModel));

    pPriv->fHardwareValidated = true;
    pHM->fHardwareAvailable = true;

    pHM->fInitialized = true;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendDestroy(IVMMExecutionBackend *pBackend)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL) {
        return VBOX_EXEC_SUCCESS;
    }

    if (pHM->pPrivate != NULL) {
        HMBackendPrivate *pPriv = (HMBackendPrivate *)pHM->pPrivate;

        /* Note: VirtualBox HM cleanup would go here */
        pPriv->pHMContext = NULL;

        free(pHM->pPrivate);
        pHM->pPrivate = NULL;
    }

    /* Shutdown VirtualBox HM subsystem */
    hm_exit();

    pHM->fInitialized = false;
    pHM->fHardwareAvailable = false;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendStart(IVMMExecutionBackend *pBackend,
                                      VMMExecutionContext *pCtx)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pHM->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pHM->fHardwareAvailable) {
        /* Priority 2.1: Do NOT silently fall back - report failure */
        return VBOX_EXEC_ERROR_HARDWARE_UNAVAILABLE;
    }

    /* Update context mode */
    pCtx->enmMode = VBOX_EXEC_MODE_HM;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendStop(IVMMExecutionBackend *pBackend,
                                     VMMExecutionContext *pCtx)
{
    (void)pBackend;
    (void)pCtx;
    /* HM cleanup is handled in VMM context */
    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendPause(IVMMExecutionBackend *pBackend,
                                      VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_PAUSED;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendResume(IVMMExecutionBackend *pBackend,
                                       VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_RUNNING;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendStep(IVMMExecutionBackend *pBackend,
                                     VMMExecutionContext *pCtx)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pHM->fInitialized || !pHM->fHardwareAvailable) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Single-step in HM requires special handling - typically via interrupt injection */
    /* For now, return not supported as HM single-step is complex */
    (void)pCtx;

    return VBOX_EXEC_ERROR_NOT_SUPPORTED;
}

static VBoxExecCapabilities hmBackendGetCaps(IVMMExecutionBackend *pBackend)
{
    HMBackend *pHM = (HMBackend *)pBackend;
    VBoxExecCapabilities caps = 0;

    if (pHM == NULL || !pHM->fHardwareAvailable) {
        return 0;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pHM->pPrivate;
    if (pPriv == NULL) {
        return 0;
    }

    /* HM always supports these */
    caps |= VBOX_EXEC_CAP_HW_VIRTUALIZATION;
    caps |= VBOX_EXEC_CAP_NESTED_PAGING;
    caps |= VBOX_EXEC_CAP_DEBUG_GUEST;

    /* Check additional capabilities from hardware */
    if (pPriv->fHardwareCaps & HM_HW_CAP_UNRESTRICTED_GUEST) {
        caps |= VBOX_EXEC_CAP_UNRESTRICTED_GUEST;
    }

    if (pPriv->fHardwareCaps & HM_HW_CAP_LARGE_PAGES) {
        caps |= VBOX_EXEC_CAP_LARGE_PAGES;
    }

    if (pPriv->fHardwareCaps & HM_HW_CAP_NESTED_HWV) {
        caps |= VBOX_EXEC_CAP_NESTED_VIRTUALIZATION;
    }

    if (pPriv->fHardwareCaps & HM_HW_CAP_SEV) {
        caps |= VBOX_EXEC_CAP_SEV;
    }

    return caps;
}

static bool hmBackendIsAvailable(IVMMExecutionBackend *pBackend)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL) {
        return false;
    }

    /* Check hardware availability */
    int rc = hm_is_available();
    return (rc == 0);
}

static void *hmBackendAllocMem(IVMMExecutionBackend *pBackend,
                                 size_t cbSize, bool fExecutable)
{
    (void)pBackend;
    (void)cbSize;
    (void)fExecutable;

    /* HM doesn't use TCG-style allocation - uses VirtualBox memory management */
    /* This would link to VBox memory allocation */
    return NULL;
}

static void hmBackendFreeMem(IVMMExecutionBackend *pBackend,
                              void *pv, size_t cbSize)
{
    (void)pBackend;
    (void)pv;
    (void)cbSize;

    /* HM doesn't use TCG-style allocation */
}

static VBoxExecResult hmBackendRun(IVMMExecutionBackend *pBackend,
                                   VMMExecutionContext *pCtx)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pHM->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pHM->fHardwareAvailable) {
        /* Priority 2.1: Do NOT silently fall back - report failure */
        return VBOX_EXEC_ERROR_HARDWARE_UNAVAILABLE;
    }

    /* Run VM on hardware */
    int rc = hm_vcpu_run((int)pCtx->idCpu);
    if (rc != 0) {
        return VBOX_EXEC_ERROR_VMEXIT;
    }

    /* Handle VM exit */
    rc = hm_vcpu_handle_exit((int)pCtx->idCpu);
    if (rc != 0) {
        /* VM exit occurred - caller should handle it */
        return VBOX_EXEC_ERROR_VMEXIT;
    }

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult hmBackendHandleVMExit(IVMMExecutionBackend *pBackend,
                                            VMMExecutionContext *pCtx)
{
    HMBackend *pHM = (HMBackend *)pBackend;

    if (pHM == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pHM->pPrivate;
    if (pPriv == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Handle VM exit */
    int rc = hm_vcpu_handle_exit((int)pCtx->idCpu);

    return (rc == 0) ? VBOX_EXEC_SUCCESS : VBOX_EXEC_ERROR_VMEXIT;
}

static VBoxExecResult hmBackendInvalidateCpuCache(IVMMExecutionBackend *pBackend,
                                                   VMMExecutionContext *pCtx)
{
    /* HM typically doesn't need explicit cache invalidation */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

/**
 * @name HM Backend Factory Functions
 */

IVMMExecutionBackend *HMBackendCreate(void)
{
    HMBackendConfig config = HMBackendGetDefaultConfig();
    return HMBackendCreateWithConfig(&config);
}

IVMMExecutionBackend *HMBackendCreateWithConfig(HMBackendConfig *pConfig)
{
    HMBackend *pHM = (HMBackend *)calloc(1, sizeof(HMBackend));
    if (pHM == NULL) {
        return NULL;
    }

    /* Set up base interface */
    pHM->base.pszName = "HM";
    pHM->base.pszDescription = "Hardware Virtualization (VT-x/AMD-V)";
    pHM->base.enmMode = VBOX_EXEC_MODE_HM;
    pHM->base.fCapabilities = VBOX_EXEC_CAP_HW_VIRTUALIZATION |
                                VBOX_EXEC_CAP_NESTED_PAGING |
                                VBOX_EXEC_CAP_UNRESTRICTED_GUEST |
                                VBOX_EXEC_CAP_DEBUG_GUEST |
                                VBOX_EXEC_CAP_LARGE_PAGES |
                                VBOX_EXEC_CAP_NESTED_VIRTUALIZATION |
                                VBOX_EXEC_CAP_SEV;

    /* Set up function pointers */
    pHM->base.pfnInit = hmBackendInit;
    pHM->base.pfnDestroy = hmBackendDestroy;
    pHM->base.pfnStart = hmBackendStart;
    pHM->base.pfnStop = hmBackendStop;
    pHM->base.pfnPause = hmBackendPause;
    pHM->base.pfnResume = hmBackendResume;
    pHM->base.pfnStep = hmBackendStep;
    pHM->base.pfnGetCaps = hmBackendGetCaps;
    pHM->base.pfnIsAvailable = hmBackendIsAvailable;
    pHM->base.pfnAllocMem = hmBackendAllocMem;
    pHM->base.pfnFreeMem = hmBackendFreeMem;
    pHM->base.pfnRun = hmBackendRun;
    pHM->base.pfnHandleVMExit = hmBackendHandleVMExit;
    pHM->base.pfnInvalidateCache = hmBackendInvalidateCpuCache;

    /* Copy configuration */
    if (pConfig != NULL) {
        pHM->Config = *pConfig;
    } else {
        pHM->Config = HMBackendGetDefaultConfig();
    }

    pHM->fInitialized = false;
    pHM->fHardwareAvailable = false;

    return &pHM->base;
}

void HMBackendDestroy(IVMMExecutionBackend *pBackend)
{
    if (pBackend != NULL && pBackend->pfnDestroy != NULL) {
        pBackend->pfnDestroy(pBackend);
    }
    free(pBackend);
}

/**
 * @name HM Backend Implementation
 */

VBoxExecResult HMBackendInitHardware(HMBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnInit(&pBackend->base);
}

VBoxExecResult HMBackendEnableFeature(HMBackend *pBackend, uint32_t fFeature)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    if (pPriv == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Enable feature in HM - would link to VirtualBox HM API */
    (void)fFeature;

    return VBOX_EXEC_SUCCESS;
}

uint32_t HMBackendGetHardwareCaps(HMBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return 0;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    return pPriv->fHardwareCaps;
}

bool HMBackendIsVTxAvailable(HMBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return false;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    return (pPriv->fHardwareCaps & HM_HW_CAP_VTX) != 0;
}

bool HMBackendIsAMDVAvailable(HMBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return false;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    return (pPriv->fHardwareCaps & HM_HW_CAP_AMDV) != 0;
}

bool HMBackendIsNestedPagingAvailable(HMBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return false;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    return (pPriv->fHardwareCaps & HM_HW_CAP_NESTED_PAGING) != 0;
}

bool HMBackendIsUnrestrictedGuestAvailable(HMBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return false;
    }

    HMBackendPrivate *pPriv = (HMBackendPrivate *)pBackend->pPrivate;
    return (pPriv->fHardwareCaps & HM_HW_CAP_UNRESTRICTED_GUEST) != 0;
}

VBoxExecResult HMBackendHandleVMExit(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnHandleVMExit(&pBackend->base, pCtx);
}

VBoxExecResult HMBackendHandleExtInterrupt(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Handle external interrupt */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult HMBackendHandleIOInstruction(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Handle I/O instruction */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult HMBackendHandleCPUID(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Handle CPUID instruction */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult HMBackendHandleMSRRead(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    (void)pBackend;
    (void)pCtx;
    /* Would link to VirtualBox HM MSR handling */
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult HMBackendHandleMSRWrite(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    (void)pBackend;
    (void)pCtx;
    /* Would link to VirtualBox HM MSR handling */
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult HMBackendHandlePageFault(HMBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Handle page fault */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

/**
 * @name Host Capability Detection
 */

uint32_t HMHardwareDetectCapabilities(void)
{
    /* Initialize HM first */
    int rc = hm_init();
    if (rc != 0) {
        return 0;
    }

    /* Check if available */
    rc = hm_is_available();
    if (rc != 0) {
        hm_exit();
        return 0;
    }

    /* Get capabilities */
    uint32_t fCaps = hm_get_capabilities();

    hm_exit();

    return fCaps;
}

bool HMHardwareIsVirtualizationAvailable(void)
{
    int rc = hm_is_available();
    return (rc == 0);
}

const char *HMHardwareGetCpuVendor(void)
{
    static char szVendor[16] = {0};
    if (szVendor[0] == '\0') {
        hm_get_cpu_vendor(szVendor, sizeof(szVendor));
    }
    return szVendor;
}

const char *HMHardwareGetCpuModel(void)
{
    static char szModel[64] = {0};
    if (szModel[0] == '\0') {
        hm_get_cpu_model(szModel, sizeof(szModel));
    }
    return szModel;
}

/**
 * @name Configuration Functions
 */

HMBackendConfig HMBackendGetDefaultConfig(void)
{
    HMBackendConfig config = {0};
    config.fNestedPaging = HM_DEFAULT_NESTED_PAGING;
    config.fUnrestrictedGuest = HM_DEFAULT_UNRESTRICTED_GUEST;
    config.fLargePages = HM_DEFAULT_LARGE_PAGES;
    config.fNestedVirtualization = HM_DEFAULT_NESTED_VIRTUALIZATION;
    config.fSEV = HM_DEFAULT_SEV;
    config.fLargePagesForSecondaryCpus = false;
    config.fVPID = true;
    config.fXSAVE = true;
    config.fInvariantTSC = true;
    config.fForceLegacyPMU = false;
    return config;
}

VBoxExecResult HMBackendApplyConfig(HMBackend *pBackend, HMBackendConfig *pConfig)
{
    if (pBackend == NULL || pConfig == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Apply configuration - would link to VirtualBox HM API */
    pBackend->Config = *pConfig;

    return VBOX_EXEC_SUCCESS;
}

/**
 * @name VMBackendsCreateHM - Factory function for backend manager
 */
IVMMExecutionBackend *VMBackendsCreateHM(void)
{
    return HMBackendCreate();
}
