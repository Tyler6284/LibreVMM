/**
 * @file qtmain.cpp
 * @brief Qt Frontend Main Entry Point
 * 
 * Primary Desktop UI for LibreVMM - Qt-based application entry point.
 * Mirrors VirtualBox 5.2 functionality while providing distinct visual identity.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QLocale>
#include <QTranslator>
#include <QLibraryInfo>
#include <csignal>

#include "MainWindow.h"
#include "../FrontendBase/FrontendBase.h"

// Global application instance
static QApplication* g_app = nullptr;
static MainWindow* g_mainWindow = nullptr;

/**
 * @brief Signal handler for clean shutdown
 */
void signalHandler(int signal)
{
    Q_UNUSED(signal);
    
    if (g_mainWindow) {
        g_mainWindow->close();
    }
    
    if (g_app) {
        g_app->quit();
    }
}

/**
 * @brief Load application stylesheet
 */
void loadStylesheet(QApplication& app)
{
    QStringList stylesheetPaths;
    
    // Check multiple locations for stylesheet
    stylesheetPaths << ":/resources/themes/LibreVMM.qss";
    stylesheetPaths << QCoreApplication::applicationDirPath() + "/resources/themes/LibreVMM.qss";
    stylesheetPaths << QDir::currentPath() + "/resources/themes/LibreVMM.qss";
    stylesheetPaths << "/usr/share/librevmm/themes/LibreVMM.qss";
    stylesheetPaths << "/usr/local/share/librevmm/themes/LibreVMM.qss";
    
    for (const QString& path : stylesheetPaths) {
        QFile file(path);
        if (file.exists() && file.open(QFile::ReadOnly)) {
            QString style = QString::fromUtf8(file.readAll());
            app.setStyleSheet(style);
            file.close();
            qDebug() << "Loaded stylesheet from:" << path;
            return;
        }
    }
    
    qWarning() << "Could not find LibreVMM.qss stylesheet";
}

/**
 * @brief Initialize translations
 */
void loadTranslations(QApplication& app)
{
    QTranslator* translator = new QTranslator(&app);
    QTranslator* qtTranslator = new QTranslator(&app);
    
    QStringList translationPaths;
    translationPaths << QCoreApplication::applicationDirPath() + "/translations";
    translationPaths << QDir::currentPath() + "/translations";
    translationPaths << "/usr/share/librevmm/translations";
    translationPaths << "/usr/local/share/librevmm/translations";
    
    QString systemLocale = QLocale::system().name();
    
    for (const QString& path : translationPaths) {
        // Try app translations
        QString appTransFile = path + "/librevmm_" + systemLocale + ".qm";
        if (QFile::exists(appTransFile)) {
            if (translator->load(appTransFile)) {
                app.installTranslator(translator);
            }
        }
        
        // Try Qt system translations
        QString qtTransFile = path + "/qt_" + systemLocale + ".qm";
        if (QFile::exists(qtTransFile)) {
            if (qtTranslator->load(qtTransFile)) {
                app.installTranslator(qtTranslator);
            }
        }
    }
    
    // Fall back to Qt translations from system
    if (qtTranslator->load("qt_" + QLocale::system().name(),
                           QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        app.installTranslator(qtTranslator);
    }
}

int main(int argc, char *argv[])
{
    // Enable high DPI scaling
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // Create application
    QApplication app(argc, argv);
    g_app = &app;
    
    // Set application metadata
    QCoreApplication::setApplicationName("LibreVMM");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("LibreVMM");
    QCoreApplication::setOrganizationDomain("librevmm.org");
    QCoreApplication::setDesktopFileName("librevmm");
    
    // Set application display name
    QGuiApplication::setDisplayName("LibreVMM");
    
    // Load translations
    loadTranslations(app);
    
    // Load stylesheet
    loadStylesheet(app);
    
    // Set default font (Ubuntu Sans or system default)
    QFont defaultFont = QFont("Ubuntu Sans", 10);
    if (!QFontDatabase().families().contains("Ubuntu Sans")) {
        defaultFont = QFont("Segoe UI", 10);  // Windows fallback
    }
    if (!QFontDatabase().families().contains(defaultFont.family())) {
        defaultFont = QFont();  // System default
    }
    app.setFont(defaultFont);
    
    // Set quit on last window closed
    app.setQuitOnLastWindowClosed(true);
    
    // Set application icon (will be loaded from resources later)
    // app.setWindowIcon(QIcon(":/resources/icons/librevmm.png"));
    
    // Install signal handler for clean shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Create and show main window
    MainWindow mainWindow;
    g_mainWindow = &mainWindow;
    
    // Show window
    mainWindow.show();
    
    // Execute application
    int result = app.exec();
    
    // Cleanup
    g_mainWindow = nullptr;
    g_app = nullptr;
    
    return result;
}
