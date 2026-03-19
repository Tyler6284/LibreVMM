# Execution Backends

> LibreVMM provides three execution backends for running virtual machines. This document describes each backend, when to use them, and how to extend them.

## Overview

LibreVMM supports three execution modes for running guest virtual machines:

| Backend | Mode | Performance | Requirements |
|---------|------|-------------|--------------|
| HM (Hardware Mode) | `VBOX_EXEC_MODE_HM` | Near-native | VT-x or AMD-V |
| TCG JIT | `VBOX_EXEC_MODE_TCG_JIT` | Good (software) | Executable memory |
| TCG Interpreter | `VBOX_EXEC_MODE_TCG_INTERPRETER` | Moderate (software) | None |

---

## Backend Selection

### When Each Backend Is Used

#### HM Backend (Hardware Virtualization)

The HM backend uses hardware virtualization extensions (Intel VT-x or AMD-V) to execute guest code with near-native performance.

**Used when:**
- VT-x or AMD-V is available on the host
- User explicitly selects HM mode
- Maximum performance is required
- Nested virtualization is needed

**Requirements:**
- Intel VT-x with EPT (Extended Page Tables), or
- AMD-V with RVI (Rapid Virtualization Indexing)
- Unrestricted guest mode (optional, for running real-mode code)
- Nested paging (recommended for performance)

**Code location:** [`src/HMBackend/`](../src/HMBackend/)

#### TCG JIT Backend (Software Emulation with JIT)

The TCG (Tiny Code Generator) JIT backend uses QEMU's TCG engine to compile guest code into host code dynamically.

**Used when:**
- No hardware virtualization is available
- Maximum compatibility is needed
- Running on non-x86 hosts (ARM64, RISC-V, etc.)

**Requirements:**
- Ability to allocate executable memory (W^X)
- Sufficient memory for JIT code cache (default: 128 MB)

**Code location:** [`src/TCGBackend/`](../src/TCGBackend/)

#### TCG Interpreter Backend (No-JIT Software Emulation)

The TCG Interpreter backend uses QEMU's interpreter mode without any JIT compilation. This is the only backend that works in W^X-restricted environments.

**Used when:**
- Running in restricted environments (UWP, Xbox One, sandboxed apps)
- No executable memory allocation is available
- Platform prohibits JIT compilation

**Requirements:**
- None - works in any environment

**Code location:** [`src/TCGInterpreter/`](../src/TCGInterpreter/)

---

## NO Silent Fallback Policy

**Critical:** LibreVMM implements a strict **NO SILENT FALLBACK** policy as defined in Priority 4.2.

### What This Means

1. **If HM is requested but unavailable:** The VM will fail to start with a clear error message. LibreVMM will NOT automatically fall back to TCG.

2. **If TCG JIT is requested but fails:** The VM will report the failure and halt. LibreVMM will NOT silently switch to interpreter mode.

3. **User must explicitly consent to fallback:** To enable fallback behavior, users must:
   - Use the `VMM_BACKEND_SEL_ALLOW_FALLBACK` flag
   - Explicitly acknowledge the fallback in the UI

### Implementation

From [`src/VMMExecutionBackends.h`](../src/VMMExecutionBackends.h):

```c
/**
 * @brief Select an execution backend
 * @note NEVER performs silent fallback - if requested mode is unavailable,
 *       this function returns NULL and sets an error message
 */
IVMMExecutionBackend *VMMBackendMgrSelectBackend(VMMExecutionBackendMgr *pMgr,
                                                   VBoxExecMode enmMode,
                                                   uint32_t fFlags);

/**
 * @brief Request backend with explicit fallback consent
 * @note This is the ONLY way to get fallback behavior - user must explicitly
 *       consent to fallback via VMM_BACKEND_SEL_ALLOW_FALLBACK flag
 */
IVMMExecutionBackend *VMMBackendMgrSelectBackendWithFallback(VMMExecutionBackendMgr *pMgr,
                                                              VBoxExecMode enmPreferredMode,
                                                              VBoxExecMode enmFallbackMode,
                                                              uint32_t fFlags);
```

### Selection Flags

| Flag | Description |
|------|-------------|
| `VMM_BACKEND_SEL_FORCE_HM` | Force hardware virtualization |
| `VMM_BACKEND_SEL_FORCE_TCG_JIT` | Force TCG JIT mode |
| `VMM_BACKEND_SEL_FORCE_TCG_INTERP` | Force TCG interpreter mode |
| `VMM_BACKEND_SEL_ALLOW_FALLBACK` | Allow fallback (requires user consent) |
| `VMM_BACKEND_SEL_REQUIRE_HM` | Require HM, fail if unavailable |
| `VMM_BACKEND_SEL_NO_EXEC_ALLOC` | Prefer no-exec-alloc mode (UWP/Xbox) |

---

## VBOX_NO_EXEC_ALLOC Flag

The `VBOX_NO_EXEC_ALLOC` compile-time flag disables all executable memory allocation, making the TCG interpreter the only available backend.

### When to Use

Define `VBOX_NO_EXEC_ALLOC` when building for:
- **UWP (Universal Windows Platform)** - Windows Store apps cannot allocate executable memory
- **Xbox One** - Console apps are restricted from JIT compilation
- **iOS** - App Store rules prohibit JIT compilation
- **Sandboxed environments** - Any sandbox that enforces W^X memory policy

### How It Works

When `VBOX_NO_EXEC_ALLOC` is defined:

1. JIT code generation is disabled in the TCG backend
2. The system automatically selects TCG interpreter mode
3. The backend allocator returns non-executable memory only
4. Performance is reduced, but compatibility is maintained

From [`include/IVMMExecutionBackend.h`](../include/IVMMExecutionBackend.h):

```c
/** @brief Define to disable executable memory allocation (for UWP/Xbox One) */
#ifdef VBOX_NO_EXEC_ALLOC
# define VBOX_EXEC_NO_JIT 1
#else
# define VBOX_EXEC_NO_JIT 0
#endif
```

### Building with VBOX_NO_EXEC_ALLOC

```bash
# Example: Build with no executable allocation
cmake -DVBOX_NO_EXEC_ALLOC=ON ..
```

---

## Extending the Backend System

### Adding a New Backend

To add a new execution backend (e.g., for a new architecture):

1. **Create backend header** in `src/<BackendName>/<BackendName>.h`:
   ```c
   typedef struct <BackendName> {
       IVMMExecutionBackend base;
       // Backend-specific fields
   } <BackendName>;
   ```

2. **Implement the IVMMExecutionBackend interface:**
   - `pfnInit` - Initialize the backend
   - `pfnDestroy` - Clean up resources
   - `pfnStart` - Start VM execution
   - `pfnStop` - Stop VM execution
   - `pfnPause` / `pfnResume` - Pause/resume
   - `pfnStep` - Single-step execution
   - `pfnGetCaps` - Return capabilities
   - `pfnIsAvailable` - Check if backend can run
   - `pfnAllocMem` / `pfnFreeMem` - Memory management
   - `pfnRun` - Execute guest code
   - `pfnHandleVMExit` - Handle exits (HM only)
   - `pfnInvalidateCache` - Invalidate CPU cache

3. **Register the backend** in `VMMBackendMgrInit()`:
   ```c
   VBoxExecResult VMMBackendMgrInit(VMMExecutionBackendMgr *pMgr)
   {
       // ... existing code ...
       
       // Register new backend
       pMgr->aBackends[pMgr->cBackends++].pBackend = <BackendName>Create();
       
       return VBOX_EXEC_SUCCESS;
   }
   ```

4. **Add factory function** to backend manager:
   ```c
   IVMMExecutionBackend *VMBackendsCreate<BackendName>(void);
   ```

### Backend Interface Reference

Full interface definition in [`include/IVMMExecutionBackend.h`](../include/IVMMExecutionBackend.h):

```c
typedef struct IVMMExecutionBackend {
    const char *pszName;
    const char *pszDescription;
    VBoxExecMode enmMode;
    VBoxExecCapabilities fCapabilities;
    
    PFNBackendInit pfnInit;
    PFNBackendDestroy pfnDestroy;
    PFNBackendStart pfnStart;
    PFNBackendStop pfnStop;
    PFNBackendPause pfnPause;
    PFNBackendResume pfnResume;
    PFNBackendStep pfnStep;
    PFNBackendGetCaps pfnGetCaps;
    PFNBackendIsAvailable pfnIsAvailable;
    PFNBackendAllocMem pfnAllocMem;
    PFNBackendFreeMem pfnFreeMem;
    PFNBackendRun pfnRun;
    PFNBackendHandleVMExit pfnHandleVMExit;
    PFNBackendInvalidateCpuCache pfnInvalidateCache;
    
    void *pvPrivate;
} IVMMExecutionBackend;
```

---

## Capabilities

Each backend reports its capabilities via the `VBoxExecCapabilities` bitfield:

| Capability | Description |
|------------|-------------|
| `VBOX_EXEC_CAP_HW_VIRTUALIZATION` | Supports hardware virtualization |
| `VBOX_EXEC_CAP_JIT_COMPILATION` | Supports JIT code generation |
| `VBOX_EXEC_CAP_INTERPRETER` | Supports interpreter mode |
| `VBOX_EXEC_CAP_NESTED_PAGING` | Supports nested paging |
| `VBOX_EXEC_CAP_NESTED_VIRTUALIZATION` | Supports nested virtualization |
| `VBOX_EXEC_CAP_UNRESTRICTED_GUEST` | Supports unrestricted guest mode |
| `VBOX_EXEC_CAP_SINGLE_STEP` | Supports single-step execution |
| `VBOX_EXEC_CAP_DEBUG_GUEST` | Supports guest debugging |
| `VBOX_EXEC_CAP_MEMORY_PROTECTION` | Supports memory protection |
| `VBOX_EXEC_CAP_LARGE_PAGES` | Supports large pages |
| `VBOX_EXEC_CAP_UEFI` | Supports UEFI |
| `VBOX_EXEC_CAP_SEV` | Supports AMD SEV |

---

## Performance Considerations

| Backend | Typical Performance | Best For |
|---------|-------------------|----------|
| HM | 95-100% of native | Production workloads |
| TCG JIT | 10-30% of native | Compatibility, legacy guests |
| TCG Interpreter | 1-5% of native | W^X-restricted environments |

### Tips for Optimizing TCG Performance

1. **Increase code cache size:** Default is 128 MB; increase for larger workloads
2. **Enable optimization:** Use optimization level 2 (`iOptLevel = 2`)
3. **Use appropriate CPU type:** "host" CPU type provides best emulation
4. **Allocate sufficient RAM:** TCG benefits from adequate memory

---

## Troubleshooting

### "Hardware virtualization is not available"

**Cause:** VT-x/AMD-V not available or disabled in BIOS/UEFI.

**Solutions:**
1. Enable VT-x/AMD-V in BIOS/UEFI settings
2. Check if hypervisor is already running (Hyper-V, VMware, etc.)
3. Use TCG JIT backend instead

### "Failed to allocate executable memory"

**Cause:** System ran out of address space for JIT cache.

**Solutions:**
1. Reduce JIT cache size
2. Use TCG interpreter mode
3. Enable `VBOX_NO_EXEC_ALLOC` for restricted environments

### "TCG backend failed to initialize"

**Cause:** QEMU TCG subsystem initialization failed.

**Solutions:**
1. Check system memory availability
2. Verify build includes QEMU TCG components
3. Try interpreter mode instead

---

## Related Documentation

- [Priority 2 — Full Software Emulation Path](../TODO.md#priority-2--full-software-emulation-path)
- [Priority 4.2 — Execution Guardrail Removal](../TODO.md#priority-4-2---execution-guardrail-removal)
- [BUILD_GUIDE.md](./BUILD_GUIDE.md)
- [`include/IVMMExecutionBackend.h`](../include/IVMMExecutionBackend.h)
- [`src/VMMExecutionBackends.h`](../src/VMMExecutionBackends.h)
