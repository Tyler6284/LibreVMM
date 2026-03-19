/**
 * @file TCGBackend.h
 * @brief TCG (Tiny Code Generator) Backend for LibreVMM
 *
 * This file defines the TCGBackend class that wraps QEMU's TCG engine
 * for software JIT execution of guest code.
 *
 * Key features:
 * - Wraps QEMU's TCG engine from vendor/qemu/
 * - Implements memory allocation for JIT code generation
 * - Implements TCG runtime initialization
 * - Connects to VirtualBox's VMM as third execution mode
 *
 * @note Priority 2.1: Must work with VBOX_NO_EXEC_ALLOC for interpreter mode
 * @note Priority 2.1: Extract qemu/accel/tcg/ and qemu/target/i386/ as libtcg_x86
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 QEMU contributors
 */

#ifndef TCG_BACKEND_H
#define TCG_BACKEND_H

#include "../../include/IVMMExecutionBackend.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name TCG Backend Configuration
 * @brief Configuration options for the TCG backend
 */
typedef struct TCGBackendConfig {
    /** @brief Maximum JIT code cache size (bytes) */
    uint64_t u64MaxCodeCacheSize;

    /** @brief TCG optimization level (0-2) */
    int iOptLevel;

    /** @brief Number of TCG threads */
    uint32_t cThreads;

    /** @brief Use interpreter mode (no JIT) */
    bool fInterpreterMode;

    /** @brief Enable TCG debugging */
    bool fDebugEnabled;

    /** @brief Use TCG tracing */
    bool fTracingEnabled;

    /** @brief TCG dirty threshold */
    uint32_t u32DirtyThreshold;

    /** @brief Guest CPU features to enable */
    uint64_t fGuestCpuFeatures;
} TCGBackendConfig;

/**
 * @name Default Configuration
 */
#define TCG_DEFAULT_MAX_CODE_CACHE_SIZE (128 * 1024 * 1024)  /**< 128 MB */
#define TCG_DEFAULT_OPT_LEVEL 2
#define TCG_DEFAULT_THREADS 1
#define TCG_DEFAULT_DIRTY_THRESHOLD 4096

/**
 * @name TCG Backend Opaque Structure
 */
typedef struct TCGBackendPrivate TCGBackendPrivate;

/**
 * @brief TCG Backend for software JIT execution
 *
 * The TCGBackend wraps QEMU's Tiny Code Generator to provide software
 * emulation of guest code. It supports both JIT compilation and
 * interpreter-only modes.
 *
 * When compiled with VBOX_NO_EXEC_ALLOC, JIT compilation is disabled
 * and the backend operates in interpreter-only mode.
 */
typedef struct TCGBackend {
    /** @brief Base execution backend interface */
    IVMMExecutionBackend base;

    /** @brief Backend configuration */
    TCGBackendConfig Config;

    /** @brief Whether JIT is enabled */
    bool fJitEnabled;

    /** @brief Whether TCG is initialized */
    bool fInitialized;

    /** @brief Opaque private data */
    TCGBackendPrivate *pPrivate;
} TCGBackend;

/**
 * @name TCG Backend Functions
 * @brief Functions for creating and managing TCG backend
 */

/**
 * @brief Create a new TCG backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *TCGBackendCreate(void);

/**
 * @brief Create a TCG backend with custom configuration
 * @param pConfig Pointer to configuration structure
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *TCGBackendCreateWithConfig(TCGBackendConfig *pConfig);

/**
 * @brief Destroy a TCG backend instance
 * @param pBackend Pointer to backend to destroy
 */
void TCGBackendDestroy(IVMMExecutionBackend *pBackend);

/**
 * @name TCG-Specific Functions
 * @brief Functions specific to TCG backend operation
 */

/**
 * @brief Initialize TCG runtime
 * @param pBackend Pointer to TCG backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendInitRuntime(TCGBackend *pBackend);

/**
 * @brief Flush TCG code cache
 * @param pBackend Pointer to TCG backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendFlushCache(TCGBackend *pBackend);

/**
 * @brief Set guest CPU type
 * @param pBackend Pointer to TCG backend
 * @param pszCpuType CPU type string (e.g., "qemu64", "qemu32", "host")
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendSetCpuType(TCGBackend *pBackend, const char *pszCpuType);

/**
 * @brief Enable or disable JIT compilation
 * @param pBackend Pointer to TCG backend
 * @param fEnable true to enable JIT, false for interpreter-only
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendSetJitEnabled(TCGBackend *pBackend, bool fEnable);

/**
 * @brief Get current code cache size
 * @param pBackend Pointer to TCG backend
 * @return Current cache size in bytes
 */
uint64_t TCGBackendGetCodeCacheSize(TCGBackend *pBackend);

/**
 * @brief Get TCG statistics
 * @param pBackend Pointer to TCG backend
 * @param pcOps pointer to store opcode count
 * @param pcJumps pointer to store jump count
 * @param pcInvalidates pointer to store invalidation count
 */
void TCGBackendGetStats(TCGBackend *pBackend,
                        uint64_t *pcOps,
                        uint64_t *pcJumps,
                        uint64_t *pcInvalidates);

/**
 * @brief Register memory region with TCG
 * @param pBackend Pointer to TCG backend
 * @param pvStart Start address
 * @param cbSize Size of region
 * @param fReadOnly Whether region is read-only
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendRegisterMemory(TCGBackend *pBackend,
                                        void *pvStart,
                                        size_t cbSize,
                                        bool fReadOnly);

/**
 * @brief Unregister memory region from TCG
 * @param pBackend Pointer to TCG backend
 * @param pvStart Start address
 * @param cbSize Size of region
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendUnregisterMemory(TCGBackend *pBackend,
                                           void *pvStart,
                                           size_t cbSize);

/**
 * @brief Invalidate TCG code cache for a memory range
 * @param pBackend Pointer to TCG backend
 * @param pvStart Start address of invalidated range
 * @param cbSize Size of invalidated range
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendInvalidateRange(TCGBackend *pBackend,
                                          void *pvStart,
                                          size_t cbSize);

/**
 * @name TCG Memory Management
 * @brief Functions for managing TCG memory allocation
 */

/**
 * @brief Allocate executable memory for JIT
 * @param pBackend Pointer to TCG backend
 * @param cbSize Size to allocate
 * @return Pointer to allocated memory, or NULL on failure
 *
 * @note Returns NULL if VBOX_NO_EXEC_ALLOC is defined (interpreter mode)
 */
void *TCGBackendAllocExec(TCGBackend *pBackend, size_t cbSize);

/**
 * @brief Free executable memory
 * @param pBackend Pointer to TCG backend
 * @param pv Pointer to memory to free
 * @param cbSize Size that was allocated
 */
void TCGBackendFreeExec(TCGBackend *pBackend, void *pv, size_t cbSize);

/**
 * @brief Allocate regular memory
 * @param pBackend Pointer to TCG backend
 * @param cbSize Size to allocate
 * @return Pointer to allocated memory, or NULL on failure
 */
void *TCGBackendAllocRam(TCGBackend *pBackend, size_t cbSize);

/**
 * @brief Free regular memory
 * @param pBackend Pointer to TCG backend
 * @param pv Pointer to memory to free
 * @param cbSize Size that was allocated
 */
void TCGBackendFreeRam(TCGBackend *pBackend, void *pv, size_t cbSize);

/**
 * @name TCG Runtime Functions
 * @brief Functions that interface with QEMU's TCG runtime
 */

/**
 * @brief Execute TCG code
 * @param pBackend Pointer to TCG backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendRun(TCGBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Execute single TCG instruction (single-step)
 * @param pBackend Pointer to TCG backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendStep(TCGBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Handle TCG async event
 * @param pBackend Pointer to TCG backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendHandleEvent(TCGBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @name CPU State Management
 * @brief Functions for managing CPU state in TCG
 */

/**
 * @brief Synchronize CPU state to TCG
 * @param pBackend Pointer to TCG backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendSyncCpuState(TCGBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @brief Synchronize CPU state from TCG
 * @param pBackend Pointer to TCG backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendRestoreCpuState(TCGBackend *pBackend, VMMExecutionContext *pCtx);

/**
 * @name TCG Initialization
 * @brief Functions for TCG runtime initialization
 */

/**
 * @brief Initialize QEMU TCG subsystem
 * @param pBackend Pointer to TCG backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendInitQEMU(TCGBackend *pBackend);

/**
 * @brief Shutdown QEMU TCG subsystem
 * @param pBackend Pointer to TCG backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGBackendShutdownQEMU(TCGBackend *pBackend);

/**
 * @brief Get default TCG configuration
 * @return Default configuration structure
 */
TCGBackendConfig TCGBackendGetDefaultConfig(void);

#endif /* TCG_BACKEND_H */
