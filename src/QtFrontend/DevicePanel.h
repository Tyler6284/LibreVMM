/**
 * @file DevicePanel.h
 * @brief Device Selection Panels for QEMU/86Box/Bochs Devices
 * 
 * Provides device configuration panels for selecting and configuring
 * devices from the expanded device catalog.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef DEVICEPANEL_H
#define DEVICEPANEL_H

#include <QDialog>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>

/**
 * @brief Device Panel Dialog
 * 
 * Provides tabs for configuring different device categories:
 * - System (CPU, Memory, Chipset)
 * - Storage
 * - Network
 * - Audio
 * - Video
 * - USB
 */
class DevicePanel : public QDialog
{
    Q_OBJECT

public:
    explicit DevicePanel(QWidget* parent = nullptr);
    ~DevicePanel();
    
    /**
     * @brief Set the VM ID to configure
     * @param vmId VM ID
     */
    void setVMId(const QString& vmId);

private slots:
    void onApplyClicked();
    void onCancelClicked();
    void onAddStorageController();
    void onRemoveStorageController();
    void onAddNetworkAdapter();
    void onRemoveNetworkAdapter();
    void onAddVideoDevice();
    void onRemoveVideoDevice();
    void onAddUSBController();
    void onRemoveUSBController();

private:
    void createTabs();
    void loadSettings();
    void saveSettings();
    
    QString m_vmId;
    QTabWidget* m_tabWidget;
    
    // System tab
    QComboBox* m_osTypeCombo;
    QSpinBox* m_memorySpin;
    QSpinBox* m_cpuSpin;
    QComboBox* m_chipsetCombo;
    QComboBox* m_firmwareCombo;
    QCheckBox* m_ioAPICCheck;
    QCheckBox* m_paeCheck;
    QCheckBox* m_nestedPagingCheck;
    QCheckBox* m_hwVirtCheck;
    
    // Storage tab
    QTreeWidget* m_storageTree;
    QPushButton* m_addStorageControllerBtn;
    QPushButton* m_removeStorageControllerBtn;
    
    // Network tab
    QTreeWidget* m_networkTree;
    QPushButton* m_addNetworkBtn;
    QPushButton* m_removeNetworkBtn;
    
    // Audio tab
    QCheckBox* m_audioEnabledCheck;
    QComboBox* m_audioBackendCombo;
    QComboBox* m_audioControllerCombo;
    
    // Video tab
    QTreeWidget* m_videoTree;
    QPushButton* m_addVideoBtn;
    QPushButton* m_removeVideoBtn;
    
    // USB tab
    QTreeWidget* m_usbTree;
    QPushButton* m_addUSBBtn;
    QPushButton* m_removeUSBBtn;
    
    // Buttons
    QPushButton* m_applyButton;
    QPushButton* m_cancelButton;
};

/**
 * @brief Storage Controller Configuration Widget
 */
class StorageControllerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StorageControllerWidget(QWidget* parent = nullptr);
    ~StorageControllerWidget();
    
    /**
     * @brief Get controller name
     * @return Controller name
     */
    QString getName() const { return m_nameEdit->text(); }
    
    /**
     * @brief Get controller type
     * @return Controller type string
     */
    QString getType() const { return m_typeCombo->currentText(); }

private:
    QLineEdit* m_nameEdit;
    QComboBox* m_typeCombo;
    QCheckBox* m_bootableCheck;
    QCheckBox* m_hotpluggableCheck;
};

/**
 * @brief Network Adapter Configuration Widget
 */
class NetworkAdapterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkAdapterWidget(QWidget* parent = nullptr);
    ~NetworkAdapterWidget();
    
    /**
     * @brief Get adapter type
     * @return Adapter type
     */
    QString getType() const { return m_typeCombo->currentText(); }
    
    /**
     * @brief Get MAC address
     * @return MAC address
     */
    QString getMACAddress() const { return m_macEdit->text(); }

private:
    QComboBox* m_typeCombo;
    QLineEdit* m_macEdit;
    QCheckBox* m_cableConnectedCheck;
    QComboBox* m_promiscModeCombo;
};

/**
 * @brief Video Device Configuration Widget
 */
class VideoDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoDeviceWidget(QWidget* parent = nullptr);
    ~VideoDeviceWidget();
    
    /**
     * @brief Get video device type
     * @return Video type
     */
    QString getType() const { return m_typeCombo->currentText(); }
    
    /**
     * @brief Get VRAM size
     * @return VRAM in MB
     */
    int getVRAMMB() const { return m_vramSpin->value(); }

private:
    QComboBox* m_typeCombo;
    QSpinBox* m_vramSpin;
    QCheckBox* m_accelerate3DCheck;
    QCheckBox* m_accelerate2DCheck;
};

#endif /* DEVICEPANEL_H */
