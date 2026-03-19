/**
 * @file App.xaml.h
 * @brief WinRT Application Header
 * 
 * This file declares the App class that corresponds to App.xaml.
 * 
 * NOTE: This file uses WinRT C++/CX syntax which is only available on Windows
 * with Visual Studio and the Windows SDK. For cross-platform development,
 * this file provides minimal stubs to allow IDE parsing.
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef APP_XAML_H
#define APP_XAML_H

#include "pch.h"

#if defined(_WIN32) && defined(__cpp_winrt)

namespace LibreVMM { namespace WinRT {

/**
 * @brief WinRT Application Class
 * 
 * Main application class for the UWP / Windows Mobile frontend.
 */
public ref class App sealed : public Windows::UI::Xaml::Application
{
public:
    /// @brief Constructor
    App();

    /// @brief Launch event handler
    void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e);

protected:
    /// @brief Suspending event handler
    void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);

    /// @brief Resuming event handler
    void OnResuming(Platform::Object^ sender, Platform::Object^ e);

private:
    // InitializeComponent - in a real WinRT app this is generated
    void InitializeComponent();
};

} } // namespace LibreVMM::WinRT

#elif defined(_WIN32)
// Windows without WinRT - minimal stub
namespace LibreVMM { namespace WinRT {
class App {
public:
    App();
    void OnLaunched(void* e);
};
} }
#else
// Non-Windows platforms - minimal stub for IDE support
namespace LibreVMM { namespace WinRT {
class App {
public:
    App() {}
    void OnLaunched(void* e) {}
};
inline void* GetFrontend() { return nullptr; }
} }
#endif

#endif // APP_XAML_H
