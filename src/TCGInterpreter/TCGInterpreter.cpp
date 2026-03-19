/**
 * @file TCGInterpreter.cpp
 * @brief TCG Interpreter Backend Implementation
 *
 * This file implements the TCGInterpreterBackend class that provides
 * software emulation WITHOUT JIT - for W^X-restricted environments
 * like UWP and Xbox One.
 *
 * @note Priority 2.1: Must have no dependency on executable memory allocation
 * @note Priority 2.1: Uses QEMU's single-step interpreter path
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 QEMU contributors
 */

#include "TCGInterpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @name Interpreter Private Implementation
 * @brief Internal structures for interpreter backend
 */

/**
 * @brief Interpreter runtime state
 */
typedef struct TCGInterpreterPrivate {
    /** @brief Interpreter context (opaque, from QEMU) */
    void *pInterpreterContext;

    /** @brief Interpreter state */
    void *pInterpreterState;

    /** @brief Total instructions executed */
    uint64_t cInstructions;

    /** @brief Total micro-ops executed */
    uint64_t cMicroOps;

    /** @brief Memory regions registered */
    void **ppvRegions;
    size_t *pcbRegions;
    uint32_t cRegions;

    /** @brief CPU state buffer */
    void *pvCpuState;

    /** @brief Instruction decode cache */
    void *pvDecodeCache;
    size_t cbDecodeCache;
} TCGInterpreterPrivate;

/**
 * @name Forward Declarations of QEMU Interpreter Functions
 * @brief These would link against libtcg_x86 from vendor/qemu/
 * @note Uses qemu/accel/tcg/tcg-accel-ops-rr.c single-step path
 */

extern "C" {

/**
 * @brief Initialize TCG in interpreter-only mode
 * @return 0 on success, non-zero on failure
 */
int tcg_interpreter_init(void);

/**
 * @brief Shutdown TCG interpreter
 */
void tcg_interpreter_exit(void);

/**
 * @brief Create interpreter context
 * @param cpus Number of CPUs
 * @return Interpreter context handle, or NULL on failure
 */
void *tcg_interpreter_context_create(int cpus);

/**
 * @brief Destroy interpreter context
 * @param ctx Interpreter context handle
 */
void tcg_interpreter_context_destroy(void *ctx);

/**
 * @brief Allocate interpreter memory (never executable)
 * @param ctx Interpreter context handle
 * @param size Size to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void *tcg_interpreter_alloc(void *ctx, size_t size);

/**
 * @brief Free interpreter memory
 * @param ctx Interpreter context handle
 * @param ptr Pointer to free
 * @param size Size that was allocated
 */
void tcg_interpreter_free(void *ctx, void *ptr, size_t size);

/**
 * @brief Execute instructions using interpreter
 * @param ctx Interpreter context handle
 * @param cpu_idx CPU index
 * @param max_cycles Maximum cycles to execute
 * @return 0 on success, non-zero on failure
 */
int tcg_interpreter_exec(void *ctx, int cpu_idx, uint64_t max_cycles);

/**
 * @brief Execute single instruction (single-step)
 * @param ctx Interpreter context handle
 * @param cpu_idx CPU index
 * @return 0 on success, non-zero on failure
 */
int tcg_interpreter_exec_step(void *ctx, int cpu_idx);

/**
 * @brief Set CPU type for interpreter
 * @param ctx Interpreter context handle
 * @param cpu_type CPU type string
 */
void tcg_interpreter_set_cpu_type(void *ctx, const char *cpu_type);

/**
 * @brief Register memory region with interpreter
 * @param ctx Interpreter context handle
 * @param start Start address
 * @param size Size of region
 * @param is_readonly Whether region is read-only
 */
void tcg_interpreter_register_memory(void *ctx, uint64_t start, uint64_t size, int is_readonly);

/**
 * @brief Unregister memory region
 * @param ctx Interpreter context handle
 * @param start Start address
 * @param size Size of region
 */
void tcg_interpreter_unregister_memory(void *ctx, uint64_t start, uint64_t size);

/**
 * @brief Get interpreter capabilities
 * @return Bitfield of interpreter capabilities
 */
uint32_t tcg_interpreter_get_capabilities(void);

} /* extern "C" */

/**
 * @name Backend Interface Implementation
 * @brief Implementation of IVMMExecutionBackend interface
 */

static VBoxExecResult interpreterBackendInit(IVMMExecutionBackend *pBackend)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Allocate private data */
    pInterp->pPrivate = (TCGInterpreterPrivate *)calloc(1, sizeof(TCGInterpreterPrivate));
    if (pInterp->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_NO_MEMORY;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;

    /* Initialize QEMU TCG interpreter subsystem */
    int rc = tcg_interpreter_init();
    if (rc != 0) {
        free(pInterp->pPrivate);
        pInterp->pPrivate = NULL;
        return VBOX_EXEC_ERROR_NOT_SUPPORTED;
    }

    /* Create interpreter context */
    pPriv->pInterpreterContext = tcg_interpreter_context_create(pInterp->Config.cThreads);
    if (pPriv->pInterpreterContext == NULL) {
        tcg_interpreter_exit();
        free(pInterp->pPrivate);
        pInterp->pPrivate = NULL;
        return VBOX_EXEC_ERROR_NO_MEMORY;
    }

    pInterp->fInitialized = true;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendDestroy(IVMMExecutionBackend *pBackend)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL) {
        return VBOX_EXEC_SUCCESS;
    }

    if (pInterp->pPrivate != NULL) {
        TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;

        /* Destroy interpreter context */
        if (pPriv->pInterpreterContext != NULL) {
            tcg_interpreter_context_destroy(pPriv->pInterpreterContext);
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

        /* Free decode cache */
        if (pPriv->pvDecodeCache != NULL) {
            free(pPriv->pvDecodeCache);
        }

        free(pInterp->pPrivate);
        pInterp->pPrivate = NULL;
    }

    /* Shutdown QEMU TCG interpreter */
    tcg_interpreter_exit();

    pInterp->fInitialized = false;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendStart(IVMMExecutionBackend *pBackend,
                                               VMMExecutionContext *pCtx)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pInterp->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Update context mode */
    pCtx->enmMode = VBOX_EXEC_MODE_TCG_INTERPRETER;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendStop(IVMMExecutionBackend *pBackend,
                                              VMMExecutionContext *pCtx)
{
    (void)pBackend;
    (void)pCtx;
    /* No cache to flush in interpreter mode */
    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendPause(IVMMExecutionBackend *pBackend,
                                               VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_PAUSED;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendResume(IVMMExecutionBackend *pBackend,
                                                VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    pCtx->enmState = VBOX_EXEC_STATE_RUNNING;

    return VBOX_EXEC_SUCCESS;
}

static VBoxExecResult interpreterBackendStep(IVMMExecutionBackend *pBackend,
                                              VMMExecutionContext *pCtx)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pInterp->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Execute single step using interpreter */
    int rc = tcg_interpreter_exec_step(pPriv->pInterpreterContext, (int)pCtx->idCpu);

    pPriv->cInstructions++;

    pCtx->enmState = VBOX_EXEC_STATE_SINGLE_STEP;

    return (rc == 0) ? VBOX_EXEC_SUCCESS : VBOX_EXEC_ERROR_CPU_ERROR;
}

static VBoxExecCapabilities interpreterBackendGetCaps(IVMMExecutionBackend *pBackend)
{
    (void)pBackend;
    /* Interpreter always supports these - NO executable memory needed */
    return VBOX_EXEC_CAP_INTERPRETER |
           VBOX_EXEC_CAP_SINGLE_STEP |
           VBOX_EXEC_CAP_DEBUG_GUEST;
}

static bool interpreterBackendIsAvailable(IVMMExecutionBackend *pBackend)
{
    /* Interpreter is always available - no hardware requirements */
    (void)pBackend;
    return true;
}

static void *interpreterBackendAllocMem(IVMMExecutionBackend *pBackend,
                                        size_t cbSize, bool fExecutable)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL) {
        return NULL;
    }

    /* Interpreter NEVER allocates executable memory */
    if (fExecutable) {
        return NULL;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return NULL;
    }

    return tcg_interpreter_alloc(pPriv->pInterpreterContext, cbSize);
}

static void interpreterBackendFreeMem(IVMMExecutionBackend *pBackend,
                                       void *pv, size_t cbSize)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL || pv == NULL) {
        return;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return;
    }

    tcg_interpreter_free(pPriv->pInterpreterContext, pv, cbSize);
}

static VBoxExecResult interpreterBackendRun(IVMMExecutionBackend *pBackend,
                                             VMMExecutionContext *pCtx)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)pBackend;

    if (pInterp == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (!pInterp->fInitialized) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pInterp->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    /* Execute interpreter - use max cycles for full execution */
    int rc = tcg_interpreter_exec(pPriv->pInterpreterContext,
                                   (int)pCtx->idCpu,
                                   pCtx->u64Ticks);

    pPriv->cInstructions++;

    return (rc == 0) ? VBOX_EXEC_SUCCESS : VBOX_EXEC_ERROR_CPU_ERROR;
}

static VBoxExecResult interpreterBackendHandleVMExit(IVMMExecutionBackend *pBackend,
                                                      VMMExecutionContext *pCtx)
{
    /* Interpreter doesn't have VM exits - this is HM-only */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_ERROR_NOT_SUPPORTED;
}

static VBoxExecResult interpreterBackendInvalidateCpuCache(IVMMExecutionBackend *pBackend,
                                                            VMMExecutionContext *pCtx)
{
    /* Interpreter doesn't have code cache to invalidate */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

/**
 * @name Interpreter Backend Factory Functions
 */

IVMMExecutionBackend *TCGInterpreterBackendCreate(void)
{
    TCGInterpreterConfig config = TCGInterpreterGetDefaultConfig();
    return TCGInterpreterBackendCreateWithConfig(&config);
}

IVMMExecutionBackend *TCGInterpreterBackendCreateWithConfig(TCGInterpreterConfig *pConfig)
{
    TCGInterpreterBackend *pInterp = (TCGInterpreterBackend *)calloc(1, sizeof(TCGInterpreterBackend));
    if (pInterp == NULL) {
        return NULL;
    }

    /* Set up base interface */
    pInterp->base.pszName = "TCG-Interpreter";
    pInterp->base.pszDescription = "QEMU Tiny Code Generator (Interpreter-only, no JIT)";
    pInterp->base.enmMode = VBOX_EXEC_MODE_TCG_INTERPRETER;
    pInterp->base.fCapabilities = VBOX_EXEC_CAP_INTERPRETER |
                                   VBOX_EXEC_CAP_SINGLE_STEP |
                                   VBOX_EXEC_CAP_DEBUG_GUEST;

    /* Set up function pointers */
    pInterp->base.pfnInit = interpreterBackendInit;
    pInterp->base.pfnDestroy = interpreterBackendDestroy;
    pInterp->base.pfnStart = interpreterBackendStart;
    pInterp->base.pfnStop = interpreterBackendStop;
    pInterp->base.pfnPause = interpreterBackendPause;
    pInterp->base.pfnResume = interpreterBackendResume;
    pInterp->base.pfnStep = interpreterBackendStep;
    pInterp->base.pfnGetCaps = interpreterBackendGetCaps;
    pInterp->base.pfnIsAvailable = interpreterBackendIsAvailable;
    pInterp->base.pfnAllocMem = interpreterBackendAllocMem;
    pInterp->base.pfnFreeMem = interpreterBackendFreeMem;
    pInterp->base.pfnRun = interpreterBackendRun;
    pInterp->base.pfnHandleVMExit = interpreterBackendHandleVMExit;
    pInterp->base.pfnInvalidateCache = interpreterBackendInvalidateCpuCache;

    /* Copy configuration */
    if (pConfig != NULL) {
        pInterp->Config = *pConfig;
    } else {
        pInterp->Config = TCGInterpreterGetDefaultConfig();
    }

    pInterp->fInitialized = false;

    return &pInterp->base;
}

void TCGInterpreterBackendDestroy(IVMMExecutionBackend *pBackend)
{
    if (pBackend != NULL && pBackend->pfnDestroy != NULL) {
        pBackend->pfnDestroy(pBackend);
    }
    free(pBackend);
}

/**
 * @name TCG Interpreter Backend Implementation
 */

VBoxExecResult TCGInterpreterInitRuntime(TCGInterpreterBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnInit(&pBackend->base);
}

VBoxExecResult TCGInterpreterSetCpuType(TCGInterpreterBackend *pBackend,
                                         const char *pszCpuType)
{
    if (pBackend == NULL || pszCpuType == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    if (pBackend->pPrivate == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv->pInterpreterContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_interpreter_set_cpu_type(pPriv->pInterpreterContext, pszCpuType);

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGInterpreterRegisterMemory(TCGInterpreterBackend *pBackend,
                                             void *pvStart,
                                             size_t cbSize,
                                             bool fReadOnly)
{
    if (pBackend == NULL || pvStart == NULL || cbSize == 0) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_interpreter_register_memory(pPriv->pInterpreterContext,
                                     (uint64_t)pvStart,
                                     cbSize,
                                     fReadOnly ? 1 : 0);

    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGInterpreterUnregisterMemory(TCGInterpreterBackend *pBackend,
                                                void *pvStart,
                                                size_t cbSize)
{
    if (pBackend == NULL || pvStart == NULL || cbSize == 0) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    tcg_interpreter_unregister_memory(pPriv->pInterpreterContext,
                                        (uint64_t)pvStart,
                                        cbSize);

    return VBOX_EXEC_SUCCESS;
}

void TCGInterpreterGetStats(TCGInterpreterBackend *pBackend,
                            uint64_t *pcInstructions,
                            uint64_t *pcMicroOps)
{
    if (pBackend == NULL) {
        return;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv == NULL) {
        return;
    }

    if (pcInstructions != NULL) {
        *pcInstructions = pPriv->cInstructions;
    }
    if (pcMicroOps != NULL) {
        *pcMicroOps = pPriv->cMicroOps;
    }
}

void *TCGInterpreterAllocMem(TCGInterpreterBackend *pBackend, size_t cbSize)
{
    if (pBackend == NULL) {
        return NULL;
    }

    /* Interpreter always succeeds - never needs executable memory */
    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        return malloc(cbSize);  /* Fall back to regular malloc */
    }

    return tcg_interpreter_alloc(pPriv->pInterpreterContext, cbSize);
}

void TCGInterpreterFreeMem(TCGInterpreterBackend *pBackend, void *pv, size_t cbSize)
{
    if (pBackend == NULL || pv == NULL) {
        return;
    }

    TCGInterpreterPrivate *pPriv = (TCGInterpreterPrivate *)pBackend->pPrivate;
    if (pPriv == NULL || pPriv->pInterpreterContext == NULL) {
        free(pv);  /* Fall back to regular free */
        return;
    }

    tcg_interpreter_free(pPriv->pInterpreterContext, pv, cbSize);
}

VBoxExecResult TCGInterpreterRun(TCGInterpreterBackend *pBackend,
                                    VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnRun(&pBackend->base, pCtx);
}

VBoxExecResult TCGInterpreterStep(TCGInterpreterBackend *pBackend,
                                     VMMExecutionContext *pCtx)
{
    if (pBackend == NULL || pCtx == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return pBackend->base.pfnStep(&pBackend->base, pCtx);
}

VBoxExecResult TCGInterpreterSyncCpuState(TCGInterpreterBackend *pBackend,
                                             VMMExecutionContext *pCtx)
{
    /* Sync CPU state to interpreter - implementation depends on VirtualBox integration */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGInterpreterRestoreCpuState(TCGInterpreterBackend *pBackend,
                                                VMMExecutionContext *pCtx)
{
    /* Restore CPU state from interpreter - implementation depends on VirtualBox integration */
    (void)pBackend;
    (void)pCtx;
    return VBOX_EXEC_SUCCESS;
}

VBoxExecResult TCGInterpreterInitQEMU(TCGInterpreterBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_ERROR_INVALID_STATE;
    }

    return interpreterBackendInit(&pBackend->base);
}

VBoxExecResult TCGInterpreterShutdownQEMU(TCGInterpreterBackend *pBackend)
{
    if (pBackend == NULL) {
        return VBOX_EXEC_SUCCESS;
    }

    return interpreterBackendDestroy(&pBackend->base);
}

TCGInterpreterConfig TCGInterpreterGetDefaultConfig(void)
{
    TCGInterpreterConfig config = {0};
    config.iOptLevel = TCG_INTERPRETER_DEFAULT_OPT_LEVEL;
    config.cThreads = TCG_INTERPRETER_DEFAULT_THREADS;
    config.fDebugEnabled = false;
    config.fTracingEnabled = false;
    config.fGuestCpuFeatures = 0;
    return config;
}

bool TCGInterpreterIsInterpreterOnly(void)
{
    /* Always true for this backend */
    return true;
}

/**
 * @name VMBackendsCreateTCGInterpreter - Factory function for backend manager
 */
IVMMExecutionBackend *VMBackendsCreateTCGInterpreter(void)
{
    return TCGInterpreterBackendCreate();
}
