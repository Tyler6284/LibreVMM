/**
 * @file IVMMExecutionBackend.h
 * @brief Execution backend abstraction interface for LibreVMM
 *
 * This file defines the IVMMExecutionBackend interface that provides a unified
 * abstraction layer for different VM execution modes:
 * - HMBackend: Hardware virtualization (VT-x/AMD-V)
 * - TCGBackend: QEMU software JIT (Tiny Code Generator)
 * - TCGInterpreterBackend: No-JIT interpreter mode for W^X-restricted environments
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (C) 2024 LibreVMM contributors
 */

#ifndef IVMM_EXECUTION_BACKEND_H
#define IVMM_EXECUTION_BACKEND_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @name Execution Mode Enumeration
 * @brief Defines the available VM execution modes
 *
 * These modes correspond to different ways of executing guest code:
 * - VBOX_EXEC_MODE_HM: Hardware-assisted virtualization using VT-x/AMD-V
 * - VBOX_EXEC_MODE_TCG_JIT: Software emulation with JIT code generation
 * - VBOX_EXEC_MODE_TCG_INTERPRETER: Software emulation without JIT (interpreter-only)
 *
 * @note Priority 2.1: Remove forced fallback from HM to TCG without user consent
 */
typedef enum VBoxExecMode {
    VBOX_EXEC_MODE_HM = 0,              /**< Hardware virtualization (VT-x/AMD-V) */
    VBOX_EXEC_MODE_TCG_JIT = 1,          /**< TCG with JIT code generation */
    VBOX_EXEC_MODE_TCG_INTERPRETER = 2,  /**< TCG interpreter-only (no JIT) */
    VBOX_EXEC_MODE_MAX = 3               /**< Maximum value for bounds checking */
} VBoxExecMode;

/**
 * @name Execution State Enumeration
 * @brief Defines the possible VM execution states
 */
typedef enum VBoxExecState {
    VBOX_EXEC_STATE_STOPPED = 0,   /**< VM is not running */
    VBOX_EXEC_STATE_RUNNING = 1,    /**< VM is actively executing */
    VBOX_EXEC_STATE_PAUSED = 2,     /**< VM is paused */
    VBOX_EXEC_STATE_SINGLE_STEP = 3 /**< VM is in single-step mode */
} VBoxExecState;

/**
 * @name Execution Control Flags
 * @brief Flags that modify execution behavior
 */
typedef enum VBoxExecFlags {
    VBOX_EXEC_FLAG_NONE = 0,            /**< No special flags */
    VBOX_EXEC_FLAG_SINGLE_STEP = 1,     /**< Execute single instruction */
    VBOX_EXEC_FLAG_TRACE = 2,           /**< Enable execution tracing */
    VBOX_EXEC_FLAG_DEBUG = 4,           /**< Enable debug mode */
    VBOX_EXEC_FLAG_NESTED_PAGING = 8,   /**< Enable nested paging (HM only) */
    VBOX_EXEC_FLAG_UNRESTRICTED_GUEST = 16 /**< Enable unrestricted guest (HM only) */
} VBoxExecFlags;

/**
 * @name Execution Result Codes
 * @brief Return codes for execution operations
 */
typedef enum VBoxExecResult {
    VBOX_EXEC_SUCCESS = 0,               /**< Operation completed successfully */
    VBOX_EXEC_ERROR_NOT_SUPPORTED = -1,  /**< Execution mode not supported */
    VBOX_EXEC_ERROR_HARDWARE_UNAVAILABLE = -2, /**< Required hardware unavailable */
    VBOX_EXEC_ERROR_NO_MEMORY = -3,      /**< Memory allocation failed */
    VBOX_EXEC_ERROR_INVALID_STATE = -4,  /**< Invalid execution state */
    VBOX_EXEC_ERROR_VMEXIT = -5,         /**< VM exit occurred (HM) */
    VBOX_EXEC_ERROR_CPU_ERROR = -6,      /**< CPU error occurred */
    VBOX_EXEC_ERROR_PERMISSION_DENIED = -7 /**< Permission denied */
} VBoxExecResult;

/**
 * @name Forward Declarations
 */
struct IVMMExecutionBackend;
struct VMMExecutionContext;

/**
 * @brief Execution backend capabilities
 *
 * Bitfield describing what features an execution backend supports.
 */
typedef uint32_t VBoxExecCapabilities;

/* Capability flags */
#define VBOX_EXEC_CAP_HW_VIRTUALIZATION    0x0001  /**< Supports hardware virtualization */
#define VBOX_EXEC_CAP_JIT_COMPILATION      0x0002  /**< Supports JIT code generation */
#define VBOX_EXEC_CAP_INTERPRETER          0x0004  /**< Supports interpreter mode */
#define VBOX_EXEC_CAP_NESTED_PAGING        0x0008  /**< Supports nested paging */
#define VBOX_EXEC_CAP_NESTED_VIRTUALIZATION 0x0010 /**< Supports nested virtualization */
#define VBOX_EXEC_CAP_UNRESTRICTED_GUEST   0x0040  /**< Supports unrestricted guest (HM only) */
#define VBOX_EXEC_CAP_SINGLE_STEP          0x0020  /**< Supports single-step execution */
#define VBOX_EXEC_CAP_DEBUG_GUEST          0x0040  /**< Supports guest debugging */
#define VBOX_EXEC_CAP_MEMORY_PROTECTION    0x0080  /**< Supports memory protection */
#define VBOX_EXEC_CAP_LARGE_PAGES          0x0100  /**< Supports large pages */
#define VBOX_EXEC_CAP_UEFI                 0x0200  /**< Supports UEFI */
#define VBOX_EXEC_CAP_SEV                  0x0400  /**< Supports AMD SEV */

/**
 * @brief VM execution context structure
 *
 * Contains all information needed to execute a VM, including CPU state,
 * memory mappings, and execution parameters.
 */
typedef struct VMMExecutionContext {
    void *pVM;                    /**< Pointer to VirtualBox VM structure */
    void *pVCpu;                  /**< Pointer to virtual CPU structure */
    uint64_t idCpu;               /**< CPU ID */
    VBoxExecMode enmMode;         /**< Current execution mode */
    VBoxExecState enmState;       /**< Current execution state */
    uint64_t u64Ticks;            /**< Execution ticks/cycles */
    uint32_t fFlags;              /**< Execution flags */
} VMMExecutionContext;

/**
 * @name Backend Operations Function Pointers
 * @brief Function pointer types for backend operations
 */

/**
 * @brief Initialize the execution backend
 * @param pBackend Pointer to the backend instance
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendInit)(struct IVMMExecutionBackend *pBackend);

/**
 * @brief Destroy the execution backend
 * @param pBackend Pointer to the backend instance
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendDestroy)(struct IVMMExecutionBackend *pBackend);

/**
 * @brief Start VM execution
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendStart)(struct IVMMExecutionBackend *pBackend,
                                           VMMExecutionContext *pCtx);

/**
 * @brief Stop VM execution
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendStop)(struct IVMMExecutionBackend *pBackend,
                                          VMMExecutionContext *pCtx);

/**
 * @brief Pause VM execution
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendPause)(struct IVMMExecutionBackend *pBackend,
                                           VMMExecutionContext *pCtx);

/**
 * @brief Resume VM execution
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendResume)(struct IVMMExecutionBackend *pBackend,
                                             VMMExecutionContext *pCtx);

/**
 * @brief Execute a single step
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendStep)(struct IVMMExecutionBackend *pBackend,
                                           VMMExecutionContext *pCtx);

/**
 * @brief Get backend capabilities
 * @param pBackend Pointer to the backend instance
 * @return Bitfield of supported capabilities
 */
typedef VBoxExecCapabilities (*PFNBackendGetCaps)(struct IVMMExecutionBackend *pBackend);

/**
 * @brief Check if backend is available on this host
 * @param pBackend Pointer to the backend instance
 * @return true if available, false otherwise
 */
typedef bool (*PFNBackendIsAvailable)(struct IVMMExecutionBackend *pBackend);

/**
 * @brief Allocate memory for the backend
 * @param pBackend Pointer to the backend instance
 * @param cbSize Size to allocate
 * @param fExecutable Whether the memory should be executable (for JIT)
 * @return Pointer to allocated memory, or NULL on failure
 */
typedef void* (*PFNBackendAllocMem)(struct IVMMExecutionBackend *pBackend,
                                      size_t cbSize, bool fExecutable);

/**
 * @brief Free backend-allocated memory
 * @param pBackend Pointer to the backend instance
 * @param pv Pointer to memory to free
 * @param cbSize Size that was allocated
 */
typedef void (*PFNBackendFreeMem)(struct IVMMExecutionBackend *pBackend,
                                    void *pv, size_t cbSize);

/**
 * @brief Execute guest code
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendRun)(struct IVMMExecutionBackend *pBackend,
                                         VMMExecutionContext *pCtx);

/**
 * @brief Handle a VM exit (for hardware virtualization)
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating how to proceed
 */
typedef VBoxExecResult (*PFNBackendHandleVMExit)(struct IVMMExecutionBackend *pBackend,
                                                   VMMExecutionContext *pCtx);

/**
 * @brief Invalidate CPU internal caches
 * @param pBackend Pointer to the backend instance
 * @param pCtx Execution context
 * @return VBoxExecResult indicating success or failure
 */
typedef VBoxExecResult (*PFNBackendInvalidateCpuCache)(struct IVMMExecutionBackend *pBackend,
                                                         VMMExecutionContext *pCtx);

/**
 * @brief Interface for VM execution backends
 *
 * This abstract interface defines the contract that all execution backends
 * must implement. Each backend provides a different mechanism for executing
 * guest code:
 *
 * - HMBackend: Uses hardware virtualization (VT-x/AMD-V) for near-native performance
 * - TCGBackend: Uses QEMU's Tiny Code Generator with JIT compilation
 * - TCGInterpreterBackend: Uses TCG in interpreter mode (no JIT) for restricted environments
 *
 * @note Priority 2.1: TCG backend must have no dependency on executable memory
 *       allocation when built in interpreter mode (VBOX_NO_EXEC_ALLOC flag)
 */
typedef struct IVMMExecutionBackend {
    /** @brief Backend name (e.g., "HM", "TCG-JIT", "TCG-Interpreter") */
    const char *pszName;

    /** @brief Backend description */
    const char *pszDescription;

    /** @brief Execution mode this backend implements */
    VBoxExecMode enmMode;

    /** @brief Supported capabilities */
    VBoxExecCapabilities fCapabilities;

    /** @brief Initialize the backend */
    PFNBackendInit pfnInit;

    /** @brief Destroy the backend */
    PFNBackendDestroy pfnDestroy;

    /** @brief Start execution */
    PFNBackendStart pfnStart;

    /** @brief Stop execution */
    PFNBackendStop pfnStop;

    /** @brief Pause execution */
    PFNBackendPause pfnPause;

    /** @brief Resume execution */
    PFNBackendResume pfnResume;

    /** @brief Execute single step */
    PFNBackendStep pfnStep;

    /** @brief Get capabilities */
    PFNBackendGetCaps pfnGetCaps;

    /** @brief Check availability */
    PFNBackendIsAvailable pfnIsAvailable;

    /** @brief Allocate memory */
    PFNBackendAllocMem pfnAllocMem;

    /** @brief Free memory */
    PFNBackendFreeMem pfnFreeMem;

    /** @brief Run guest code */
    PFNBackendRun pfnRun;

    /** @brief Handle VM exit (HM only) */
    PFNBackendHandleVMExit pfnHandleVMExit;

    /** @brief Invalidate CPU cache */
    PFNBackendInvalidateCpuCache pfnInvalidateCache;

    /** @brief Opaque backend-specific data */
    void *pvPrivate;
} IVMMExecutionBackend;

/**
 * @name Backend Factory Functions
 * @brief Functions to create and destroy backend instances
 */

/**
 * @brief Create an HM (Hardware Memory) backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *VMBackendsCreateHM(void);

/**
 * @brief Create a TCG JIT backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *VMBackendsCreateTCG(void);

/**
 * @brief Create a TCG Interpreter backend instance
 * @return Pointer to new backend, or NULL on failure
 */
IVMMExecutionBackend *VMBackendsCreateTCGInterpreter(void);

/**
 * @brief Destroy an execution backend instance
 * @param pBackend Pointer to backend to destroy
 */
void VMBackendsDestroy(IVMMExecutionBackend *pBackend);

/**
 * @name Utility Functions
 * @brief Helper functions for working with execution backends
 */

/**
 * @brief Get the name of an execution mode
 * @param enmMode The execution mode
 * @return Human-readable name string
 */
const char *VBoxExecModeGetName(VBoxExecMode enmMode);

/**
 * @brief Get the description of an execution mode
 * @param enmMode The execution mode
 * @return Human-readable description string
 */
const char *VBoxExecModeGetDescription(VBoxExecMode enmMode);

/**
 * @brief Check if an execution mode requires executable memory
 * @param enmMode The execution mode
 * @return true if mode requires executable memory, false otherwise
 */
bool VBoxExecModeRequiresExecutableMemory(VBoxExecMode enmMode);

/**
 * @brief Check if a mode is a software emulation mode
 * @param enmMode The execution mode
 * @return true if mode is software emulation, false if hardware
 */
bool VBoxExecModeIsSoftwareEmulation(VBoxExecMode enmMode);

/**
 * @brief Get capabilities for an execution mode
 * @param enmMode The execution mode
 * @return Bitfield of capabilities
 */
VBoxExecCapabilities VBoxExecModeGetCapabilities(VBoxExecMode enmMode);

/**
 * @name Compile-Time Configuration Flags
 * @brief Flags that affect backend behavior
 */

/** @brief Define to disable executable memory allocation (for UWP/Xbox One) */
#ifdef VBOX_NO_EXEC_ALLOC
# define VBOX_EXEC_NO_JIT 1
#else
# define VBOX_EXEC_NO_JIT 0
#endif

/**
 * @name Version Information
 */
#define VBOX_EXEC_BACKEND_VERSION_MAJOR 1
#define VBOX_EXEC_BACKEND_VERSION_MINOR 0
#define VBOX_EXEC_BACKEND_VERSION_BUILD 0

#endif /* IVMM_EXECUTION_BACKEND_H */
