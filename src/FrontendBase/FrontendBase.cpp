/**
 * @file FrontendBase.cpp
 * @brief Implementation of FrontendBase
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#include "FrontendBase.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

// Implementation of FrontendBase

FrontendBase::FrontendBase(FrontendType frontendType)
    : m_frontendType(frontendType)
    , m_running(false)
    , m_nextCallbackId(0)
{
}

FrontendBase::~FrontendBase()
{
    shutdown();
}

std::string FrontendBase::createSimpleVM(const std::string& name, const std::string& ostype,
                                          uint32_t memoryMB, uint32_t cpuCount)
{
    if (!m_frontend) {
        log("error", "Frontend not initialized");
        return "";
    }
    
    VMConfig config;
    config.name = name;
    config.ostype = ostype;
    config.hardware.memoryMB = memoryMB;
    config.hardware.cpuCount = cpuCount;
    
    // Add default storage controller
    StorageControllerConfig sata;
    sata.name = "SATA";
    sata.type = StorageControllerType::SATA;
    sata.bootable = true;
    config.storageControllers.push_back(sata);
    
    // Add default network adapter
    NetworkAdapterConfig nat;
    nat.name = "NAT";
    nat.type = NetworkAdapterType::NAT;
    nat.enabled = true;
    config.networkAdapters.push_back(nat);
    
    // Add default audio
    AudioConfig audio;
    audio.enabled = true;
    audio.backend = AudioBackendType::PulseAudio;
    config.audioDevices.push_back(audio);
    
    std::string vmId = m_frontend->createVM(config);
    if (!vmId.empty()) {
        fireEvent(FrontendInternalEvent::VMCreated, vmId, "VM created: " + name);
    }
    
    return vmId;
}

std::string FrontendBase::createVMWithDisk(const std::string& name, const std::string& ostype,
                                            uint32_t memoryMB, uint32_t cpuCount,
                                            const std::string& diskPath, uint64_t diskSizeMB)
{
    std::string vmId = createSimpleVM(name, ostype, memoryMB, cpuCount);
    if (vmId.empty()) {
        return "";
    }
    
    // Create and attach disk
    std::string mediumId = createDisk("VDI", diskPath, diskSizeMB);
    if (!mediumId.empty()) {
        attachDisk(vmId, "SATA", 0, 0, diskPath);
    }
    
    return vmId;
}

std::string FrontendBase::getVMByName(const std::string& name)
{
    if (!m_frontend) {
        return "";
    }
    return m_frontend->findVMByName(name);
}

std::string FrontendBase::startVM(const std::string& vmIdOrName)
{
    return startVM(vmIdOrName, "gui");
}

std::string FrontendBase::startVM(const std::string& vmIdOrName, const std::string& sessionType)
{
    if (!m_frontend) {
        log("error", "Frontend not initialized");
        return "";
    }
    
    std::string vmId = vmIdOrName;
    if (m_frontend->findVMByName(vmIdOrName).empty() == false) {
        vmId = m_frontend->findVMByName(vmIdOrName);
    }
    
    std::string sessionId = m_frontend->startVM(vmId, sessionType);
    if (!sessionId.empty()) {
        trackSession(sessionId, vmId, sessionType);
        fireEvent(FrontendInternalEvent::VMStarted, vmId, "VM started");
    }
    
    return sessionId;
}

bool FrontendBase::stopVM(const std::string& vmId, int waitSeconds)
{
    if (!m_frontend) {
        return false;
    }
    
    VMPowerState state = m_frontend->getVMState(vmId);
    if (state == VMPowerState::PoweredOff || state == VMPowerState::Saved) {
        return true;
    }
    
    // Try graceful shutdown first
    m_frontend->saveVMState(vmId);
    
    // Wait for state change
    return waitForVMState(vmId, VMPowerState::Saved, waitSeconds * 1000);
}

void FrontendBase::forceStopVM(const std::string& vmId)
{
    if (!m_frontend) {
        return;
    }
    
    m_frontend->powerOffVM(vmId);
    fireEvent(FrontendInternalEvent::VMStopped, vmId, "VM powered off");
}

std::vector<VMDescriptor> FrontendBase::getAllVMs()
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->listVMs();
}

std::string FrontendBase::createDisk(const std::string& format, const std::string& path, uint64_t sizeMB)
{
    if (!m_frontend) {
        return "";
    }
    
    return m_frontend->createMedium(format, path, sizeMB * 1024 * 1024, "HardDisk");
}

bool FrontendBase::attachDisk(const std::string& vmId, const std::string& controllerName,
                               int port, int device, const std::string& mediumPath)
{
    if (!m_frontend) {
        return false;
    }
    
    // Find medium by path
    auto media = m_frontend->getMedia();
    for (const auto& medium : media) {
        if (medium.location == mediumPath) {
            m_frontend->attachMedium(vmId, controllerName, port, device, medium.id);
            fireEvent(FrontendInternalEvent::DeviceAdded, vmId, "Disk attached");
            return true;
        }
    }
    
    return false;
}

std::vector<MediumInfo> FrontendBase::getAllMedia()
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getMedia();
}

std::vector<HostInterfaceDescriptor> FrontendBase::getHostInterfaces()
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getHostInterfaces();
}

bool FrontendBase::addNATAdapter(const std::string& vmId)
{
    if (!m_frontend) {
        return false;
    }
    
    NetworkAdapterConfig adapter;
    adapter.name = "NAT";
    adapter.type = NetworkAdapterType::NAT;
    adapter.enabled = true;
    
    m_frontend->addNetworkAdapter(vmId, adapter);
    return true;
}

bool FrontendBase::addBridgedAdapter(const std::string& vmId, const std::string& hostInterface)
{
    if (!m_frontend) {
        return false;
    }
    
    NetworkAdapterConfig adapter;
    adapter.name = "Bridged";
    adapter.type = NetworkAdapterType::Bridged;
    adapter.enabled = true;
    adapter.hostInterface = hostInterface;
    
    m_frontend->addNetworkAdapter(vmId, adapter);
    return true;
}

std::string FrontendBase::takeSnapshot(const std::string& vmId, const std::string& name,
                                         const std::string& description)
{
    if (!m_frontend) {
        return "";
    }
    
    std::string snapshotId = m_frontend->takeSnapshot(vmId, name, description);
    if (!snapshotId.empty()) {
        fireEvent(FrontendInternalEvent::SnapshotTaken, vmId, "Snapshot taken: " + name);
    }
    
    return snapshotId;
}

bool FrontendBase::restoreSnapshotByName(const std::string& vmId, const std::string& snapshotName)
{
    std::string snapshotId = findSnapshotByName(vmId, snapshotName);
    if (snapshotId.empty()) {
        return false;
    }
    
    if (!m_frontend) {
        return false;
    }
    
    m_frontend->restoreSnapshot(vmId, snapshotId);
    fireEvent(FrontendInternalEvent::SnapshotRestored, vmId, "Snapshot restored: " + snapshotName);
    return true;
}

std::vector<SnapshotInfo> FrontendBase::getAllSnapshots(const std::string& vmId)
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getSnapshots(vmId);
}

bool FrontendBase::deleteSnapshotByName(const std::string& vmId, const std::string& snapshotName)
{
    std::string snapshotId = findSnapshotByName(vmId, snapshotName);
    if (snapshotId.empty()) {
        return false;
    }
    
    if (!m_frontend) {
        return false;
    }
    
    m_frontend->deleteSnapshot(vmId, snapshotId);
    return true;
}

ConfigValidationResult FrontendBase::validateConfig(const VMConfig& config)
{
    ConfigValidationResult result;
    
    // Validate memory
    if (config.hardware.memoryMB < 4) {
        result.valid = false;
        result.errors.push_back("Memory must be at least 4 MB");
    }
    
    // Validate CPU count
    if (config.hardware.cpuCount < 1) {
        result.valid = false;
        result.errors.push_back("At least 1 CPU required");
    }
    
    // Warn about high memory
    SystemProperties props = getSystemProperties();
    if (config.hardware.memoryMB > props.memorySize) {
        result.warnings.push_back("VM memory exceeds host memory");
    }
    
    return result;
}

VMHardwareConfig FrontendBase::getRecommendedSettings(const std::string& ostype)
{
    VMHardwareConfig config = {};
    
    // Basic recommendations based on OS type
    if (ostype.find("Windows") != std::string::npos) {
        if (ostype.find("64") != std::string::npos) {
            config.memoryMB = 2048;
            config.cpuCount = 2;
        } else {
            config.memoryMB = 1024;
            config.cpuCount = 1;
        }
    } else if (ostype.find("Linux") != std::string::npos) {
        config.memoryMB = 1024;
        config.cpuCount = 1;
    } else if (ostype.find("DOS") != std::string::npos) {
        config.memoryMB = 64;
        config.cpuCount = 1;
    } else {
        // Default
        config.memoryMB = 512;
        config.cpuCount = 1;
    }
    
    config.vramMB = 128;
    config.hardwareVirt = true;
    config.ioAPIC = true;
    
    return config;
}

void FrontendBase::trackSession(const std::string& sessionId, const std::string& vmId,
                                  const std::string& sessionType)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    VMSessionInfo info;
    info.sessionId = sessionId;
    info.vmId = vmId;
    info.sessionType = sessionType;
    info.startTime = std::chrono::steady_clock::now();
    info.isActive = true;
    
    m_sessions[sessionId] = info;
}

void FrontendBase::untrackSession(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sessions.erase(sessionId);
}

size_t FrontendBase::getActiveSessionCount() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sessions.size();
}

int FrontendBase::registerEventHandler(FrontendEventHandler handler)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    int id = m_nextCallbackId.fetch_add(1);
    m_eventHandlers.push_back(handler);
    m_registeredCallbacks.push_back(id);
    
    return id;
}

void FrontendBase::unregisterEventHandler(int handlerId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    auto it = std::find(m_registeredCallbacks.begin(), m_registeredCallbacks.end(), handlerId);
    if (it != m_registeredCallbacks.end()) {
        size_t index = std::distance(m_registeredCallbacks.begin(), it);
        m_eventHandlers.erase(m_eventHandlers.begin() + index);
        m_registeredCallbacks.erase(it);
    }
}

void FrontendBase::fireEvent(FrontendInternalEvent event, const std::string& vmId,
                              const std::string& message, int errorCode)
{
    FrontendInternalEventData data;
    data.event = event;
    data.vmId = vmId;
    data.message = message;
    data.errorCode = errorCode;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (const auto& handler : m_eventHandlers) {
        try {
            handler(data);
        } catch (...) {
            // Ignore handler exceptions
        }
    }
}

SystemProperties FrontendBase::getSystemProperties()
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getSystemProperties();
}

std::vector<std::string> FrontendBase::getOSTypes()
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getOSTypes();
}

VMRuntimeInfo FrontendBase::getVMRuntimeInfo(const std::string& vmId)
{
    if (!m_frontend) {
        return {};
    }
    return m_frontend->getVMRuntimeInfo(vmId);
}

bool FrontendBase::waitForVMState(const std::string& vmId, VMPowerState state, int timeoutMs)
{
    if (!m_frontend) {
        return false;
    }
    
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        VMPowerState currentState = m_frontend->getVMState(vmId);
        if (currentState == state) {
            return true;
        }
        
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > timeoutMs) {
            return false;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool FrontendBase::isVMRunning(const std::string& vmId)
{
    if (!m_frontend) {
        return false;
    }
    
    VMPowerState state = m_frontend->getVMState(vmId);
    return state == VMPowerState::Running;
}

void FrontendBase::setFrontend(std::unique_ptr<IVBoxFrontend> frontend)
{
    m_frontend = std::move(frontend);
}

void FrontendBase::log(const std::string& level, const std::string& message)
{
    std::cout << "[" << level << "] " << message << std::endl;
}

void FrontendBase::showError(const std::string& message)
{
    log("error", message);
}

void FrontendBase::showWarning(const std::string& message)
{
    log("warning", message);
}

void FrontendBase::showInfo(const std::string& message)
{
    log("info", message);
}

std::string FrontendBase::findSnapshotByName(const std::string& vmId, const std::string& name)
{
    if (!m_frontend) {
        return "";
    }
    
    auto snapshots = m_frontend->getSnapshots(vmId);
    for (const auto& snapshot : snapshots) {
        if (snapshot.name == name) {
            return snapshot.id;
        }
    }
    
    return "";
}
