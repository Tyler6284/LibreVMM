/**
 * @file pch.h
 * @brief Precompiled Header for WinRT Frontend
 * 
 * This file serves as the precompiled header for the WinRT frontend.
 * 
 * IMPORTANT: This file is designed to work across platforms. The actual
 * WinRT headers are only available when building on Windows with the
 * Windows SDK. For cross-platform development, this file provides
 * minimal stubs that allow the code to be parsed by IDEs on any platform.
 * 
 * When building on Windows with Visual Studio and the Windows SDK:
 * - The real WinRT headers will be available
 * - The code will compile to a real UWP app
 * 
 * When building on other platforms:
 * - This stub version allows IDE IntelliSense to work
 * - The actual compilation will only happen on Windows
 * 
 * SPDX-License-Identifier: GPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright (C) 2024 LibreVMM Contributors
 */

#ifndef PCH_H
#define PCH_H

// Standard C++ Library - always available
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// On Windows with WinRT SDK, include the real headers
#if defined(_WIN32) && defined(__cpp_winrt)
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/Windows.UI.Xaml.Media.h>
#elif defined(_WIN32)
// On Windows without WinRT SDK, provide namespace stubs
namespace winrt {}
namespace Windows { namespace UI { namespace Xaml { namespace Controls { class Page; } } } }
#else
// On non-Windows platforms, provide minimal stubs for IDE support
// These are intentionally incomplete but allow IntelliSense to parse the code
namespace winrt {
    namespace Windows {
        namespace Foundation {
            template<typename T> using Collections = void;
        }
        namespace UI {
            namespace Xaml {
                namespace Controls {
                    class Page;
                    class Button;
                    class ListView;
                    class TextBlock;
                    class StackPanel;
                    class ScrollViewer;
                    class Border;
                }
                namespace Navigation {
                    class RoutedEventArgs;
                }
                namespace Media {
                    class Ellipse;
                }
            }
        }
    }
}
namespace Platform {
    template<typename T> class String;
    class Object;
}
#endif

// Project Headers
#include <IVBoxFrontend.h>

#endif // PCH_H
