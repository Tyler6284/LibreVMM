/**
 * @file App.xaml.cpp
 * @brief WinRT Application Entry Point
 * 
 * UWP / Windows Mobile frontend entry point.
 * Implements the IVBoxFrontend API for Windows Store deployment.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#include "pch.h"
#include "App.xaml.h"

using namespace LibreVMM::WinRT;

#include <IVBoxFrontend.h>

// Application entry point
App::App()
{
    InitializeComponent();
    
    // Subscribe to activation events
    this->Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
    this->Resuming += ref new EventHandler<Object^>(this, &App::OnResuming);
}

// Called when the application is launched
void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
    // Create and show main window
    auto frame = ref new Frame();
    Window::Current->Content = frame;
    frame->Navigate(TypeName(MainPage::typeid));
    Window::Current->Activate();
}

// Called when application is suspended
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    // Save application state
    auto deferral = e->SuspendingOperation->GetDeferral();
    
    // TODO: Save VM state if running
    // This is critical for UWP - VMs should be paused or saved
    
    deferral->Complete();
}

// Called when application is resumed
void App::OnResuming(Object^ sender, Object^ e)
{
    // Restore application state
    // Refresh VM list from backend
}

// Get the global frontend instance
IVBoxFrontend* GetFrontend()
{
    static IVBoxFrontend* frontend = nullptr;
    if (!frontend) {
        // Create frontend - in real implementation, connect to actual backend
        // frontend = CreateVBoxFrontend(ExecutionBackendType::TCG_Interpreter);
    }
    return frontend;
}
