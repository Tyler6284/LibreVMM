/**
 * @file DeviceRegistry.cpp
 * @brief Device Abstraction Layer - Runtime Device Registry
 *
 * This file implements the runtime device registry including:
 * - VBOX_EXT_DEVICES_DIR loading
 * - Device discovery at runtime
 * - Device capability flag system
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "../include/VBoxExtDevReg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <errno.h>

/**
 * @name Registry Global State
 */
static struct
{
    /**< Device directory path */
    char               *pszDevicesDir;

    /**< Loaded modules */
    VBoxExtModuleDescriptor *pModules;

    /**< Number of loaded modules */
    uint32_t            cModules;

    /**< Registered devices */
    VBoxExtDevRegInfo **papDevices;

    /**< Number of registered devices */
    uint32_t            cDevices;

    /**< Conflict rules */
    VBoxExtDeviceConflict *paConflicts;

    /**< Number of conflict rules */
    uint32_t            cConflicts;

    /**< Registry initialized */
    bool                fInitialized;

} g_DeviceRegistry = {0};

/**
 * @name Device Source String Conversion
 */
const char *VBoxExtDevSourceToString(VBoxExtDevSource enmSource)
{
    static const char *apszSourceNames[] = {
        "None",
        "VirtualBox",
        "QEMU",
        "Bochs",
        "86Box",
        "DOSBox",
        "ThirdParty"
    };

    if (enmSource >= VBOX_EXT_DEV_SOURCE_NONE && enmSource < VBOX_EXT_DEV_SOURCE_MAX)
        return apszSourceNames[enmSource];

    return "Unknown";
}

VBoxExtDevSource VBoxExtDevSourceFromString(const char *pszSource)
{
    if (!pszSource)
        return VBOX_EXT_DEV_SOURCE_NONE;

    for (int i = 0; i < VBOX_EXT_DEV_SOURCE_MAX; i++)
    {
        VBoxExtDevSource enmSource = (VBoxExtDevSource)i;
        if (strcmp(pszSource, VBoxExtDevSourceToString(enmSource)) == 0)
            return enmSource;
    }

    return VBOX_EXT_DEV_SOURCE_NONE;
}

/**
 * @brief Initialize the device registry
 */
int VBoxExtDevRegInit(void)
{
    if (g_DeviceRegistry.fInitialized)
        return VBOX_SUCCESS;

    memset(&g_DeviceRegistry, 0, sizeof(g_DeviceRegistry));
    g_DeviceRegistry.fInitialized = true;

    /* Set default device directory from environment or use default */
    const char *pszEnvDir = getenv("VBOX_EXT_DEVICES_DIR");
    if (pszEnvDir)
    {
        g_DeviceRegistry.pszDevicesDir = strdup(pszEnvDir);
    }
    else
    {
        /* Default: /usr/local/lib/vbox-ext-devices or ./devices */
        g_DeviceRegistry.pszDevicesDir = strdup("./devices");
    }

    /* Initialize built-in conflict rules */
    VBoxExtDevRegInitConflictRules();

    return VBOX_SUCCESS;
}

/**
 * @brief Initialize built-in conflict rules
 */
void VBoxExtDevRegInitConflictRules(void)
{
    /* Add default conflict rules */

    /* Cannot have both legacy ISA and modern PCI video */
    VBoxExtDevRegAddConflictRule("VGA", "cirrus-vga",
        "Legacy VGA and Cirrus VGA cannot coexist", 1);
    VBoxExtDevRegAddConflictRule("VGA", "vmware-svga",
        "Legacy VGA and VMware SVGA cannot coexist", 1);

    /* Virtio devices conflict with their legacy counterparts */
    VBoxExtDevRegAddConflictRule("virtio-net", "e1000",
        "Use either virtio-net or e1000, not both", 0);
    VBoxExtDevRegAddConflictRule("virtio-blk", "ide",
        "Use either virtio-blk or IDE, not both", 0);

    /* Multiple USB controllers can cause issues */
    VBoxExtDevRegAddConflictRule("xhci", "ehci",
        "XHCI supersedes EHCI - may cause conflicts", 0);
}

/**
 * @brief Shutdown the device registry
 */
int VBoxExtDevRegShutdown(void)
{
    if (!g_DeviceRegistry.fInitialized)
        return VBOX_SUCCESS;

    /* Unload all modules */
    while (g_DeviceRegistry.cModules > 0)
    {
        VBoxExtDevRegUnregisterModule(&g_DeviceRegistry.pModules[g_DeviceRegistry.cModules - 1]);
    }

    /* Free all registered devices */
    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i])
            free(g_DeviceRegistry.papDevices[i]);
    }
    free(g_DeviceRegistry.papDevices);

    /* Free conflict rules */
    free(g_DeviceRegistry.paConflicts);

    /* Free device directory */
    free(g_DeviceRegistry.pszDevicesDir);

    memset(&g_DeviceRegistry, 0, sizeof(g_DeviceRegistry));

    return VBOX_SUCCESS;
}

/**
 * @brief Set the device directory path
 */
int VBoxExtDevRegSetDevicesDir(const char *pszPath)
{
    if (!pszPath)
        return VBOXERR_INVALID_PARAM;

    free(g_DeviceRegistry.pszDevicesDir);
    g_DeviceRegistry.pszDevicesDir = strdup(pszPath);

    return VBOX_SUCCESS;
}

/**
 * @brief Get the device directory path
 */
const char *VBoxExtDevRegGetDevicesDir(void)
{
    return g_DeviceRegistry.pszDevicesDir;
}

/**
 * @brief Register a device internally
 */
int VBoxExtDevRegRegisterDevice(VBoxExtDevRegInfo *pDevRegInfo)
{
    if (!pDevRegInfo)
        return VBOXERR_INVALID_PARAM;

    /* Validate magic number */
    if (pDevRegInfo->u32Magic != VBOX_EXT_DEVREG_MAGIC)
        return VBOXERR_INVALID_PARAM;

    /* Check if device already registered */
    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            strcmp(g_DeviceRegistry.papDevices[i]->pszName, pDevRegInfo->pszName) == 0)
        {
            return VBOXERR_ALREADY_EXISTS;
        }
    }

    /* Allocate space for new device */
    VBoxExtDevRegInfo **papNewDevices = (VBoxExtDevRegInfo **)realloc(
        g_DeviceRegistry.papDevices,
        (g_DeviceRegistry.cDevices + 1) * sizeof(VBoxExtDevRegInfo *)
    );
    if (!papNewDevices)
        return VBOXERR_NO_MEMORY;

    g_DeviceRegistry.papDevices = papNewDevices;

    /* Copy registration info */
    VBoxExtDevRegInfo *pCopy = (VBoxExtDevRegInfo *)malloc(sizeof(VBoxExtDevRegInfo));
    if (!pCopy)
        return VBOXERR_NO_MEMORY;

    memcpy(pCopy, pDevRegInfo, sizeof(VBoxExtDevRegInfo));
    g_DeviceRegistry.papDevices[g_DeviceRegistry.cDevices] = pCopy;
    g_DeviceRegistry.cDevices++;

    return VBOX_SUCCESS;
}

/**
 * @brief Unregister a device
 */
int VBoxExtDevRegUnregisterDevice(const char *pszDeviceName)
{
    if (!pszDeviceName)
        return VBOXERR_INVALID_PARAM;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            strcmp(g_DeviceRegistry.papDevices[i]->pszName, pszDeviceName) == 0)
        {
            free(g_DeviceRegistry.papDevices[i]);
            g_DeviceRegistry.papDevices[i] = NULL;
            return VBOX_SUCCESS;
        }
    }

    return VBOXERR_DEVICE_NOT_FOUND;
}

/**
 * @brief Find a device by name
 */
const VBoxExtDevRegInfo *VBoxExtDevRegFindDevice(const char *pszDeviceName)
{
    if (!pszDeviceName)
        return NULL;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            strcmp(g_DeviceRegistry.papDevices[i]->pszName, pszDeviceName) == 0)
        {
            return g_DeviceRegistry.papDevices[i];
        }
    }

    return NULL;
}

/**
 * @brief Enumerate all registered devices
 */
int VBoxExtDevRegEnumerateDevices(PFNVBOXEXTDEVICEENUM pfnCallback, void *pvUser)
{
    if (!pfnCallback)
        return VBOXERR_INVALID_PARAM;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i])
        {
            int rc = pfnCallback(g_DeviceRegistry.papDevices[i], pvUser);
            if (rc != VBOX_SUCCESS)
                return rc;
        }
    }

    return VBOX_SUCCESS;
}

/**
 * @brief Get device count
 */
uint32_t VBoxExtDevRegGetDeviceCount(void)
{
    return g_DeviceRegistry.cDevices;
}

/**
 * @brief Find devices by type
 */
uint32_t VBoxExtDevRegFindDevicesByType(VBoxExtDeviceType enmType,
                                          VBoxExtDevRegInfo *paDevRegInfo,
                                          uint32_t cMaxDevices)
{
    uint32_t cFound = 0;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices && cFound < cMaxDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            g_DeviceRegistry.papDevices[i]->enmDeviceType == enmType)
        {
            paDevRegInfo[cFound++] = *g_DeviceRegistry.papDevices[i];
        }
    }

    return cFound;
}

/**
 * @brief Find devices by source
 */
uint32_t VBoxExtDevRegFindDevicesBySource(VBoxExtDevSource enmSource,
                                            VBoxExtDevRegInfo *paDevRegInfo,
                                            uint32_t cMaxDevices)
{
    uint32_t cFound = 0;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices && cFound < cMaxDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            g_DeviceRegistry.papDevices[i]->enmSource == enmSource)
        {
            paDevRegInfo[cFound++] = *g_DeviceRegistry.papDevices[i];
        }
    }

    return cFound;
}

/**
 * @brief Find devices by capability
 */
uint32_t VBoxExtDevRegFindDevicesByCaps(uint32_t fCaps,
                                         VBoxExtDevRegInfo *paDevRegInfo,
                                         uint32_t cMaxDevices)
{
    uint32_t cFound = 0;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices && cFound < cMaxDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            (g_DeviceRegistry.papDevices[i]->fCaps & fCaps) == fCaps)
        {
            paDevRegInfo[cFound++] = *g_DeviceRegistry.papDevices[i];
        }
    }

    return cFound;
}

/**
 * @brief Find devices by bus type
 */
uint32_t VBoxExtDevRegFindDevicesByBus(VBoxExtBusType enmBus,
                                        VBoxExtDevRegInfo *paDevRegInfo,
                                        uint32_t cMaxDevices)
{
    uint32_t cFound = 0;

    for (uint32_t i = 0; i < g_DeviceRegistry.cDevices && cFound < cMaxDevices; i++)
    {
        if (g_DeviceRegistry.papDevices[i] &&
            (g_DeviceRegistry.papDevices[i]->fSupportedBuses & (1U << enmBus)))
        {
            paDevRegInfo[cFound++] = *g_DeviceRegistry.papDevices[i];
        }
    }

    return cFound;
}

/**
 * @brief Register a device module from a shared library
 */
int VBoxExtDevRegRegisterModule(const char *pszPath, VBoxExtModuleDescriptor **ppModule)
{
    if (!pszPath)
        return VBOXERR_INVALID_PARAM;

    /* Load the shared library */
    void *pvModule = dlopen(pszPath, RTLD_NOW);
    if (!pvModule)
    {
        fprintf(stderr, "DeviceRegistry: Failed to load module '%s': %s\n",
                pszPath, dlerror());
        return VBOXERR_GENERIC_FAILURE;
    }

    /* Find the registration function */
    PFNVBOXEXTDEVICESREGISTER pfnRegister = (PFNVBOXEXTDEVICESREGISTER)dlsym(
        pvModule, VBOX_EXT_DEVICES_REGISTER_NAME
    );
    if (!pfnRegister)
    {
        fprintf(stderr, "DeviceRegistry: Module '%s' does not export '%s'\n",
                pszPath, VBOX_EXT_DEVICES_REGISTER_NAME);
        dlclose(pvModule);
        return VBOXERR_NOT_SUPPORTED;
    }

    /* Create a minimal PDMDEVREGCB for registration */
    struct
    {
        PDMDEVREGCB cb;
    } RegCallbacks;

    memset(&RegCallbacks, 0, sizeof(RegCallbacks));
    RegCallbacks.cb.u32Version = 1;
    /* Note: pfnRegisterDevice would be set here in full implementation */

    /* Call the registration function */
    /* Note: In full implementation, we would provide proper callbacks */
    int rc = pfnRegister(&RegCallbacks.cb);

    if (rc != VBOX_SUCCESS)
    {
        fprintf(stderr, "DeviceRegistry: Module '%s' registration failed: %d\n",
                pszPath, rc);
        dlclose(pvModule);
        return rc;
    }

    /* Allocate module descriptor */
    VBoxExtModuleDescriptor *pModule = (VBoxExtModuleDescriptor *)calloc(
        1, sizeof(VBoxExtModuleDescriptor)
    );
    if (!pModule)
    {
        dlclose(pvModule);
        return VBOXERR_NO_MEMORY;
    }

    pModule->pvModule = pvModule;
    pModule->pfnRegister = pfnRegister;
    pModule->pszPath = strdup(pszPath);
    pModule->fLoaded = true;

    /* Add to module list */
    VBoxExtModuleDescriptor *pNewModules = (VBoxExtModuleDescriptor *)realloc(
        g_DeviceRegistry.pModules,
        (g_DeviceRegistry.cModules + 1) * sizeof(VBoxExtModuleDescriptor)
    );
    if (!pNewModules)
    {
        free(pModule);
        dlclose(pvModule);
        return VBOXERR_NO_MEMORY;
    }

    g_DeviceRegistry.pModules = pNewModules;
    g_DeviceRegistry.pModules[g_DeviceRegistry.cModules - 1] = *pModule;
    free(pModule);

    if (ppModule)
        *ppModule = pModule;

    return VBOX_SUCCESS;
}

/**
 * @brief Unregister and unload a device module
 */
int VBoxExtDevRegUnregisterModule(VBoxExtModuleDescriptor *pModule)
{
    if (!pModule)
        return VBOXERR_INVALID_PARAM;

    /* Note: In full implementation, we would call device destructors here */

    /* Unload the shared library */
    if (pModule->pvModule)
    {
        dlclose(pModule->pvModule);
    }

    /* Free module path */
    free((void *)pModule->pszPath);

    /* Remove from list */
    for (uint32_t i = 0; i < g_DeviceRegistry.cModules; i++)
    {
        if (&g_DeviceRegistry.pModules[i] == pModule)
        {
            /* Shift remaining modules */
            for (uint32_t j = i; j < g_DeviceRegistry.cModules - 1; j++)
            {
                g_DeviceRegistry.pModules[j] = g_DeviceRegistry.pModules[j + 1];
            }
            g_DeviceRegistry.cModules--;
            break;
        }
    }

    free(pModule);
    return VBOX_SUCCESS;
}

/**
 * @brief Discover and load all device modules
 */
int VBoxExtDevRegDiscoverDevices(void)
{
    const char *pszDir = g_DeviceRegistry.pszDevicesDir;
    if (!pszDir)
        return VBOXERR_INVALID_STATE;

    DIR *pDir = opendir(pszDir);
    if (!pDir)
    {
        fprintf(stderr, "DeviceRegistry: Cannot open device directory '%s': %s\n",
                pszDir, strerror(errno));
        return VBOXERR_GENERIC_FAILURE;
    }

    struct dirent *pEntry;
    int cLoaded = 0;

    while ((pEntry = readdir(pDir)) != NULL)
    {
        /* Check for shared library extensions */
        const char *pszExt = strrchr(pEntry->d_name, '.');
        if (!pszExt)
            continue;

        bool fIsValidExt = false;
        if (strcmp(pszExt, ".so") == 0)  /* Linux */
            fIsValidExt = true;
        else if (strcmp(pszExt, ".dll") == 0)  /* Windows */
            fIsValidExt = true;
        else if (strcmp(pszExt, ".dylib") == 0)  /* macOS */
            fIsValidExt = true;

        if (!fIsValidExt)
            continue;

        /* Skip hidden files and symbolic links to avoid issues */
        if (pEntry->d_name[0] == '.')
            continue;

        /* Build full path */
        char szPath[1024];
        snprintf(szPath, sizeof(szPath), "%s/%s", pszDir, pEntry->d_name);

        /* Check if it's a regular file */
        struct stat st;
        if (stat(szPath, &st) != 0 || !S_ISREG(st.st_mode))
            continue;

        /* Try to load the module */
        int rc = VBoxExtDevRegRegisterModule(szPath, NULL);
        if (rc == VBOX_SUCCESS)
        {
            printf("DeviceRegistry: Loaded device module: %s\n", pEntry->d_name);
            cLoaded++;
        }
        else
        {
            fprintf(stderr, "DeviceRegistry: Failed to load '%s': %d\n",
                    pEntry->d_name, rc);
        }
    }

    closedir(pDir);

    printf("DeviceRegistry: Loaded %d device modules from '%s'\n", cLoaded, pszDir);

    return VBOX_SUCCESS;
}

/**
 * @brief Register a device conflict rule
 */
int VBoxExtDevRegAddConflictRule(const char *pszDevice1,
                                  const char *pszDevice2,
                                  const char *pszReason,
                                  uint8_t u8Severity)
{
    if (!pszDevice1 || !pszDevice2 || !pszReason)
        return VBOXERR_INVALID_PARAM;

    VBoxExtDeviceConflict *pNewConflicts = (VBoxExtDeviceConflict *)realloc(
        g_DeviceRegistry.paConflicts,
        (g_DeviceRegistry.cConflicts + 1) * sizeof(VBoxExtDeviceConflict)
    );
    if (!pNewConflicts)
        return VBOXERR_NO_MEMORY;

    g_DeviceRegistry.paConflicts = pNewConflicts;

    VBoxExtDeviceConflict *pConflict = &g_DeviceRegistry.paConflicts[g_DeviceRegistry.cConflicts];
    pConflict->pszDevice1 = strdup(pszDevice1);
    pConflict->pszDevice2 = strdup(pszDevice2);
    pConflict->pszReason = strdup(pszReason);
    pConflict->u8Severity = u8Severity;

    g_DeviceRegistry.cConflicts++;

    return VBOX_SUCCESS;
}

/**
 * @brief Validate device combination
 */
int VBoxExtDevRegValidateCombination(const char **pszDevices,
                                      uint32_t cDevices,
                                      VBoxExtDeviceValidationResult *pResult)
{
    if (!pszDevices || !pResult)
        return VBOXERR_INVALID_PARAM;

    memset(pResult, 0, sizeof(*pResult));

    /* Allocate space for conflicts */
    pResult->paConflicts = (VBoxExtDeviceConflict *)calloc(
        cDevices * cDevices, sizeof(VBoxExtDeviceConflict)
    );
    if (!pResult->paConflicts)
        return VBOXERR_NO_MEMORY;

    /* Check all device pairs for conflicts */
    for (uint32_t i = 0; i < cDevices; i++)
    {
        for (uint32_t j = i + 1; j < cDevices; j++)
        {
            const char *pszDev1 = pszDevices[i];
            const char *pszDev2 = pszDevices[j];

            /* Check against all conflict rules */
            for (uint32_t k = 0; k < g_DeviceRegistry.cConflicts; k++)
            {
                VBoxExtDeviceConflict *pRule = &g_DeviceRegistry.paConflicts[k];

                bool fMatches = false;
                if (strcmp(pRule->pszDevice1, pszDev1) == 0 &&
                    strcmp(pRule->pszDevice2, pszDev2) == 0)
                {
                    fMatches = true;
                }
                else if (strcmp(pRule->pszDevice1, pszDev2) == 0 &&
                         strcmp(pRule->pszDevice2, pszDev1) == 0)
                {
                    fMatches = true;
                }

                if (fMatches)
                {
                    /* Add conflict to result */
                    uint32_t idx = pResult->cWarnings + pResult->cErrors;
                    pResult->paConflicts[idx].pszDevice1 = pszDev1;
                    pResult->paConflicts[idx].pszDevice2 = pszDev2;
                    pResult->paConflicts[idx].pszReason = pRule->pszReason;
                    pResult->paConflicts[idx].u8Severity = pRule->u8Severity;

                    if (pRule->u8Severity == 0)
                        pResult->cWarnings++;
                    else
                        pResult->cErrors++;
                }
            }
        }
    }

    /* Combination is valid if no errors */
    pResult->fValid = (pResult->cErrors == 0);

    return VBOX_SUCCESS;
}

/**
 * @brief Free device validation result
 */
void VBoxExtDevRegFreeValidationResult(VBoxExtDeviceValidationResult *pResult)
{
    if (!pResult)
        return;

    free(pResult->paConflicts);
    memset(pResult, 0, sizeof(*pResult));
}

/**
 * @name Device Registry Initialization Helper
 * @brief Called by the main application to initialize the registry
 */
int VBoxExtDevRegCreate(void)
{
    int rc = VBoxExtDevRegInit();
    if (rc != VBOX_SUCCESS)
        return rc;

    /* Load built-in devices */
    /* Note: This would load statically linked devices in a full implementation */

    /* Discover dynamic devices */
    rc = VBoxExtDevRegDiscoverDevices();
    if (rc != VBOX_SUCCESS && rc != VBOXERR_GENERIC_FAILURE)
        return rc;

    return VBOX_SUCCESS;
}
