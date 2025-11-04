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

        // --- (Group 1) Set radio check for Buffer Mode ---
        int bufferMode = RegistryManager::GetBufferMode(); // Gets 0, 1, or 2
        UINT bufferItemToCheck = IDM_MODE_AUTO; // Default to Auto
        if (bufferMode == 1) {
            bufferItemToCheck = IDM_MODE_STREAMING;
        }
        else if (bufferMode == 2) {
            bufferItemToCheck = IDM_MODE_FULLBUFFER;
        }

        CheckMenuRadioItem(hAdvancedMenu,   // Menu handle
            IDM_MODE_AUTO,                  // First item in group
            IDM_MODE_FULLBUFFER,            // Last item in group
            bufferItemToCheck,              // Item to check
            MF_BYCOMMAND);                  // Find items by ID

        // --- (Group 2) Set radio check for Engine Mode ---
        int engineMode = RegistryManager::GetEngineMode(); // Get 0, 1, or 2
        UINT itemToCheck = IDM_ENGINE_AUTO;
        if (engineMode == 1) {
            itemToCheck = IDM_ENGINE_DS;
        }
        else if (engineMode == 2) {
            itemToCheck = IDM_ENGINE_WASAPI;
        }

        CheckMenuRadioItem(hAdvancedMenu,   // Menu handle
            IDM_ENGINE_AUTO,                // First item in group
            IDM_ENGINE_WASAPI,              // Last item in group
            itemToCheck,                    // Item to check
            MF_BYCOMMAND);                  // Find items by ID

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
