/**
 * @file RawConfigMode.h
 * @brief Raw Configuration Mode - Exposes All Settings
 * 
 * Provides a configuration mode that exposes all available settings
 * without OS-type filtering or automated corrections.
 * Per Priority 4 requirements - all guardrails become warnings only.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef RAWCONFIGMODE_H
#define RAWCONFIGMODE_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>

/**
 * @brief Raw Configuration Mode Dialog
 * 
 * Shows raw VM configuration without any filtering or auto-correction.
 * Displays the complete configuration XML/JSON and allows direct editing.
 */
class RawConfigMode : public QDialog
{
    Q_OBJECT

public:
    explicit RawConfigMode(QWidget* parent = nullptr);
    ~RawConfigMode();
    
    /**
     * @brief Set VM ID to configure
     * @param vmId VM ID
     */
    void setVMId(const QString& vmId);

private slots:
    void onApplyClicked();
    void onCancelClicked();
    void onResetClicked();
    void onExportClicked();
    void onImportClicked();
    void onShowAllSettingsToggled(bool checked);

private:
    void loadConfiguration();
    void saveConfiguration();
    void populateFormFromConfig();
    void populateConfigFromForm();
    void showWarnings();
    
    QString m_vmId;
    QTabWidget* m_tabWidget;
    QTextEdit* m_rawConfigEdit;
    
    // Buttons
    QPushButton* m_applyButton;
    QPushButton* m_cancelButton;
    QPushButton* m_resetButton;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    
    // Checkbox for showing all settings
    QCheckBox* m_showAllSettingsCheck;
    
    // Warning display
    QTextEdit* m_warningsEdit;
    
    // Form fields - General
    QLineEdit* m_nameEdit;
    QLineEdit* m_osTypeEdit;
    QLineEdit* m_descriptionEdit;
    
    // Form fields - System
    QSpinBox* m_memorySpin;
    QSpinBox* m_cpuSpin;
    QSpinBox* m_vramSpin;
    QComboBox* m_chipsetCombo;
    QComboBox* m_firmwareCombo;
    QComboBox* m_executionBackendCombo;
    QCheckBox* m_ioAPICCheck;
    QCheckBox* m_paeCheck;
    QCheckBox* m_longModeCheck;
    QCheckBox* m_nestedPagingCheck;
    QCheckBox* m_hwVirtCheck;
    QCheckBox* m_efiCheck;
    
    // Form fields - Display
    QComboBox* m_accelerate3DCombo;
    QComboBox* m_accelerate2DCombo;
    
    // Form fields - Network (expandable)
    QWidget* m_networkContainer;
    QVector<QWidget*> m_networkWidgets;
    
    // Form fields - Storage (expandable)
    QWidget* m_storageContainer;
    QVector<QWidget*> m_storageWidgets;
    
    // Form fields - Audio
    QCheckBox* m_audioEnabledCheck;
    QComboBox* m_audioBackendCombo;
    
    // Form fields - USB
    QWidget* m_usbContainer;
    QVector<QWidget*> m_usbWidgets;
    
    // Raw configuration string
    QString m_rawConfig;
};

#endif /* RAWCONFIGMODE_H */
