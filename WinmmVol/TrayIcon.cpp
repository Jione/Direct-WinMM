#include "TrayIcon.h"
#include "resource.h"
#include "RegistryManager.h"
#include "VolumeSlider.h"
#include "UiLang.h"
#include <shellapi.h> // For Shell_NotifyIcon

namespace {

    const UINT TRAY_ICON_ID = 1; // Unique ID for our icon
    static NOTIFYICONDATAW nid = { sizeof(nid) };

    // Cached icons for volume states
    static HICON s_iconMute = NULL;
    static HICON s_iconLv1  = NULL;
    static HICON s_iconLv2  = NULL;
    static HICON s_iconLv3  = NULL;

    static BOOL g_showAllApps = FALSE;

    static void LocalizeTrayMenu(HMENU hRootPopup) {
        if (!hRootPopup) return;

        std::wstring s;

        // Root items: About / Exit
        ModifyMenuW(hRootPopup, IDM_INFO_USAGE, MF_BYCOMMAND | MF_STRING, IDM_INFO_USAGE,
            UILang::Get(L"MENU_USAGE", s));
        ModifyMenuW(hRootPopup, IDM_INFO_LICENSE, MF_BYCOMMAND | MF_STRING, IDM_INFO_LICENSE,
            UILang::Get(L"MENU_LICENSE", s));
        ModifyMenuW(hRootPopup, IDM_MODE_CLEAR, MF_BYCOMMAND | MF_STRING, IDM_MODE_CLEAR,
            UILang::Get(L"MENU_CLEAR", s));
        ModifyMenuW(hRootPopup, IDM_SHOW_ALL_APPS, MF_BYCOMMAND | MF_STRING, IDM_SHOW_ALL_APPS,
            UILang::Get(L"MENU_VIEWALL", s));
        ModifyMenuW(hRootPopup, IDM_EXIT, MF_BYCOMMAND | MF_STRING, IDM_EXIT,
            UILang::Get(L"MENU_EXIT", s));
    }

    // Load icon helper
    static HICON LoadSmallIcon(HINSTANCE hInst, UINT id) {
        int cx = GetSystemMetrics(SM_CXSMICON);
        int cy = GetSystemMetrics(SM_CYSMICON);
        HICON h = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(id), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
        if (!h) h = LoadIconW(hInst, MAKEINTRESOURCEW(id));
        return h;
    }

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
        nid.cbSize = sizeof(NOTIFYICONDATA);
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
        NOTIFYICONDATA nid = { 0, };
        nid.hWnd = hOwnerWnd;
        nid.uID = TRAY_ICON_ID;
        nid.cbSize = sizeof(NOTIFYICONDATA);
        BOOL ok = Shell_NotifyIconW(NIM_DELETE, &nid);
        return ok;
    }

    void RefreshFromRegistry() {
        const DWORD ov = RegistryManager::GetGlobalOverride();

        const BOOL isMute = RegistryManager::OV_GetMute(ov);
        const int  percent = RegistryManager::OV_GetVolume(ov);

        HICON hNew = ChooseIcon(isMute, percent);
        if (hNew) {
            nid.uFlags = NIF_ICON;
            nid.hIcon = hNew;
            Shell_NotifyIconW(NIM_MODIFY, &nid);
        }
    }

    void RefreshForTarget(const std::wstring& guid) {
        DWORD ov = guid.empty()
            ? RegistryManager::GetGlobalOverride()
            : RegistryManager::ComputeEffective(guid);
        const BOOL isMute = RegistryManager::OV_GetMute(ov);
        const int  percent = RegistryManager::OV_GetVolume(ov);
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

        CheckMenuItem(hSubMenu, IDM_SHOW_ALL_APPS,
            MF_BYCOMMAND | (g_showAllApps ? MF_CHECKED : MF_UNCHECKED));

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
        PostMessageW(hOwnerWnd, WM_NULL, 0, 0);

        DestroyMenu(hMenu);
    }

    void SetShowAllApps(BOOL enable) {
        g_showAllApps = enable ? TRUE : FALSE;
        VolumeSlider::SetShowAllApps(g_showAllApps);
    }

    BOOL GetShowAllApps() {
        return g_showAllApps;
    }

} // namespace TrayIcon
