/**
 * @file VMWizard.h
 * @brief VM Creation Wizard (mirrors VirtualBox 5.2)
 * 
 * Step-by-step wizard for creating new virtual machines.
 * Layout and workflow based on VirtualBox 5.2 for user familiarity.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef VMWIZARD_H
#define VMWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>

/**
 * @brief VM Creation Wizard
 * 
 * Mirrors VirtualBox 5.2 wizard steps:
 * 1. Name and OS Type
 * 2. Memory
 * 3. Hard Disk (Create new or use existing)
 * 4. Hard Disk File Type
 * 5. Storage on Physical Disk
 * 6. File Location and Size
 * 7. Summary
 */
class VMWizard : public QWizard
{
    Q_OBJECT

public:
    explicit VMWizard(QWidget* parent = nullptr);
    ~VMWizard();

    /**
     * @brief Get the created VM configuration
     * @return VM name
     */
    QString getVMName() const { return m_nameEdit->text(); }
    
    /**
     * @brief Get selected OS type
     * @return OS type string
     */
    QString getOSType() const { return m_osTypeCombo->currentText(); }
    
    /**
     * @brief Get memory size
     * @return Memory in MB
     */
    int getMemoryMB() const { return m_memorySpin->value(); }
    
    /**
     * @brief Get CPU count
     * @return Number of CPUs
     */
    int getCPUCount() const { return m_cpuSpin->value(); }
    
    /**
     * @brief Get disk size
     * @return Disk size in MB
     */
    int getDiskSizeMB() const { return m_diskSizeSpin->value(); }
    
    /**
     * @brief Get disk file path
     * @return Disk file path
     */
    QString getDiskPath() const { return m_diskPath; }

private slots:
    void onCreateDiskClicked();
    void onBrowseDiskClicked();

private:
    void createPages();
    
    // Page 1: Name and OS Type
    QLineEdit* m_nameEdit;
    QComboBox* m_osTypeCombo;
    
    // Page 2: Memory
    QSpinBox* m_memorySpin;
    QSpinBox* m_cpuSpin;
    QLabel* m_memoryWarningLabel;
    
    // Page 3: Hard Disk
    QRadioButton* m_createDiskRadio;
    QRadioButton* m_useExistingRadio;
    QListWidget* m_existingDisksList;
    
    // Page 4: File Type
    QComboBox* m_diskFormatCombo;
    
    // Page 5: Storage Type
    QRadioButton* m_dynamicStorageRadio;
    QRadioButton* m_fixedStorageRadio;
    
    // Page 6: Disk Location and Size
    QLineEdit* m_diskLocationEdit;
    QSpinBox* m_diskSizeSpin;
    QLabel* m_diskSizeWarningLabel;
    
    // File path to use
    QString m_diskPath;
};

/**
 * @brief Page 1: Name and OS Type
 */
class VMWizardNamePage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardNamePage(QWidget* parent = nullptr);
    
    bool validatePage() override;
    int nextId() const override;

private:
    QLineEdit* m_nameEdit;
    QComboBox* m_osTypeCombo;
    QLabel* m_infoLabel;
};

/**
 * @brief Page 2: Memory
 */
class VMWizardMemoryPage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardMemoryPage(QWidget* parent = nullptr);
    
    bool validatePage() override;
    int nextId() const override;

private slots:
    void updateMemoryWarning();

private:
    QSpinBox* m_memorySpin;
    QSpinBox* m_cpuSpin;
    QLabel* m_memoryWarningLabel;
    QLabel* m_recommendedLabel;
};

/**
 * @brief Page 3: Hard Disk
 */
class VMWizardDiskPage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardDiskPage(QWidget* parent = nullptr);
    
    int nextId() const override;

private slots:
    void onCreateDiskToggled(bool checked);
    void onUseExistingToggled(bool checked);

private:
    QRadioButton* m_createDiskRadio;
    QRadioButton* m_useExistingRadio;
    QListWidget* m_existingDisksList;
};

/**
 * @brief Page 4: Disk Format
 */
class VMWizardDiskFormatPage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardDiskFormatPage(QWidget* parent = nullptr);
    
    int nextId() const override;

private:
    QComboBox* m_formatCombo;
    QLabel* m_formatInfoLabel;
};

/**
 * @brief Page 5: Storage Type
 */
class VMWizardStoragePage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardStoragePage(QWidget* parent = nullptr);
    
    int nextId() const override;

private:
    QRadioButton* m_dynamicRadio;
    QRadioButton* m_fixedRadio;
};

/**
 * @brief Page 6: Disk Size and Location
 */
class VMWizardDiskSizePage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardDiskSizePage(QWidget* parent = nullptr);
    
    bool validatePage() override;
    int nextId() const override;

private slots:
    void onBrowseClicked();

private:
    QLineEdit* m_locationEdit;
    QSpinBox* m_sizeSpin;
    QLabel* m_warningLabel;
    QLabel* m_sizeInfoLabel;
};

/**
 * @brief Page 7: Summary
 */
class VMWizardSummaryPage : public QWizardPage
{
    Q_OBJECT

public:
    VMWizardSummaryPage(QWidget* parent = nullptr);
    
    void initializePage() override;

private:
    QLabel* m_summaryLabel;
};

#endif /* VMWIZARD_H */
