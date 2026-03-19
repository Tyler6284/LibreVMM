/**
 * @file MainWindow.cpp
 * @brief Main Window Implementation
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#include "MainWindow.h"
#include "VMWizard.h"
#include "DevicePanel.h"
#include "RawConfigMode.h"

#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QAction>
#include <QMenu>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDebug>
#include <QIcon>

// Include Qt headers for the display widget
#include <QPainter>
#include <QImage>

// ===== VMDisplayWidget Implementation =====

VMDisplayWidget::VMDisplayWidget(QWidget* parent)
    : QWidget(parent)
    , m_displayEnabled(false)
{
    setMinimumSize(640, 480);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // Default to a dark background
    setStyleSheet("background-color: #1a1a1a;");
}

VMDisplayWidget::~VMDisplayWidget()
{
}

void VMDisplayWidget::updateFramebuffer(int width, int height, const QVector<quint32>& data)
{
    if (width <= 0 || height <= 0 || data.isEmpty()) {
        m_displayEnabled = false;
        update();
        return;
    }
    
    m_displayEnabled = true;
    
    // Convert pixel data to QImage
    m_framebufferImage = QImage(reinterpret_cast<const uchar*>(data.constData()),
                                width, height, QImage::Format_ARGB32);
    
    update();
}

void VMDisplayWidget::setDisplayEnabled(bool enabled)
{
    m_displayEnabled = enabled;
    update();
}

void VMDisplayWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    
    if (!m_displayEnabled || m_framebufferImage.isNull()) {
        // Draw placeholder
        painter.fillRect(rect(), QColor("#1a1a1a"));
        
        painter.setPen(Qt::gray);
        painter.drawText(rect(), Qt::AlignCenter, 
                        tr("No VM running\nSelect a VM and click Start"));
        return;
    }
    
    // Scale image to fit widget while maintaining aspect ratio
    QImage scaled = m_framebufferImage.scaled(rect().size(), 
                                               Qt::KeepAspectRatio, 
                                               Qt::SmoothTransformation);
    
    // Center the image
    int x = (rect().width() - scaled.width()) / 2;
    int y = (rect().height() - scaled.height()) / 2;
    
    painter.drawImage(x, y, scaled);
}

// ===== MainWindow Implementation =====

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_frontend(nullptr)
    , m_vmTree(nullptr)
    , m_detailsTable(nullptr)
    , m_displayWidget(nullptr)
    , m_vmListDock(nullptr)
    , m_detailsDock(nullptr)
    , m_displayDock(nullptr)
    , m_newVMAction(nullptr)
    , m_openVMAction(nullptr)
    , m_closeVMAction(nullptr)
    , m_startAction(nullptr)
    , m_pauseAction(nullptr)
    , m_resumeAction(nullptr)
    , m_stopAction(nullptr)
    , m_powerOffAction(nullptr)
    , m_snapshotAction(nullptr)
    , m_settingsAction(nullptr)
    , m_rawConfigAction(nullptr)
    , m_refreshAction(nullptr)
    , m_exitAction(nullptr)
    , m_aboutAction(nullptr)
    , m_startToolButton(nullptr)
    , m_pauseToolButton(nullptr)
    , m_statusLabel(nullptr)
    , m_vmCountLabel(nullptr)
    , m_updateTimer(nullptr)
{
    // Initialize frontend - in real implementation, connect to actual backend
    // m_frontend = new FrontendBase(FrontendType::Qt);
    // m_frontend->initialize();
    
    initUI();
    
    // Set up update timer
    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::refreshVMList);
    m_updateTimer->start(5000);  // Update every 5 seconds
    
    // Initial refresh
    refreshVMList();
}

MainWindow::~MainWindow()
{
    if (m_updateTimer) {
        m_updateTimer->stop();
    }
    
    // if (m_frontend) {
    //     m_frontend->shutdown();
    //     delete m_frontend;
    // }
}

void MainWindow::initUI()
{
    // Set window properties
    setWindowTitle("LibreVMM");
    setMinimumSize(1024, 768);
    resize(1280, 800);
    
    // Create UI components
    createMenuBar();
    createToolBar();
    createStatusBar();
    createDockWidgets();
    
    // Set central widget for display
    m_displayWidget = new VMDisplayWidget(this);
    setCentralWidget(m_displayWidget);
    
    // Update actions
    updateActions();
}

void MainWindow::createMenuBar()
{
    // File Menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    
    m_newVMAction = new QAction(tr("&New..."), this);
    m_newVMAction->setShortcut(QKeySequence::New);
    m_newVMAction->setStatusTip(tr("Create a new virtual machine"));
    connect(m_newVMAction, &QAction::triggered, this, &MainWindow::onNewVM);
    m_fileMenu->addAction(m_newVMAction);
    
    m_openVMAction = new QAction(tr("&Open..."), this);
    m_openVMAction->setShortcut(QKeySequence::Open);
    m_openVMAction->setStatusTip(tr("Open an existing virtual machine"));
    m_fileMenu->addAction(m_openVMAction);
    
    m_closeVMAction = new QAction(tr("&Close"), this);
    m_closeVMAction->setShortcut(QKeySequence::Close);
    m_closeVMAction->setStatusTip(tr("Close the selected virtual machine"));
    m_fileMenu->addAction(m_closeVMAction);
    
    m_fileMenu->addSeparator();
    
    m_refreshAction = new QAction(tr("&Refresh"), this);
    m_refreshAction->setShortcut(QKeySequence::Refresh);
    m_refreshAction->setStatusTip(tr("Refresh VM list"));
    connect(m_refreshAction, &QAction::triggered, this, &MainWindow::refreshVMList);
    m_fileMenu->addAction(m_refreshAction);
    
    m_fileMenu->addSeparator();
    
    m_exitAction = new QAction(tr("E&xit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit LibreVMM"));
    connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExit);
    m_fileMenu->addAction(m_exitAction);
    
    // Machine Menu
    m_machineMenu = menuBar()->addMenu(tr("&Machine"));
    
    m_startAction = new QAction(tr("&Start"), this);
    m_startAction->setShortcut(QKeySequence(tr("F5")));
    m_startAction->setStatusTip(tr("Start the selected virtual machine"));
    connect(m_startAction, &QAction::triggered, this, &MainWindow::onStartVM);
    m_machineMenu->addAction(m_startAction);
    
    m_pauseAction = new QAction(tr("&Pause"), this);
    m_pauseAction->setShortcut(QKeySequence(tr("F6")));
    m_pauseAction->setStatusTip(tr("Pause the selected virtual machine"));
    connect(m_pauseAction, &QAction::triggered, this, &MainWindow::onPauseVM);
    m_machineMenu->addAction(m_pauseAction);
    
    m_resumeAction = new QAction(tr("&Resume"), this);
    m_resumeAction->setShortcut(QKeySequence(tr("F7")));
    m_resumeAction->setStatusTip(tr("Resume the selected virtual machine"));
    connect(m_resumeAction, &QAction::triggered, this, &MainWindow::onResumeVM);
    m_machineMenu->addAction(m_resumeAction);
    
    m_machineMenu->addSeparator();
    
    m_stopAction = new QAction(tr("&Save State"), this);
    m_stopAction->setShortcut(QKeySequence(tr("F8")));
    m_stopAction->setStatusTip(tr("Save the state of the selected virtual machine"));
    connect(m_stopAction, &QAction::triggered, this, &MainWindow::onStopVM);
    m_machineMenu->addAction(m_stopAction);
    
    m_powerOffAction = new QAction(tr("&Power Off"), this);
    m_powerOffAction->setShortcut(QKeySequence(tr("F9")));
    m_powerOffAction->setStatusTip(tr("Power off the selected virtual machine"));
    connect(m_powerOffAction, &QAction::triggered, this, &MainWindow::onPowerOffVM);
    m_machineMenu->addAction(m_powerOffAction);
    
    m_machineMenu->addSeparator();
    
    m_snapshotAction = new QAction(tr("&Take Snapshot..."), this);
    m_snapshotAction->setShortcut(QKeySequence(tr("Ctrl+Shift+T")));
    m_snapshotAction->setStatusTip(tr("Take a snapshot of the selected virtual machine"));
    connect(m_snapshotAction, &QAction::triggered, this, &MainWindow::onTakeSnapshot);
    m_machineMenu->addAction(m_snapshotAction);
    
    m_machineMenu->addSeparator();
    
    m_settingsAction = new QAction(tr("&Settings..."), this);
    m_settingsAction->setShortcut(QKeySequence::Preferences);
    m_settingsAction->setStatusTip(tr("Show settings for selected virtual machine"));
    connect(m_settingsAction, &QAction::triggered, this, &MainWindow::onShowSettings);
    m_machineMenu->addAction(m_settingsAction);
    
    m_rawConfigAction = new QAction(tr("Show &Raw Configuration..."), this);
    m_rawConfigAction->setStatusTip(tr("Show raw configuration for selected VM"));
    connect(m_rawConfigAction, &QAction::triggered, this, &MainWindow::onShowRawConfig);
    m_machineMenu->addAction(m_rawConfigAction);
    
    // View Menu
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    
    // Help Menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    
    m_aboutAction = new QAction(tr("&About LibreVMM"), this);
    m_aboutAction->setStatusTip(tr("About LibreVMM"));
    connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    m_helpMenu->addAction(m_aboutAction);
}

void MainWindow::createToolBar()
{
    QToolBar* toolbar = addToolBar(tr("Main Toolbar"));
    toolbar->setMovable(false);
    
    // Start button
    m_startToolButton = new QToolButton(this);
    m_startToolButton->setDefaultAction(m_startAction);
    m_startToolButton->setText(tr("Start"));
    toolbar->addWidget(m_startToolButton);
    
    // Pause button
    m_pauseToolButton = new QToolButton(this);
    m_pauseToolButton->setDefaultAction(m_pauseAction);
    m_pauseToolButton->setText(tr("Pause"));
    toolbar->addWidget(m_pauseToolButton);
    
    toolbar->addSeparator();
    
    // Settings button
    toolbar->addAction(m_settingsAction);
    
    // Refresh button
    toolbar->addAction(m_refreshAction);
}

void MainWindow::createStatusBar()
{
    m_statusLabel = new QLabel(tr("Ready"), this);
    statusBar()->addWidget(m_statusLabel);
    
    m_vmCountLabel = new QLabel(tr("0 VMs"), this);
    statusBar()->addPermanentWidget(m_vmCountLabel);
}

void MainWindow::createDockWidgets()
{
    // VM List Dock
    m_vmListDock = new QDockWidget(tr("Virtual Machines"), this);
    m_vmListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    m_vmTree = new QTreeWidget(this);
    m_vmTree->setHeaderLabel(tr("Virtual Machines"));
    m_vmTree->setColumnCount(2);
    m_vmTree->setColumnHidden(1, true);  // Hide UUID column
    
    connect(m_vmTree, &QTreeWidget::itemSelectionChanged,
            this, &MainWindow::updateActions);
    connect(m_vmTree, &QTreeWidget::itemDoubleClicked,
            this, &MainWindow::onStartVM);
    
    m_vmListDock->setWidget(m_vmTree);
    addDockWidget(Qt::LeftDockWidgetArea, m_vmListDock);
    
    // Details Dock
    m_detailsDock = new QDockWidget(tr("Details"), this);
    m_detailsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    
    m_detailsTable = new QTableWidget(this);
    m_detailsTable->setColumnCount(2);
    m_detailsTable->setRowCount(0);
    m_detailsTable->horizontalHeader()->setVisible(false);
    m_detailsTable->verticalHeader()->setVisible(false);
    m_detailsTable->setShowGrid(false);
    m_detailsTable->setAlternatingRowColors(true);
    
    m_detailsDock->setWidget(m_detailsTable);
    addDockWidget(Qt::RightDockWidgetArea, m_detailsDock);
    
    // Connect VM list to details
    connect(m_vmTree, &QTreeWidget::itemSelectionChanged,
            this, &MainWindow::refreshDetails);
}

void MainWindow::createVMListPanel()
{
    // Implemented in createDockWidgets
}

void MainWindow::createDetailsPanel()
{
    // Implemented in createDockWidgets
}

void MainWindow::createDisplayWidget()
{
    // Already set as central widget in initUI
}

void MainWindow::updateActions()
{
    bool hasSelection = !getCurrentVMId().isEmpty();
    
    m_startAction->setEnabled(hasSelection);
    m_pauseAction->setEnabled(hasSelection);
    m_resumeAction->setEnabled(hasSelection);
    m_stopAction->setEnabled(hasSelection);
    m_powerOffAction->setEnabled(hasSelection);
    m_snapshotAction->setEnabled(hasSelection);
    m_settingsAction->setEnabled(hasSelection);
    m_rawConfigAction->setEnabled(hasSelection);
    m_closeVMAction->setEnabled(hasSelection);
    
    if (m_startToolButton) {
        m_startToolButton->setEnabled(hasSelection);
    }
    if (m_pauseToolButton) {
        m_pauseToolButton->setEnabled(hasSelection);
    }
}

QString MainWindow::getCurrentVMId() const
{
    QList<QTreeWidgetItem*> selected = m_vmTree->selectedItems();
    if (selected.isEmpty()) {
        return QString();
    }
    
    // Return UUID from second column
    return selected.first()->text(1);
}

// ===== Action Slots =====

void MainWindow::onNewVM()
{
    VMWizard wizard(this);
    wizard.setWindowModality(Qt::WindowModal);
    
    if (wizard.exec() == QDialog::Accepted) {
        // VM created successfully
        refreshVMList();
        m_statusLabel->setText(tr("Virtual machine created"));
    }
}

void MainWindow::onStartVM()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    m_statusLabel->setText(tr("Starting virtual machine..."));
    // In real implementation: m_frontend->startVM(vmId);
    
    updateActions();
}

void MainWindow::onPauseVM()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    m_statusLabel->setText(tr("Pausing virtual machine..."));
    // In real implementation: m_frontend->pauseVM(vmId);
}

void MainWindow::onResumeVM()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    m_statusLabel->setText(tr("Resuming virtual machine..."));
    // In real implementation: m_frontend->resumeVM(vmId);
}

void MainWindow::onStopVM()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    m_statusLabel->setText(tr("Saving virtual machine state..."));
    // In real implementation: m_frontend->stopVM(vmId);
}

void MainWindow::onPowerOffVM()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Power Off VM"),
        tr("Are you sure you want to power off this VM?"),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_statusLabel->setText(tr("Powering off virtual machine..."));
        // In real implementation: m_frontend->forceStopVM(vmId);
    }
}

void MainWindow::onTakeSnapshot()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    bool ok;
    QString snapshotName = QInputDialog::getText(
        this, tr("Take Snapshot"),
        tr("Snapshot Name:"), QLineEdit::Normal,
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"), &ok);
    
    if (ok && !snapshotName.isEmpty()) {
        m_statusLabel->setText(tr("Taking snapshot..."));
        // In real implementation: m_frontend->takeSnapshot(vmId, snapshotName);
    }
}

void MainWindow::onShowSettings()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    // Show device panel
    DevicePanel panel(this);
    panel.setVMId(vmId);
    panel.exec();
}

void MainWindow::onShowRawConfig()
{
    QString vmId = getCurrentVMId();
    if (vmId.isEmpty()) {
        return;
    }
    
    RawConfigMode dialog(this);
    dialog.setVMId(vmId);
    dialog.exec();
}

void MainWindow::refreshVMList()
{
    m_vmTree->clear();
    
    // In real implementation:
    // auto vms = m_frontend->getAllVMs();
    // for (const auto& vm : vms) {
    //     QTreeWidgetItem* item = new QTreeWidgetItem(m_vmTree);
    //     item->setText(0, vm.name);
    //     item->setText(1, vm.id);
    //     item->setText(2, vm.state);
    // }
    
    // Update status
    int vmCount = m_vmTree->topLevelItemCount();
    m_vmCountLabel->setText(tr("%n VM(s)", "", vmCount));
}

void MainWindow::refreshDetails()
{
    QString vmId = getCurrentVMId();
    
    m_detailsTable->setRowCount(0);
    
    if (vmId.isEmpty()) {
        return;
    }
    
    // In real implementation:
    // auto config = m_frontend->getVMConfig(vmId);
    // auto state = m_frontend->getVMState(vmId);
    // 
    // QStringList details = {
    //     tr("Name") + ":" + config.name,
    //     tr("OS Type") + ":" + config.ostype,
    //     tr("State") + ":" + stateToString(state),
    //     tr("Memory") + ":" + QString::number(config.hardware.memoryMB) + " MB",
    //     tr("CPUs") + ":" + QString::number(config.hardware.cpuCount),
    // };
    // 
    // int row = 0;
    // for (const QString& detail : details) {
    //     QStringList parts = detail.split(":");
    //     if (parts.size() == 2) {
    //         m_detailsTable->insertRow(row);
    //         m_detailsTable->setItem(row, 0, new QTableWidgetItem(parts[0]));
    //         m_detailsTable->setItem(row, 1, new QTableWidgetItem(parts[1]));
    //         row++;
    //     }
    // }
}

void MainWindow::onAbout()
{
    QMessageBox aboutBox(this);
    aboutBox.setWindowTitle(tr("About LibreVMM"));
    aboutBox.setText(tr(
        "<h3>LibreVMM</h3>"
        "<p>Version 1.0.0</p>"
        "<p>A platform-agnostic virtualization solution forked from VirtualBox.</p>"
        "<p>LibreVMM is a fork of the VirtualBox&reg; base package. "
        "VirtualBox is a registered trademark of Oracle Corporation. "
        "LibreVMM is not affiliated with, endorsed by, or sponsored by "
        "Oracle Corporation.</p>"
        "<p>Copyright (C) 2024 LibreVMM Contributors</p>"
        "<p>This program is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License.</p>"
    ));
    aboutBox.setStandardButtons(QMessageBox::Ok);
    aboutBox.exec();
}

void MainWindow::onExit()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Check for running VMs
    // if (m_frontend && m_frontend->getActiveSessionCount() > 0) {
    //     QMessageBox::StandardButton reply = QMessageBox::question(
    //         this, tr("Running VMs"),
    //         tr("There are running VMs. Do you want to power them off?"),
    //         QMessageBox::Yes | QMessageBox::No);
    //     
    //     if (reply == QMessageBox::Yes) {
    //         // Stop all VMs
    //     }
    // }
    
    event->accept();
}
