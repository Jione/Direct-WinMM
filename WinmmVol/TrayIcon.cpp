#include "TrayIcon.h"
#include "resource.h"
#include "RegistryManager.h"
#include "UiLang.h"
#include <shellapi.h> // For Shell_NotifyIcon

// Find the submenu item index that hosts a given submenu HMENU
static int FindSubmenuIndex(HMENU hRootPopup, HMENU hSubmenu) {
    if (!hRootPopup || !hSubmenu) return -1;
    const int count = GetMenuItemCount(hRootPopup);
    for (int i = 0; i < count; ++i) {
        if (GetSubMenu(hRootPopup, i) == hSubmenu) return i;
    }
    return -1;
}

// Robustly set submenu caption without deleting the item
static void SetSubmenuCaptionSafe(HMENU hRootPopup, HMENU hSubmenu, const wchar_t* newText) {
    if (!hRootPopup || !hSubmenu || !newText) return;

    int pos = FindSubmenuIndex(hRootPopup, hSubmenu);
    if (pos < 0) return;

    MENUITEMINFOW mii = {};
    mii.cbSize = sizeof(mii);
    // Keep submenu handle and type, just set caption as string
    mii.fMask = MIIM_SUBMENU | MIIM_FTYPE | MIIM_STRING;
    mii.fType = MFT_STRING;
    mii.hSubMenu = hSubmenu;
    mii.dwTypeData = const_cast<LPWSTR>(newText);
    mii.cch = (UINT)lstrlenW(newText);
    SetMenuItemInfoW(hRootPopup, pos, TRUE, &mii);
}

static HMENU FindAdvancedSubmenu(HMENU hRootPopup) {
    if (!hRootPopup) return NULL;
    const int count = GetMenuItemCount(hRootPopup);
    for (int i = 0; i < count; ++i) {
        HMENU h = GetSubMenu(hRootPopup, i);
        if (h) return h;
    }
    return NULL;
}

static void LocalizeTrayMenu(HMENU hRootPopup) {
    if (!hRootPopup) return;

    std::wstring s;

    // Root items: About / Exit
    ModifyMenuW(hRootPopup, IDM_ABOUT, MF_BYCOMMAND | MF_STRING, IDM_ABOUT,
        UILang::Get(L"MENU_ABOUT", s));
    ModifyMenuW(hRootPopup, IDM_EXIT, MF_BYCOMMAND | MF_STRING, IDM_EXIT,
        UILang::Get(L"MENU_EXIT", s));

    // Localize submenu items
    ModifyMenuW(hRootPopup, IDM_MODE_AUTO, MF_BYCOMMAND | MF_STRING, IDM_MODE_AUTO,
        UILang::Get(L"MENU_ADVANCED_BUFFER_AUTO", s));
    ModifyMenuW(hRootPopup, IDM_MODE_STREAMING, MF_BYCOMMAND | MF_STRING, IDM_MODE_STREAMING,
        UILang::Get(L"MENU_ADVANCED_STREAMING", s));
    ModifyMenuW(hRootPopup, IDM_MODE_FULLBUFFER, MF_BYCOMMAND | MF_STRING, IDM_MODE_FULLBUFFER,
        UILang::Get(L"MENU_ADVANCED_FULLBUFFER", s));

    ModifyMenuW(hRootPopup, IDM_ENGINE_AUTO, MF_BYCOMMAND | MF_STRING, IDM_ENGINE_AUTO,
        UILang::Get(L"MENU_ADVANCED_ENGINE_AUTO", s));
    ModifyMenuW(hRootPopup, IDM_ENGINE_DS, MF_BYCOMMAND | MF_STRING, IDM_ENGINE_DS,
        UILang::Get(L"MENU_ADVANCED_DS", s));
    ModifyMenuW(hRootPopup, IDM_ENGINE_WASAPI, MF_BYCOMMAND | MF_STRING, IDM_ENGINE_WASAPI,
        UILang::Get(L"MENU_ADVANCED_WASAPI", s));

    // Localize Advanced submenu
    HMENU hAdvanced = FindAdvancedSubmenu(hRootPopup);
    if (hAdvanced) {
        SetSubmenuCaptionSafe(hRootPopup, hAdvanced, UILang::Get(L"MENU_ADVANCED", s));
    }
}

// Load icon helper
static HICON LoadSmallIcon(HINSTANCE hInst, UINT id) {
    int cx = GetSystemMetrics(SM_CXSMICON);
    int cy = GetSystemMetrics(SM_CYSMICON);
    HICON h = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(id), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
    if (!h) h = LoadIconW(hInst, MAKEINTRESOURCEW(id));
    return h;
}


namespace {
    const UINT TRAY_ICON_ID = 1; // Unique ID for our icon
    NOTIFYICONDATAW nid = { sizeof(nid) };

    // Cached icons for volume states
    HICON s_iconMute = NULL;
    HICON s_iconLv1  = NULL;
    HICON s_iconLv2  = NULL;
    HICON s_iconLv3  = NULL;

    // Choose icon by state
    static HICON ChooseIcon(BOOL isMute, int percent) {
        if (isMute || percent <= 0) return s_iconMute ? s_iconMute : nid.hIcon;
        if (percent <= 32)          return s_iconLv1 ? s_iconLv1 : nid.hIcon;
        if (percent <= 65)          return s_iconLv2 ? s_iconLv2 : nid.hIcon;
        return s_iconLv3 ? s_iconLv3 : nid.hIcon;
    }
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


        // Add icon
        BOOL ok = Shell_NotifyIconW(NIM_ADD, &nid);
        nid.uVersion = NOTIFYICON_VERSION_4;
        Shell_NotifyIconW(NIM_SETVERSION, &nid);

        // load cached icons once
        HINSTANCE hInst = GetModuleHandleW(NULL);
        if (!s_iconMute) s_iconMute = LoadSmallIcon(hInst, IDI_TRAYICON_MUTE);
        if (!s_iconLv1)  s_iconLv1 = LoadSmallIcon(hInst, IDI_TRAYICON_LVL1);
        if (!s_iconLv2)  s_iconLv2 = LoadSmallIcon(hInst, IDI_TRAYICON_LVL2);
        if (!s_iconLv3)  s_iconLv3 = LoadSmallIcon(hInst, IDI_TRAYICON_LVL3);

        // Immediately reflect current registry state
        RefreshFromRegistry();

        return ok;
    }

    BOOL Destroy(HWND hOwnerWnd) {
        nid.hWnd = hOwnerWnd; nid.uID = TRAY_ICON_ID;
        BOOL ok = Shell_NotifyIconW(NIM_DELETE, &nid);
        return ok;
    }

    void RefreshFromRegistry() {
        BOOL isMute = RegistryManager::GetMute();
        int percent = RegistryManager::GetVolumePercent();
        HICON hNew = ChooseIcon(isMute, percent);

        if (hNew) {
            nid.uFlags = NIF_ICON;
            nid.hIcon = hNew;
            Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
    }

    void ShowContextMenu(HWND hOwnerWnd) {
        HMENU hMenu = LoadMenuW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDR_TRAYMENU));
        if (!hMenu) return;

        HMENU hSubMenu = GetSubMenu(hMenu, 0); // Get the first popup menu
        if (!hSubMenu) {
            DestroyMenu(hMenu);
            return;
        }

        LocalizeTrayMenu(hSubMenu);

        // Dynamically check the correct radio item
        HMENU hAdvancedMenu = FindAdvancedSubmenu(hSubMenu);

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
