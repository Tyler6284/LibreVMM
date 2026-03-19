/**
 * @file VMMExecutionBackends.h
 * @brief Execution Backend Manager for LibreVMM
 *
 * This file defines the VMMExecutionBackendMgr class that manages all VM
 * execution backends and handles backend selection based on host capabilities.
 *
 * Key principles:
 * - NO SILENT FALLBACK: If requested backend fails, report failure and halt
 * - User must explicitly consent to any fallback from HM to TCG
 * - All backends are first-class citizens
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 */

#ifndef VMM_EXECUTION_BACKENDS_H
#define VMM_EXECUTION_BACKENDS_H

#include "IVMMExecutionBackend.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name Maximum number of registered backends
 */
#define VMM_EXEC_BACKEND_MAX 4

/**
 * @name Backend Selection Flags
 * @brief Flags that affect backend selection behavior
 */
typedef enum VMMBackendSelFlags {
    VMM_BACKEND_SEL_NONE = 0,            /**< No special selection flags */
    VMM_BACKEND_SEL_FORCE_HM = 1,        /**< Force hardware virtualization */
    VMM_BACKEND_SEL_FORCE_TCG_JIT = 2,   /**< Force TCG JIT mode */
    VMM_BACKEND_SEL_FORCE_TCG_INTERP = 4,/**< Force TCG interpreter mode */
    VMM_BACKEND_SEL_ALLOW_FALLBACK = 8,  /**< Allow fallback on failure (requires user consent) */
    VMM_BACKEND_SEL_REQUIRE_HM = 16,     /**< Require HM, fail if unavailable */
    VMM_BACKEND_SEL_NO_EXEC_ALLOC = 32   /**< Prefer no-exec-alloc mode (UWP/Xbox) */
} VMMBackendSelFlags;

/**
 * @name Backend Status
 * @brief Current status of a backend
 */
typedef enum VMMBackendStatus {
    VMM_BACKEND_STATUS_UNINITIALIZED = 0,   /**< Backend not yet initialized */
    VMM_BACKEND_STATUS_AVAILABLE = 1,        /**< Backend is available on this host */
    VMM_BACKEND_STATUS_UNAVAILABLE = 2,      /**< Backend is not available */
    VMM_BACKEND_STATUS_INITIALIZED = 3,     /**< Backend is initialized */
    VMM_BACKEND_STATUS_ACTIVE = 4,          /**< Backend is currently active */
    VMM_BACKEND_STATUS_FAILED = 5,          /**< Backend failed to initialize */
    VMM_BACKEND_STATUS_DESTROYED = 6         /**< Backend has been destroyed */
} VMMBackendStatus;

/**
 * @name Backend Information Structure
 * @brief Contains information about a registered backend
 */
typedef struct VMMBackendInfo {
    IVMMExecutionBackend *pBackend;      /**< Pointer to backend instance */
    VMMBackendStatus enmStatus;           /**< Current status */
    VBoxExecCapabilities fCaps;           /**< Cached capabilities */
    bool fAvailable;                      /**< Whether backend is available */
} VMMBackendInfo;

/**
 * @name Execution Backend Manager
 * @brief Manages all execution backends and handles selection
 *
 * The VMMExecutionBackendMgr is responsible for:
 * - Registering all available execution backends
 * - Detecting host capabilities
 * - Selecting appropriate backend based on user preferences
 * - NEVER performing silent fallback (Priority 2.1 requirement)
 */
typedef struct VMMExecutionBackendMgr {
    /** @brief Registered backends */
    VMMBackendInfo aBackends[VMM_EXEC_BACKEND_MAX];

    /** @brief Number of registered backends */
    uint32_t cBackends;

    /** @brief Currently active backend index */
    int32_t iActiveBackend;

    /** @brief Last selected execution mode */
    VBoxExecMode enmLastMode;

    /** @brief Selection flags */
    uint32_t fSelFlags;

    /** @brief Whether initialization is complete */
    bool fInitialized;

    /** @brief Error message from last failure */
    char szErrorMessage[512];
} VMMExecutionBackendMgr;

/**
 * @name Backend Manager Functions
 * @brief Functions for managing execution backends
 */

/**
 * @brief Create a new backend manager instance
 * @return Pointer to new manager, or NULL on failure
 */
VMMExecutionBackendMgr *VMMBackendMgrCreate(void);

/**
 * @brief Destroy a backend manager instance
 * @param pMgr Pointer to manager to destroy
 */
void VMMBackendMgrDestroy(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Initialize the backend manager and register all backends
 * @param pMgr Pointer to the manager
 * @return VBoxExecResult indicating success or failure
 *
 * @note This registers HM, TCG JIT, and TCG Interpreter backends
 */
VBoxExecResult VMMBackendMgrInit(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Detect available backends on this host
 * @param pMgr Pointer to the manager
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrDetectCapabilities(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Select an execution backend
 * @param pMgr Pointer to the manager
 * @param enmMode Desired execution mode
 * @param fFlags Selection flags
 * @return Pointer to selected backend, or NULL on failure
 *
 * @note NEVER performs silent fallback - if requested mode is unavailable,
 *       this function returns NULL and sets an error message
 */
IVMMExecutionBackend *VMMBackendMgrSelectBackend(VMMExecutionBackendMgr *pMgr,
                                                   VBoxExecMode enmMode,
                                                   uint32_t fFlags);

/**
 * @brief Request backend with explicit fallback consent
 * @param pMgr Pointer to the manager
 * @param enmPreferredMode Preferred execution mode
 * @param enmFallbackMode Fallback mode if preferred fails
 * @param fFlags Selection flags
 * @return Pointer to selected backend, or NULL on failure
 *
 * @note This is the ONLY way to get fallback behavior - user must explicitly
 *       consent to fallback via VMM_BACKEND_SEL_ALLOW_FALLBACK flag
 */
IVMMExecutionBackend *VMMBackendMgrSelectBackendWithFallback(VMMExecutionBackendMgr *pMgr,
                                                               VBoxExecMode enmPreferredMode,
                                                               VBoxExecMode enmFallbackMode,
                                                               uint32_t fFlags);

/**
 * @brief Start execution with selected backend
 * @param pMgr Pointer to the manager
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrStart(VMMExecutionBackendMgr *pMgr,
                                   VMMExecutionContext *pCtx);

/**
 * @brief Stop execution
 * @param pMgr Pointer to the manager
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrStop(VMMExecutionBackendMgr *pMgr,
                                  VMMExecutionContext *pCtx);

/**
 * @brief Pause execution
 * @param pMgr Pointer to the manager
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrPause(VMMExecutionBackendMgr *pMgr,
                                    VMMExecutionContext *pCtx);

/**
 * @brief Resume execution
 * @param pMgr Pointer to the manager
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrResume(VMMExecutionBackendMgr *pMgr,
                                     VMMExecutionContext *pCtx);

/**
 * @brief Execute single step
 * @param pMgr Pointer to the manager
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
VBoxExecResult VMMBackendMgrStep(VMMExecutionBackendMgr *pMgr,
                                   VMMExecutionContext *pCtx);

/**
 * @brief Get the currently active backend
 * @param pMgr Pointer to the manager
 * @return Pointer to active backend, or NULL if none active
 */
IVMMExecutionBackend *VMMBackendMgrGetActive(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Get the current execution mode
 * @param pMgr Pointer to the manager
 * @return Current execution mode
 */
VBoxExecMode VMMBackendMgrGetMode(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Get capabilities of a specific mode
 * @param pMgr Pointer to the manager
 * @param enmMode Execution mode to query
 * @return Capabilities bitfield
 */
VBoxExecCapabilities VMMBackendMgrGetModeCaps(VMMExecutionBackendMgr *pMgr,
                                                VBoxExecMode enmMode);

/**
 * @brief Check if a mode is available
 * @param pMgr Pointer to the manager
 * @param enmMode Execution mode to check
 * @return true if available, false otherwise
 */
bool VMMBackendMgrIsModeAvailable(VMMExecutionBackendMgr *pMgr,
                                    VBoxExecMode enmMode);

/**
 * @brief Get the last error message
 * @param pMgr Pointer to the manager
 * @return Pointer to error message string
 */
const char *VMMBackendMgrGetError(VMMExecutionBackendMgr *pMgr);

/**
 * @brief Log a backend-related message
 * @param pMgr Pointer to the manager
 * @param pszFormat Format string
 * @param ... Additional arguments
 */
void VMMBackendMgrLog(VMMExecutionBackendMgr *pMgr,
                       const char *pszFormat, ...);

/**
 * @name Host Capability Detection
 * @brief Functions to detect host hardware capabilities
 */

/**
 * @brief Detect if hardware virtualization is available
 * @return true if VT-x/AMD-V is available, false otherwise
 */
bool VMMHostCapabilitiesDetectHMVirtualization(void);

/**
 * @brief Detect if nested paging is available
 * @return true if nested paging is available, false otherwise
 */
bool VMMHostCapabilitiesDetectNestedPaging(void);

/**
 * @brief Detect if unrestricted guest is available
 * @return true if unrestricted guest is available, false otherwise
 */
bool VMMHostCapabilitiesDetectUnrestrictedGuest(void);

/**
 * @brief Detect large page support
 * @return true if large pages are available, false otherwise
 */
bool VMMHostCapabilitiesDetectLargePages(void);

/**
 * @brief Detect SEV support (AMD)
 * @return true if AMD SEV is available, false otherwise
 */
bool VMMHostCapabilitiesDetectSEV(void);

/**
 * @brief Get all host execution capabilities
 * @return Bitfield of all host capabilities
 */
VBoxExecCapabilities VMMHostCapabilitiesGetAll(void);

/**
 * @name Utility Functions
 * @brief Helper functions for backend management
 */

/**
 * @brief Get the backend index for a mode
 * @param enmMode Execution mode
 * @return Backend index, or -1 if invalid
 */
int32_t VMMBackendGetIndexForMode(VBoxExecMode enmMode);

/**
 * @brief Get the mode name for error reporting
 * @param enmMode Execution mode
 * @return Human-readable mode name
 */
const char *VMMBackendGetModeName(VBoxExecMode enmMode);

#endif /* VMM_EXECUTION_BACKENDS_H */
