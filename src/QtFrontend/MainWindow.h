/**
 * @file MainWindow.h
 * @brief Main Window for Qt Frontend
 * 
 * Primary window implementation that mirrors VirtualBox 5.2 functionality.
 * Provides VM management, device configuration, and display output.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTableWidget>
#include <QAction>
#include <QVector>
#include <QString>
#include <QTimer>
#include <QIcon>

#include "../FrontendBase/FrontendBase.h"
#include "VMWizard.h"
#include "DevicePanel.h"
#include "RawConfigMode.h"

// Forward declarations
class QMenu;
class QAction;
class QToolButton;
class QTreeWidgetItem;
class VMDisplayWidget;

/**
 * @brief Main Window class for LibreVMM Qt Frontend
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Constructor
     * @param parent Parent widget
     */
    explicit MainWindow(QWidget* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~MainWindow();

signals:
    /**
     * @brief Signal emitted when VM list changes
     */
    void vmListChanged();
    
    /**
     * @brief Signal emitted when selection changes
     * @param vmId Selected VM ID
     */
    void vmSelectionChanged(const QString& vmId);

public slots:
    /**
     * @brief Create a new VM
     */
    void onNewVM();
    
    /**
     * @brief Start selected VM
     */
    void onStartVM();
    
    /**
     * @brief Pause selected VM
     */
    void onPauseVM();
    
    /**
     * @brief Resume selected VM
     */
    void onResumeVM();
    
    /**
     * @brief Stop selected VM
     */
    void onStopVM();
    
    /**
     * @brief Power off selected VM
     */
    void onPowerOffVM();
    
    /**
     * @brief Take snapshot of selected VM
     */
    void onTakeSnapshot();
    
    /**
     * @brief Show VM settings
     */
    void onShowSettings();
    
    /**
     * @brief Show raw configuration mode
     */
    void onShowRawConfig();
    
    /**
     * @brief Refresh VM list
     */
    void refreshVMList();
    
    /**
     * @brief Show about dialog
     */
    void onAbout();
    
    /**
     * @brief Exit application
     */
    void onExit();

protected:
    /**
     * @brief Override close event
     * @param event Close event
     */
    void closeEvent(QCloseEvent* event) override;

private:
    /**
     * @brief Initialize the UI
     */
    void initUI();
    
    /**
     * @brief Create menu bar
     */
    void createMenuBar();
    
    /**
     * @brief Create tool bar
     */
    void createToolBar();
    
    /**
     * @brief Create status bar
     */
    void createStatusBar();
    
    /**
     * @brief Create dock widgets
     */
    void createDockWidgets();
    
    /**
     * @brief Create VM list panel
     */
    void createVMListPanel();
    
    /**
     * @brief Create details panel
     */
    void createDetailsPanel();
    
    /**
     * @brief Create VM display widget
     */
    void createDisplayWidget();
    
    /**
     * @brief Update VM actions based on selection
     */
    void updateActions();
    
    /**
     * @brief Get current VM ID
     * @return Current VM ID or empty string
     */
    QString getCurrentVMId() const;

private:
    // Frontend base for common functionality
    FrontendBase* m_frontend;
    
    // UI Components
    QTreeWidget* m_vmTree;
    QTableWidget* m_detailsTable;
    VMDisplayWidget* m_displayWidget;
    
    // Docks
    QDockWidget* m_vmListDock;
    QDockWidget* m_detailsDock;
    QDockWidget* m_displayDock;
    
    // Menu
    QMenu* m_fileMenu;
    QMenu* m_machineMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    // Actions
    QAction* m_newVMAction;
    QAction* m_openVMAction;
    QAction* m_closeVMAction;
    QAction* m_startAction;
    QAction* m_pauseAction;
    QAction* m_resumeAction;
    QAction* m_stopAction;
    QAction* m_powerOffAction;
    QAction* m_snapshotAction;
    QAction* m_settingsAction;
    QAction* m_rawConfigAction;
    QAction* m_refreshAction;
    QAction* m_exitAction;
    QAction* m_aboutAction;
    
    // Tool buttons
    QToolButton* m_startToolButton;
    QToolButton* m_pauseToolButton;
    
    // Status bar
    QLabel* m_statusLabel;
    QLabel* m_vmCountLabel;
    
    // Update timer
    QTimer* m_updateTimer;
    
    // Current selection
    QString m_currentVMId;
};

/**
 * @brief VM Display Widget - Shows VM screen output
 */
class VMDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VMDisplayWidget(QWidget* parent = nullptr);
    ~VMDisplayWidget();

    /**
     * @brief Update display with new framebuffer data
     * @param width Framebuffer width
     * @param height Framebuffer height
     * @param data Framebuffer pixel data
     */
    void updateFramebuffer(int width, int height, const QVector<quint32>& data);

    /**
     * @brief Set display enabled/disabled
     * @param enabled Display enabled flag
     */
    void setDisplayEnabled(bool enabled);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage m_framebufferImage;
    bool m_displayEnabled;
};

#endif /* MAINWINDOW_H */
