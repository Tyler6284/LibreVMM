/**
 * @file FrontendBase.h
 * @brief Abstract base class for all LibreVMM frontends
 * 
 * Provides common functionality for VM lifecycle, device management,
 * and snapshot operations that all frontends can inherit from.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef FRONTENDBASE_H
#define FRONTENDBASE_H

#include <IVBoxFrontend.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>

/**
 * @brief Frontend type enumeration
 */
enum class FrontendType
{
    CLI,
    Qt,
    WinRT,
    Java,
    Android,
    Web
};

/**
 * @brief Frontend event types for internal handling
 */
enum class FrontendInternalEvent
{
    VMCreated,
    VMDeleted,
    VMStarted,
    VMStopped,
    VMPaused,
    VMResumed,
    SnapshotTaken,
    SnapshotRestored,
    DeviceAdded,
    DeviceRemoved,
    Error,
    Warning,
    Info
};

/**
 * @brief Frontend event data structure
 */
struct FrontendInternalEventData
{
    FrontendInternalEvent event;
    std::string vmId;
    std::string message;
    int errorCode;
    std::map<std::string, std::string> extra;
};

/**
 * @brief Callback type for internal frontend events
 */
using FrontendEventHandler = std::function<void(const FrontendInternalEventData&)>;

/**
 * @brief VM session information
 */
struct VMSessionInfo
{
    std::string sessionId;
    std::string vmId;
    std::string sessionType;
    std::chrono::steady_clock::time_point startTime;
    bool isActive;
};

/**
 * @brief Configuration validation result
 */
struct ConfigValidationResult
{
    bool valid;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
    int errorCode;
    
    ConfigValidationResult() : valid(true), errorCode(0) {}
};

/**
 * @brief FrontendBase - Abstract base class for all UI frontends
 * 
 * This class provides the common foundation that all frontend implementations
 * (CLI, Qt, WinRT, Java, Android, Web) should inherit from or use as a reference.
 * It wraps the IVBoxFrontend interface with additional convenience methods.
 */
class FrontendBase
{
protected:
    std::unique_ptr<IVBoxFrontend> m_frontend;
    FrontendType m_frontendType;
    std::map<std::string, VMSessionInfo> m_sessions;
    std::vector<FrontendEventHandler> m_eventHandlers;
    mutable std::mutex m_mutex;
    std::atomic<bool> m_running;
    std::atomic<int> m_nextCallbackId;
    std::vector<int> m_registeredCallbacks;

public:
    explicit FrontendBase(FrontendType frontendType);
    virtual ~FrontendBase();
    
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual FrontendType getType() const { return m_frontendType; }
    virtual bool isRunning() const { return m_running.load(); }
    
    std::string createSimpleVM(const std::string& name, const std::string& ostype,
                                uint32_t memoryMB, uint32_t cpuCount);
    
    std::string createVMWithDisk(const std::string& name, const std::string& ostype,
                                   uint32_t memoryMB, uint32_t cpuCount,
                                   const std::string& diskPath, uint64_t diskSizeMB);
    
    std::string getVMByName(const std::string& name);
    std::string startVM(const std::string& vmIdOrName);
    std::string startVM(const std::string& vmIdOrName, const std::string& sessionType);
    bool stopVM(const std::string& vmId, int waitSeconds = 30);
    void forceStopVM(const std::string& vmId);
    std::vector<VMDescriptor> getAllVMs();
    
    std::string createDisk(const std::string& format, const std::string& path, uint64_t sizeMB);
    bool attachDisk(const std::string& vmId, const std::string& controllerName,
                    int port, int device, const std::string& mediumPath);
    std::vector<MediumInfo> getAllMedia();
    
    std::vector<HostInterfaceDescriptor> getHostInterfaces();
    bool addNATAdapter(const std::string& vmId);
    bool addBridgedAdapter(const std::string& vmId, const std::string& hostInterface);
    
    std::string takeSnapshot(const std::string& vmId, const std::string& name,
                              const std::string& description = "");
    bool restoreSnapshotByName(const std::string& vmId, const std::string& snapshotName);
    std::vector<SnapshotInfo> getAllSnapshots(const std::string& vmId);
    bool deleteSnapshotByName(const std::string& vmId, const std::string& snapshotName);
    
    ConfigValidationResult validateConfig(const VMConfig& config);
    VMHardwareConfig getRecommendedSettings(const std::string& ostype);
    
    void trackSession(const std::string& sessionId, const std::string& vmId,
                      const std::string& sessionType);
    void untrackSession(const std::string& sessionId);
    size_t getActiveSessionCount() const;
    
    int registerEventHandler(FrontendEventHandler handler);
    void unregisterEventHandler(int handlerId);
    void fireEvent(FrontendInternalEvent event, const std::string& vmId,
                   const std::string& message, int errorCode = 0);
    
    SystemProperties getSystemProperties();
    std::vector<std::string> getOSTypes();
    VMRuntimeInfo getVMRuntimeInfo(const std::string& vmId);
    bool waitForVMState(const std::string& vmId, VMPowerState state, int timeoutMs = 30000);
    bool isVMRunning(const std::string& vmId);

protected:
    void setFrontend(std::unique_ptr<IVBoxFrontend> frontend);
    IVBoxFrontend* getFrontend() { return m_frontend.get(); }
    const IVBoxFrontend* getFrontend() const { return m_frontend.get(); }
    virtual void log(const std::string& level, const std::string& message);
    virtual void showError(const std::string& message);
    virtual void showWarning(const std::string& message);
    virtual void showInfo(const std::string& message);

private:
    std::string findSnapshotByName(const std::string& vmId, const std::string& name);
};

#endif /* FRONTENDBASE_H */
