#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Custom message sent by the tray icon to the main window
#define WM_TRAYICON (WM_APP + 100)

namespace TrayIcon {
    // Adds the icon to the notification area
    BOOL Create(HWND hOwnerWnd, UINT uCallbackMessage, HICON hIcon, const wchar_t* szTip);

    // Removes the icon
    BOOL Destroy(HWND hOwnerWnd);

    // Shows the context menu near the cursor
    void ShowContextMenu(HWND hOwnerWnd);

    // Refresh tray icon from registry (mute + volume%)
    void RefreshFromRegistry();

    void SetShowAllApps(BOOL enable);
    BOOL GetShowAllApps();

} // namespace TrayIcon
