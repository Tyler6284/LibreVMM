/**
 * @file TCGBackend.cpp
 * @brief TCG (Tiny Code Generator) Backend Implementation
 *
 * This file implements the TCGBackend class that wraps QEMU's TCG engine
 * for software JIT execution of guest code.
 *
 * @note Priority 2.1: Must work with VBOX_NO_EXEC_ALLOC for interpreter mode
 * @note Priority 2.1: Extract qemu/accel/tcg/ and qemu/target/i386/ as libtcg_x86
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 QEMU contributors
 */

#include "TCGBackend.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @name TCG Backend Private Implementation
 * @brief Internal structures for TCG backend
 */

/**
 * @brief TCG runtime state
 */
typedef struct TCGPrivate {
    /** @brief TCG context (opaque, from QEMU) */
    void *pTcgContext;

    /** @brief TCG code generator state */
    void *pTcgState;

    /** @brief Current code cache size */
    uint64_t u64CodeCacheSize;

    /** @brief Total ops executed */
    uint64_t cOps;

    /** @brief Total jumps executed */
    uint64_t cJumps;

    /** @brief Total cache invalidations */
    uint64_t cInvalidates;

    /** @brief Memory regions registered */
    void **ppvRegions;
    size_t *pcbRegions;
    uint32_t cRegions;

    /** @brief CPU state buffer */
    void *pvCpuState;
} TCGPrivate;

/**
 * @name Forward Declarations of QEMU TCG Functions
 * @brief These would link against libtcg_x86 from vendor/qemu/
 */

/* These functions would be implemented in the extracted QEMU TCG code */
extern "C" {

/**
 * @brief Initialize TCG runtime
 * @return 0 on success, non-zero on failure
 */
int tcg_init(void);

/**
 * @brief Shutdown TCG runtime
 */
void tcg_exit(void);

/**
 * @brief Create TCG context
 * @param cpus Number of CPUs
 * @return TCG context handle, or NULL on failure
 */
void *tcg_context_create(int cpus);

/**
 * @brief Destroy TCG context
 * @param ctx TCG context handle
 */
void tcg_context_destroy(void *ctx);

/**
 * @brief Allocate TCG code buffer
 * @param ctx TCG context handle
 * @param size Size to allocate
 * @return Pointer to code buffer, or NULL on failure
 */
void *tcg_code_gen_alloc(void *ctx, size_t size);

/**
 * @brief Free TCG code buffer
 * @param ctx TCG context handle
 * @param ptr Pointer to free
 * @param size Size that was allocated
 */
void tcg_code_gen_free(void *ctx, void *ptr, size_t size);

/**
 * @brief Flush TCG code cache
 * @param ctx TCG context handle
 */
void tcg_code_gen_flush(void *ctx);

/**
 * @brief Set TCG optimization level
 * @param ctx TCG context handle
 * @param level Optimization level (0-2)
 */
void tcg_set_optimization_level(void *ctx, int level);

/**
 * @brief Set CPU type for TCG
 * @param ctx TCG context handle
 * @param cpu_type CPU type string
 */
void tcg_set_cpu_type(void *ctx, const char *cpu_type);

/**
 * @brief Execute TCG code
 * @param ctx TCG context handle
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int tcg_cpu_exec(void *ctx, int cpu_idx);

/**
 * @brief Single step TCG execution
 * @param ctx TCG context handle
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int tcg_cpu_exec_step(void *ctx, int cpu_idx);

/**
 * @brief Register memory region
 * @param ctx TCG context handle
 * @param start Start address
 * @param size Size of region
 * @param is_readonly Whether region is read-only
 */
void tcg_register_memory(void *ctx, uint64_t start, uint64_t size, int is_readonly);

/**
 * @brief Unregister memory region
 * @param ctx TCG context handle
 * @param start Start address
 * @param size Size of region
 */
void tcg_unregister_memory(void *ctx, uint64_t start, uint64_t size);

/**
 * @brief Invalidate code cache for range
 * @param ctx TCG context handle
 * @param start Start address
 * @param size Size of range
 */
void tcg_invalidate_code_gen_cache_range(void *ctx, uint64_t start, uint64_t size);

/**
 * @brief Get TCG capabilities
 * @return Bitfield of TCG capabilities
 */
uint32_t tcg_get_capabilities(void);

} /* extern "C" */

/**
 * @name Backend Interface Implementation
 * @brief Implementation of IVMMExecutionBackend interface
 */

static VBoxExecResult tcgBackendInit(IVMMExecutionBackend *pBackend)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Allocate private data */
    pTcg->pPrivate = (TCGBackendPrivate *)calloc(1, sizeof(TCGPrivate));
    if (pTcg->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_NO_MEMORY;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;

    /* Determine if JIT is enabled based on configuration and build flags */
#if defined(VBOX_NO_EXEC_ALLOC)
    /* Interpreter mode - no JIT */
    pTcg->fJitEnabled = false;
    pTcg->Config.fInterpreterMode = true;
#else
    /* Check configuration preference */
    if (pTcg->Config.fInterpreterMode) {
        pTcg->fJitEnabled = false;
    } else {
        pTcg->fJitEnabled = true;
    }
#endif

    /* Initialize QEMU TCG subsystem */
    int rc = tcg_init();
    if (rc != 0) {
        free(pTcg->pPrivate);
        pTcg->pPrivate = NULL;
        return VBOX_EXEC_ERROR_NOT_SUPPORTED;
    }

    /* Create TCG context */
    pPriv->pTcgContext = tcg_context_create(pTcg->Config.cThreads);
    if (pPriv->pTcgContext == NULL) {
        tcg_exit();
        free(pTcg->pPrivate);
        pTcg->pPrivate = NULL;
        return VBOX_EXEC_ERROR_NO_MEMORY;
    }

    /* Set optimization level */
    tcg_set_optimization_level(pPriv->pTcgContext, pTcg->Config.iOptLevel);

    pTcg->fInitialized = true;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendDestroy(IVMMExecutionBackend *pBackend)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL) {
        return VBOX_EXEC_SUCCESS;
    }

    if (pTcg->pPrivate != NULL) {
        TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;

        /* Destroy TCG context */
        if (pPriv->pTcgContext != NULL) {
            tcg_context_destroy(pPriv->pTcgContext);
        }

        /* Free memory regions */
        if (pPriv->ppvRegions != NULL) {
            free(pPriv->ppvRegions);
        }
        if (pPriv->pcbRegions != NULL) {
            free(pPriv->pcbRegions);
        }

        /* Free CPU state buffer */
        if (pPriv->pvCpuState != NULL) {
            free(pPriv->pvCpuState);
        }

        free(pTcg->pPrivate);
        pTcg->pPrivate = NULL;
    }

    /* Shutdown QEMU TCG subsystem */
    tcg_exit();

    pTcg->fInitialized = false;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendStart(IVMMExecutionBackend *pBackend,
                                       VMMExecutionContext *pCtx)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pTcg->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Update context mode */
    pCtx->enmMode = VBOX_EXEC_MODE_TCG_JIT;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendStop(IVMMExecutionBackend *pBackend,
                                      VMMExecutionContext *pCtx)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Flush code cache on stop */
    if (pTcg->pPrivate != NULL) {
        TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
        if (pPriv->pTcgContext != NULL) {
            tcg_code_gen_flush(pPriv->pTcgContext);
        }
    }

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendPause(IVMMExecutionBackend *pBackend,
                                       VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_PAUSED;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendResume(IVMMExecutionBackend *pBackend,
                                        VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_RUNNING;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult tcgBackendStep(IVMMExecutionBackend *pBackend,
                                      VMMExecutionContext *pCtx)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pTcg->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Execute single step */
    int rc = tcg_cpu_exec_step(pPriv->pTcgContext, (int)pCtx->idCpu);

    pCtx->enmState = VBOX_EXEC_STATE_SINGLE_STEP;

    return (rc == 0) ? VBOX_EXEC_SUCCESS : VBOX_EXEC_ERROR_CPU_ERROR;
}

static VBoxExecCapabilities tcgBackendGetCaps(IVMMExecutionBackend *pBackend)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;
    VBoxExecCapabilities caps = 0;

    /* TCG always supports interpreter */
    caps |= VBOX_EXEC_CAP_INTERPRETER;
    caps |= VBOX_EXEC_CAP_SINGLE_STEP;
    caps |= VBOX_EXEC_CAP_DEBUG_GUEST;
    caps |= VBOX_EXEC_CAP_NESTED_PAGING;

    /* JIT requires executable memory allocation */
#if !defined(VBOX_NO_EXEC_ALLOC)
    if (pTcg != NULL && !pTcg->Config.fInterpreterMode) {
        caps |= VBOX_EXEC_CAP_JIT_COMPILATION;
    }
#endif

    return caps;
}

static bool tcgBackendIsAvailable(IVMMExecutionBackend *pBackend)
{
    /* TCG is always available as it's pure software emulation */
    (void)pBackend;
    return true;
}

static void *tcgBackendAllocMem(IVMMExecutionBackend *pBackend,
                                 size_t cbSize, bool fExecutable)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL) {
        return NULL;
    }

#if defined(VBOX_NO_EXEC_ALLOC)
    /* No executable memory available - return NULL for exec allocations */
    if (fExecutable) {
        return NULL;
    }
#endif

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return NULL;
    }

    if (fExecutable && !pTcg->fJitEnabled) {
        /* JIT not enabled, cannot allocate executable memory */
        return NULL;
    }

    return tcg_code_gen_alloc(pPriv->pTcgContext, cbSize);
}

static void tcgBackendFreeMem(IVMMExecutionBackend *pBackend,
                              void *pv, size_t cbSize)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL || pv == NULL) {
        return;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return;
    }

    tcg_code_gen_free(pPriv->pTcgContext, pv, cbSize);
}

static VBoxExecResult tcgBackendRun(IVMMExecutionBackend *pBackend,
                                     VMMExecutionContext *pCtx)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pTcg->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Execute TCG code */
    int rc = tcg_cpu_exec(pPriv->pTcgContext, (int)pCtx->idCpu);

    /* Update stats */
    pPriv->cOps++;

    return (rc == 0) ? VBOX_EXEC_SUCCESS : VBOX_EXEC_ERROR_CPU_ERROR;
}

static VBoxExecResult tcgBackendHandleVMExit(IVMMExecutionBackend *pBackend,
                                              VMMExecutionContext *pCtx)
{
    /* TCG doesn't have VM exits - this is HM-only */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_ERROR_NOT_SUPPORTED;
}

static VBoxExecResult tcgBackendInvalidateCpuCache(IVMMExecutionBackend *pBackend,
                                                    VMMExecutionContext *pCtx)
{
    TCGBackend *pTcg = (TCGBackend *)pBackend;

    if (pTcg == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pTcg->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Flush entire code cache on CPU cache invalidation */
    tcg_code_gen_flush(pPriv->pTcgContext);
    pPriv->cInvalidates++;

    return VBOX_EXEC_SUCCESS;
}

/**
 * @name TCG Backend Factory Functions
 */

IVMMExecutionBackend *TCGBackendCreate(void)
{
    TCGBackendConfig config = TCGBackendGetDefaultConfig();
    return TCGBackendCreateWithConfig(&config);
}

IVMMExecutionBackend *TCGBackendCreateWithConfig(TCGBackendConfig *pConfig)
{
    TCGBackend *pTcg = (TCGBackend *)calloc(1, sizeof(TCGBackend));
    if (pTcg == NULL) {
        return NULL;
    }

    /* Set up base interface */
    pTcg->base.pszName = "TCG-JIT";
    pTcg->base.pszDescription = "QEMU Tiny Code Generator with JIT compilation";
    pTcg->base.enmMode = VBOX_EXEC_MODE_TCG_JIT;
    pTcg->base.fCapabilities = VBOX_EXEC_CAP_INTERPRETER | VBOX_EXEC_CAP_JIT_COMPILATION |
                                VBOX_EXEC_CAP_SINGLE_STEP | VBOX_EXEC_CAP_DEBUG_GUEST |
                                VBOX_EXEC_CAP_NESTED_PAGING;

    /* Set up function pointers */
    pTcg->base.pfnInit = tcgBackendInit;
    pTcg->base.pfnDestroy = tcgBackendDestroy;
    pTcg->base.pfnStart = tcgBackendStart;
    pTcg->base.pfnStop = tcgBackendStop;
    pTcg->base.pfnPause = tcgBackendPause;
    pTcg->base.pfnResume = tcgBackendResume;
    pTcg->base.pfnStep = tcgBackendStep;
    pTcg->base.pfnGetCaps = tcgBackendGetCaps;
    pTcg->base.pfnIsAvailable = tcgBackendIsAvailable;
    pTcg->base.pfnAllocMem = tcgBackendAllocMem;
    pTcg->base.pfnFreeMem = tcgBackendFreeMem;
    pTcg->base.pfnRun = tcgBackendRun;
    pTcg->base.pfnHandleVMExit = tcgBackendHandleVMExit;
    pTcg->base.pfnInvalidateCache = tcgBackendInvalidateCpuCache;

    /* Copy configuration */
    if (pConfig != NULL) {
        pTcg->Config = *pConfig;
    } else {
        pTcg->Config = TCGBackendGetDefaultConfig();
    }

    pTcg->fInitialized = false;
    pTcg->fJitEnabled = !pTcg->Config.fInterpreterMode;

    return &pTcg->base;
}

void TCGBackendDestroy(IVMMExecutionBackend *pBackend)
{
    if (pBackend != NULL && pBackend->pfnDestroy != NULL) {
        pBackend->pfnDestroy(pBackend);
    }
    free(pBackend);
}

/**
 * @name TCG Backend Implementation
 */

VBoxExecResult TCGBackendInitRuntime(TCGBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnInit(&pBackend->base);
}

VBoxExecResult TCGBackendFlushCache(TCGBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_code_gen_flush(pPriv->pTcgContext);
    pPriv->cInvalidates++;

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendSetCpuType(TCGBackend *pBackend, const char *pszCpuType)
{
    if (pBackend == NULL || pszCpuType == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (pBackend->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_set_cpu_type(pPriv->pTcgContext, pszCpuType);

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendSetJitEnabled(TCGBackend *pBackend, bool fEnable)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

#if defined(VBOX_NO_EXEC_ALLOC)
    /* Cannot enable JIT in interpreter-only build */
    if (fEnable) {
        return VBOX_EXEC_ERROR_NOT_SUPPORTED;
    }
#else
    pBackend->fJitEnabled = fEnable;
    pBackend->Config.fInterpreterMode = !fEnable;
#endif

    return VBOX_EXEC_SUCCESS;
}

uint64_t TCGBackendGetCodeCacheSize(TCGBackend *pBackend)
{
    if (pBackend == NULL || pBackend->pPrivate == NULL) {
        return 0;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    return pPriv->u64CodeCacheSize;
}

void TCGBackendGetStats(TCGBackend *pBackend,
                        uint64_t *pcOps,
                        uint64_t *pcJumps,
                        uint64_t *pcInvalidates)
{
    if (pBackend == NULL) {
        return;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL) {
        return;
    }

    if (pcOps != NULL) {
        *pcOps = pPriv->cOps;
    }
    if (pcJumps != NULL) {
        *pcJumps = pPriv->cJumps;
    }
    if (pcInvalidates != NULL) {
        *pcInvalidates = pPriv->cInvalidates;
    }
}

VBoxExecResult TCGBackendRegisterMemory(TCGBackend *pBackend,
                                         void *pvStart,
                                         size_t cbSize,
                                         bool fReadOnly)
{
    if (pBackend == NULL || pvStart == NULL || cbSize == 0) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_register_memory(pPriv->pTcgContext, (uint64_t)pvStart, cbSize, fReadOnly ? 1 : 0);

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendUnregisterMemory(TCGBackend *pBackend,
                                            void *pvStart,
                                            size_t cbSize)
{
    if (pBackend == NULL || pvStart == NULL || cbSize == 0) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_unregister_memory(pPriv->pTcgContext, (uint64_t)pvStart, cbSize);

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendInvalidateRange(TCGBackend *pBackend,
                                          void *pvStart,
                                          size_t cbSize)
{
    if (pBackend == NULL || pvStart == NULL || cbSize == 0) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_invalidate_code_gen_cache_range(pPriv->pTcgContext, (uint64_t)pvStart, cbSize);
    pPriv->cInvalidates++;

    return VBOX_EXEC_SUCCESS;
}

void *TCGBackendAllocExec(TCGBackend *pBackend, size_t cbSize)
{
    if (pBackend == NULL) {
        return NULL;
    }

#if defined(VBOX_NO_EXEC_ALLOC)
    /* Cannot allocate executable memory in interpreter-only build */
    (void)cbSize;
    return NULL;
#else
    if (!pBackend->fJitEnabled) {
        return NULL;
    }

    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return NULL;
    }

    return tcg_code_gen_alloc(pPriv->pTcgContext, cbSize);
#endif
}

void TCGBackendFreeExec(TCGBackend *pBackend, void *pv, size_t cbSize)
{
    if (pBackend == NULL || pv == NULL) {
        return;
    }

#if !defined(VBOX_NO_EXEC_ALLOC)
    TCGPrivate *pPriv = (TCGPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pTcgContext == NULL) {
        return;
    }

    tcg_code_gen_free(pPriv->pTcgContext, pv, cbSize);
#else
    (void)pBackend;
    (void)cbSize;
#endif
}

void *TCGBackendAllocRam(TCGBackend *pBackend, size_t cbSize)
{
    /* Regular memory allocation - no special TCG handling needed */
    (void)pBackend;
    return malloc(cbSize);
}

void TCGBackendFreeRam(TCGBackend *pBackend, void *pv, size_t cbSize)
{
    (void)pBackend;
    (void)cbSize;
    free(pv);
}

VBoxExecResult TCGBackendRun(TCGBackend *pBackend, VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnRun(&pBackend->base, pCtx);
}

VBoxExecResult TCGBackendStep(TCGBackend *pBackend, VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnStep(&pBackend->base, pCtx);
}

VBoxExecResult TCGBackendHandleEvent(TCGBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Handle async events in TCG */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendSyncCpuState(TCGBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Sync CPU state to TCG - implementation depends on VirtualBox integration */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendRestoreCpuState(TCGBackend *pBackend, VMMExecutionContext *pCtx)
{
    /* Restore CPU state from TCG - implementation depends on VirtualBox integration */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGBackendInitQEMU(TCGBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return tcgBackendInit(&pBackend->base);
}

VBoxExecResult TCGBackendShutdownQEMU(TCGBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_SUCCESS;
    }

    return tcgBackendDestroy(&pBackend->base);
}

TCGBackendConfig TCGBackendGetDefaultConfig(void)
{
    TCGBackendConfig config = {0};
    config.u64MaxCodeCacheSize = TCG_DEFAULT_MAX_CODE_CACHE_SIZE;
    config.iOptLevel = TCG_DEFAULT_OPT_LEVEL;
    config.cThreads = TCG_DEFAULT_THREADS;
    config.fInterpreterMode = false;
    config.fDebugEnabled = false;
    config.fTracingEnabled = false;
    config.u32DirtyThreshold = TCG_DEFAULT_DIRTY_THRESHOLD;
    config.fGuestCpuFeatures = 0;

#if defined(VBOX_NO_EXEC_ALLOC)
    config.fInterpreterMode = true;
#endif

    return config;
}

/**
 * @name VMBackendsCreateTCG - Factory function for backend manager
 */
IVMMExecutionBackend *VMBackendsCreateTCG(void)
{
    return TCGBackendCreate();
}
