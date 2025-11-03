#include "TrayIcon.h"
#include "resource.h"
#include "RegistryManager.h"
#include <shellapi.h> // For Shell_NotifyIcon

namespace {
    const UINT TRAY_ICON_ID = 1; // Unique ID for our icon
    NOTIFYICONDATAW nid = { sizeof(nid) };
}

namespace TrayIcon {

    BOOL Create(HWND hOwnerWnd, UINT uCallbackMessage, HICON hIcon, const wchar_t* szTip) {
        nid.hWnd = hOwnerWnd;
        nid.uID = TRAY_ICON_ID;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = uCallbackMessage;
        nid.hIcon = hIcon;
        if (szTip) {
            lstrcpynW(nid.szTip, szTip, (sizeof(nid.szTip) / sizeof(nid.szTip[0])));
        }
        else {
            nid.szTip[0] = L'\0';
        }

        // Add the icon
        BOOL result = Shell_NotifyIconW(NIM_ADD, &nid);

        // For Win7+, need to set version or messages might not work correctly
        nid.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &nid);

        return result;
    }

    BOOL Destroy(HWND hOwnerWnd) {
        nid.hWnd = hOwnerWnd;
        nid.uID = TRAY_ICON_ID;
        return Shell_NotifyIconW(NIM_DELETE, &nid);
    }

    void ShowContextMenu(HWND hOwnerWnd) {
        HMENU hMenu = LoadMenuW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDR_TRAYMENU));
        if (!hMenu) return;

        HMENU hSubMenu = GetSubMenu(hMenu, 0); // Get the first popup menu
        if (!hSubMenu) {
            DestroyMenu(hMenu);
            return;
        }

        // Dynamically check the correct radio item
        HMENU hAdvancedMenu = GetSubMenu(hSubMenu, 0);
        BOOL isFullBuffer = RegistryManager::GetBufferMode();

        // Apply radio checks to the 'hAdvancedMenu', not 'hSubMenu'
        CheckMenuRadioItem(hAdvancedMenu,               // Menu handle
            IDM_MODE_STREAMING,                     // First item in group
            IDM_MODE_FULLBUFFER,                    // Last item in group
            isFullBuffer ? IDM_MODE_FULLBUFFER : IDM_MODE_STREAMING, // Item to check
            MF_BYCOMMAND);                          // Find items by ID

        POINT pt;
        GetCursorPos(&pt);

        // SetForegroundWindow is necessary for the menu to disappear correctly
        SetForegroundWindow(hOwnerWnd);

        // TrackPopupMenu blocks until a selection is made or menu dismissed
        TrackPopupMenu(hSubMenu,
            TPM_LEFTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
            pt.x, pt.y,
            0, hOwnerWnd, NULL);

        // Post a dummy message to ensure focus returns correctly after menu closes (WinXP fix)
        PostMessage(hOwnerWnd, WM_NULL, 0, 0);

        DestroyMenu(hMenu);
    }

} // namespace TrayIcon
