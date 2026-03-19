/**
 * @file TCGInterpreter.h
 * @brief TCG Interpreter Backend for LibreVMM
 *
 * This file defines the TCGInterpreterBackend class that provides
 * software emulation WITHOUT JIT - for W^X-restricted environments
 * like UWP and Xbox One.
 *
 * Key features:
 * - No JIT code generation
 * - Works when VBOX_NO_EXEC_ALLOC is defined
 * - Uses QEMU's single-step interpreter path (qemu/accel/tcg/tcg-accel-ops-rr.c)
 *
 * @note Priority 2.1: Must have no dependency on executable memory allocation
 * @note Priority 2.1: For W^X-restricted environments like UWP
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 * Copyright (C) 2005-2024 QEMU contributors
 */

#ifndef TCG_INTERPRETER_H
#define TCG_INTERPRETER_H

#include "../../include/IVMMExecutionBackend.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name TCG Interpreter Configuration
 * @brief Configuration options for the TCG interpreter backend
 */
typedef struct TCGInterpreterConfig {
    /** @brief Interpreter optimization level (0-2) */
    int iOptLevel;

    /** @brief Number of interpreter threads */
    uint32_t cThreads;

    /** @brief Enable interpreter debugging */
    bool fDebugEnabled;

    /** @brief Use interpreter tracing */
    bool fTracingEnabled;

    /** @brief Guest CPU features to enable */
    uint64_t fGuestCpuFeatures;
} TCGInterpreterConfig;

/**
 * @name Default Configuration
 */
#define TCG_INTERPRETER_DEFAULT_OPT_LEVEL 1
#define TCG_INTERPRETER_DEFAULT_THREADS 1

/**
 * @name TCG Interpreter Backend Opaque Structure
 */
typedef struct TCGInterpreterPrivate TCGInterpreterPrivate;

/**
 * @brief TCG Interpreter Backend for W^X-restricted environments
 *
 * The TCGInterpreterBackend provides software emulation using QEMU's
 * interpreter mode. This backend has NO dependency on executable memory
 * allocation and works in W^X-restricted environments.
 *
 * This is the only backend that should be used when VBOX_NO_EXEC_ALLOC is defined.
 */
typedef struct TCGInterpreterBackend {
    /** @brief Base execution backend interface */
    IVMMExecutionBackend base;

    /** @brief Interpreter configuration */
    TCGInterpreterConfig Config;

    /** @brief Whether interpreter is initialized */
    bool fInitialized;

    /** @brief Opaque private data */
    TCGInterpreterPrivate *pPrivate;
} TCGInterpreterBackend;

/**
 * @name Interpreter Backend Functions
 * @brief Functions for creating and managing interpreter backend
 */

/**
 * @brief Create a new TCG interpreter backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *TCGInterpreterBackendCreate(void);

/**
 * @brief Create a TCG interpreter backend with custom configuration
 * @param pConfig Pointer to configuration structure
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *TCGInterpreterBackendCreateWithConfig(TCGInterpreterConfig *pConfig);

/**
 * @brief Destroy a TCG interpreter backend instance
 * @param pBackend Pointer to backend to destroy
 */
void TCGInterpreterBackendDestroy(IVMMExecutionBackend *pBackend);

/**
 * @name TCG Interpreter-Specific Functions
 * @brief Functions specific to interpreter backend operation
 */

/**
 * @brief Initialize interpreter runtime
 * @param pBackend Pointer to interpreter backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterInitRuntime(TCGInterpreterBackend *pBackend);

/**
 * @brief Set guest CPU type
 * @param pBackend Pointer to interpreter backend
 * @param pszCpuType CPU type string
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterSetCpuType(TCGInterpreterBackend *pBackend,
                                         const char *pszCpuType);

/**
 * @brief Register memory region with interpreter
 * @param pBackend Pointer to interpreter backend
 * @param pvStart Start address
 * @param cbSize Size of region
 * @param fReadOnly Whether region is read-only
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterRegisterMemory(TCGInterpreterBackend *pBackend,
                                             void *pvStart,
                                             size_t cbSize,
                                             bool fReadOnly);

/**
 * @brief Unregister memory region from interpreter
 * @param pBackend Pointer to interpreter backend
 * @param pvStart Start address
 * @param cbSize Size of region
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterUnregisterMemory(TCGInterpreterBackend *pBackend,
                                                void *pvStart,
                                                size_t cbSize);

/**
 * @brief Get interpreter statistics
 * @param pBackend Pointer to interpreter backend
 * @param pcInstructions pointer to store instruction count
 * @param pcMicroOps pointer to store micro-op count
 */
void TCGInterpreterGetStats(TCGInterpreterBackend *pBackend,
                            uint64_t *pcInstructions,
                            uint64_t *pcMicroOps);

/**
 * @name Interpreter Memory Management
 * @brief Functions for managing interpreter memory (no executable memory needed)
 */

/**
 * @brief Allocate memory for interpreter
 * @param pBackend Pointer to interpreter backend
 * @param cbSize Size to allocate
 * @return Pointer to allocated memory, or NULL on failure
 *
 * @note This ALWAYS succeeds - interpreter does not need executable memory
 */
void *TCGInterpreterAllocMem(TCGInterpreterBackend *pBackend, size_t cbSize);

/**
 * @brief Free interpreter-allocated memory
 * @param pBackend Pointer to interpreter backend
 * @param pv Pointer to memory to free
 * @param cbSize Size that was allocated
 */
void TCGInterpreterFreeMem(TCGInterpreterBackend *pBackend, void *pv, size_t cbSize);

/**
 * @name Interpreter Execution Functions
 * @brief Functions for running interpreter
 */

/**
 * @brief Execute instructions using interpreter
 * @param pBackend Pointer to interpreter backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterRun(TCGInterpreterBackend *pBackend,
                                   VMMExecutionContext *pCtx);

/**
 * @brief Execute single instruction (single-step)
 * @param pBackend Pointer to interpreter backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterStep(TCGInterpreterBackend *pBackend,
                                    VMMExecutionContext *pCtx);

/**
 * @name CPU State Management
 * @brief Functions for managing CPU state in interpreter
 */

/**
 * @brief Synchronize CPU state to interpreter
 * @param pBackend Pointer to interpreter backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterSyncCpuState(TCGInterpreterBackend *pBackend,
                                            VMMExecutionContext *pCtx);

/**
 * @brief Synchronize CPU state from interpreter
 * @param pBackend Pointer to interpreter backend
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterRestoreCpuState(TCGInterpreterBackend *pBackend,
                                               VMMExecutionContext *pCtx);

/**
 * @name Interpreter Initialization
 * @brief Functions for interpreter runtime initialization
 */

/**
 * @brief Initialize QEMU TCG interpreter subsystem
 * @param pBackend Pointer to interpreter backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterInitQEMU(TCGInterpreterBackend *pBackend);

/**
 * @brief Shutdown QEMU TCG interpreter subsystem
 * @param pBackend Pointer to interpreter backend
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult TCGInterpreterShutdownQEMU(TCGInterpreterBackend *pBackend);

/**
 * @brief Get default interpreter configuration
 * @return Default configuration structure
 */
TCGInterpreterConfig TCGInterpreterGetDefaultConfig(void);

/**
 * @name Check Functions
 * @brief Utility functions to check interpreter capabilities
 */

/**
 * @brief Check if running in interpreter-only mode
 * @return true if interpreter-only mode is active
 */
bool TCGInterpreterIsInterpreterOnly(void);

#endif /* TCG_INTERPRETER_H */
