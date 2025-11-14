#include "VolumeSlider.h"
#include "RegistryManager.h"
#include "resource.h"
#include "CustomSlider.h"
#include "TrayIcon.h"
#include "UiLang.h"
#include "WineCompat.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <algorithm>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Version.lib")

namespace {

    static const wchar_t* SLIDER_WINDOW_CLASS = L"WinMMStubsVolumePanel";

    // Window + child handles
    static HWND g_hwndPanel     = NULL;
    static HWND g_hwndIcon      = NULL; // speaker icon
    static HWND g_hwndValue     = NULL;
    static HWND g_hwndSlider    = NULL; // CustomSlider
    static HWND g_hwndLblApp    = NULL;
    static HWND g_hwndCmbApp    = NULL;
    static HWND g_hwndLblVolume = NULL;
    static HWND g_hwndLblEngine = NULL;
    static HWND g_hwndCmbEngine = NULL;
    static HWND g_hwndLblBuffer = NULL;
    static HWND g_hwndCmbBuffer = NULL;
    static HWND g_hwndBtnClose  = NULL; // close button

    // Icons
    static HICON g_hIconMute  = NULL;
    static HICON g_hIconLvl1  = NULL;
    static HICON g_hIconLvl2  = NULL;
    static HICON g_hIconLvl3  = NULL;
    static HICON g_hIconClose = NULL;

    static BOOL g_isMuted     = FALSE;
    static BOOL g_showAllApps = FALSE;

    static HFONT g_hFontSpecial = NULL;


    // current target (empty = Global, otherwise GUID)
    static std::wstring g_currentGuid;
    static DWORD g_curOverride = RegistryManager::OV_DEFAULT; // cache of current target's override

    static const UINT_PTR kSubId_LabelDrag = 0xD0A0;

    // App combo item payload
    struct AppItem {
        std::wstring guid;       // empty for Global
        std::wstring exePath;    // for app rows
        std::wstring display;    // caption(filename)
        HICON        hIcon;      // per-row small icon
        BOOL         isSpecial;
        AppItem() : hIcon(NULL), isSpecial(FALSE) {}
    };

    static std::vector<AppItem*> g_items; // managed; free on destroy

    // Layout
    struct Layout {
        int width = 320;
        int height = 400;

        int padX = 16;
        int padY = 14;
        int gapY = 36;

        int rowH = 28;     // label/combos
        int labelW = 260;  // label width
        int comboH = 24;
        int comboW = 280;

        // Volume row
        int volIconW = 24;
        int volIconH = 18;
        int volTrackH = 28;
        int volNumberW = 40;

        // App icon at left of app combo
        int appIconW = 20;
        int appIconH = 20;
        int appIconGap = 6;
    } L;

    // remember last show position
    static POINT g_lastShowPt = { 0, 0 };

    // Helpers -----------------------------------------------------------------

    static HFONT CreateUIFont(LONG h, LONG w) {
        LOGFONTW lf = {};
        lf.lfHeight = h;
        lf.lfWeight = w;
        lstrcpyW(lf.lfFaceName, L"Segoe UI");
        return CreateFontIndirectW(&lf);
    }

    static void PaintBackground(HWND hwnd, HDC hdc) {
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(66, 66, 66));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(142, 142, 142));
        HPEN old = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, rc.left, rc.top, NULL);
        LineTo(hdc, rc.right - 1, rc.top);
        LineTo(hdc, rc.right - 1, rc.bottom - 1);
        LineTo(hdc, rc.left, rc.bottom - 1);
        LineTo(hdc, rc.left, rc.top);
        SelectObject(hdc, old);
        DeleteObject(pen);
    }

    static const wchar_t* SimpleUiText(const wchar_t* key, std::wstring& tmp) {
        const wchar_t* p = UILang::Get(key, tmp);
        return (p && *p) ? p : L"";
    }

    static void UpdateValueText(int percent) {
        if (!g_hwndValue) return;
        wchar_t buf[12];
        wsprintfW(buf, L"%d", percent);
        SetWindowTextW(g_hwndValue, buf);
    }

    static void ApplyThemeToSlider(HWND hCtrl) {
        CustomSlider::Theme t;
        t.bg = RGB(66, 66, 66);
        t.trackBg = RGB(142, 142, 142);
        t.trackFill = RGB(200, 200, 200);
        t.thumb = RGB(240, 240, 240);
        t.trackH = 4;
        t.thumbR = 6;
        t.paddingX = 6;
        CustomSlider::SetTheme(hCtrl, t);
    }

    static void UpdateVolumeIcon(int percent, BOOL isMuted) {
        if (!g_hwndIcon) return;
        HICON hIcon = NULL;
        if (isMuted || percent <= 0)       hIcon = g_hIconMute;
        else if (percent <= 32)            hIcon = g_hIconLvl1;
        else if (percent <= 65)            hIcon = g_hIconLvl2;
        else                               hIcon = g_hIconLvl3;
        SendMessageW(g_hwndIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
        g_isMuted = isMuted;
    }

    static void AppendDefaultSuffix(std::wstring& s) {
        std::wstring tmp;
        s.append(L" (").append(UILang::Get(L"UI_DEFAULT", tmp)).append(L")");
    }

    static const wchar_t* Txt(const wchar_t* key) {
        static std::wstring tmp; // not thread-safe, ok for UI thread
        return UILang::Get(key, tmp);
    }

    static void RebuildEngineCombo(BOOL isGlobalScope) {
        SendMessageW(g_hwndCmbEngine, CB_RESETCONTENT, 0, 0);

        const BOOL isVista = RegistryManager::IsVistaOrLater();
        EnableWindow(g_hwndCmbEngine, TRUE);

        if (!isVista) {
            // XP
            if (isGlobalScope) {
                // Global(XP): [ DS(Default), WaveOut ]
                std::wstring s = Txt(L"UI_ENGINE_DS"); AppendDefaultSuffix(s);
                SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)s.c_str());                // idx0 -> field 0 (Auto=DS default)
                SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_WAVEOUT"));// idx1 -> field 3
            }
            else {
                // App(XP): [ Follow, DS, WaveOut ]
                SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_FOLLOW_GLOBAL")); // idx0 -> field 0
                SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_DS"));     // idx1 -> field 1
                SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_WAVEOUT"));// idx2 -> field 3
            }
            return;
        }

        // Vista+
        if (isGlobalScope) {
            // Global(Vista+): [ WASAPI(Default), DS, WaveOut ]
            std::wstring s = Txt(L"UI_ENGINE_WASAPI"); AppendDefaultSuffix(s);
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)s.c_str());                   // idx0 -> field 0 (default)
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_DS"));        // idx1 -> field 1
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_WAVEOUT"));   // idx2 -> field 3
        }
        else {
            // App(Vista+): [ Follow, WASAPI, DS, WaveOut ]
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_FOLLOW_GLOBAL"));    // idx0 -> field 0
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_WASAPI"));    // idx1 -> field 2
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_DS"));        // idx2 -> field 1
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_ENGINE_WAVEOUT"));   // idx3 -> field 3
        }
    }

    static void RebuildBufferCombo(BOOL isGlobalScope) {
        SendMessageW(g_hwndCmbBuffer, CB_RESETCONTENT, 0, 0);

        if (isGlobalScope) {
            std::wstring s = Txt(L"UI_BUFFER_STREAMING");
            AppendDefaultSuffix(s);
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)s.c_str());                   // idx 0 -> field 1
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_BUFFER_FULL"));      // idx 1 -> field 2
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_BUFFER_RESAMPLE"));  // idx 2 -> field 3
        }
        else {
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_FOLLOW_GLOBAL"));    // idx 0 -> field 0
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_BUFFER_STREAMING")); // idx 1 -> field 1
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_BUFFER_FULL"));      // idx 2 -> field 2
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)Txt(L"UI_BUFFER_RESAMPLE"));  // idx 3 -> field 3
        }
    }

    static void RebuildCombosForScope() {
        const BOOL isGlobal = g_currentGuid.empty() ? TRUE : FALSE;
        RebuildEngineCombo(isGlobal);
        RebuildBufferCombo(isGlobal);
    }

    // small utils to read/write current target override -----------------------

    static DWORD ReadTargetOverride() {
        if (g_currentGuid.empty()) {
            return RegistryManager::GetGlobalOverride();
        }
        else {
            DWORD ov = RegistryManager::OV_DEFAULT;
            RegistryManager::GetAppOverride(g_currentGuid, ov);
            return ov;
        }
    }

    static void WriteTargetOverride(DWORD ov, bool refreshTray = true) {
        if (g_currentGuid.empty()) {
            RegistryManager::SetGlobalOverride(ov);
        }
        else {
            RegistryManager::SetAppOverride(g_currentGuid, ov);
        }
        g_curOverride = ov;
        if (refreshTray) {
            TrayIcon::RefreshForTarget(g_currentGuid);
        }
    }

    // sync UI controls (engine/buffer/slider/mute) from current target override
    static void SyncControlsFromTarget() {
        g_curOverride = ReadTargetOverride();

        // -------- Engine selection ----------
        const BOOL isVista = RegistryManager::IsVistaOrLater();
        const BOOL isGlobal = g_currentGuid.empty() ? TRUE : FALSE;
        int engine = RegistryManager::OV_GetEngine(g_curOverride); // field: 0=Auto(default),1=DS,2=WASAPI,3=WaveOut
        int engineSel = 0;

        if (!isVista) {
            // XP
            if (isGlobal) {
                // [ DS(Default)=idx0(field0), WaveOut=idx1(field3) ]
                engineSel = (engine == 3) ? 1 : 0; // 3 -> idx1, others(0/1/2) -> idx0
            }
            else {
                // App: [ Follow=0->0, DS=1->1, WaveOut=2->3 ]
                if (engine == 0) engineSel = 0;          // Follow
                else if (engine == 1) engineSel = 1;          // DS
                else if (engine == 3) engineSel = 2;          // WaveOut
                else                  engineSel = 0;
            }
            SendMessageW(g_hwndCmbEngine, CB_SETCURSEL, engineSel, 0);
        }
        else {
            // Vista+
            if (isGlobal) {
                // [ WASAPI(Default)=idx0(field0), DS=idx1(field1), WaveOut=idx2(field3) ]
                if (engine == 1) engineSel = 1;
                else if (engine == 3) engineSel = 2;
                else                  engineSel = 0; // 0 or 2 -> Default
            }
            else {
                // [ Follow=0->0, WASAPI=1->2, DS=2->1, WaveOut=3->3 ]
                if (engine == 0) engineSel = 0;
                else if (engine == 2) engineSel = 1;
                else if (engine == 1) engineSel = 2;
                else                  engineSel = 3; // 3
            }
            SendMessageW(g_hwndCmbEngine, CB_SETCURSEL, engineSel, 0);
        }

        // -------- Buffer selection ----------
        int buf = RegistryManager::OV_GetBuffer(g_curOverride); // field: 0(auto),1(stream),2(full),3(resample)
        int bufSel = 0;
        if (isGlobal) {
            // Global UI: [ Streaming(Default)=idx0->field1, Full=idx1->field2, Resample=idx2->field3 ]
            if (buf == 2) bufSel = 1;
            else if (buf == 3) bufSel = 2;
            else /* 0(auto) or 1(stream) */ bufSel = 0; // Streaming(Default)
            SendMessageW(g_hwndCmbBuffer, CB_SETCURSEL, bufSel, 0);
        }
        else {
            // App UI: [ Follow=idx0->field0, Streaming=1, Full=2, Resample=3 ]
            if (buf == 0) bufSel = 0;
            else if (buf == 1) bufSel = 1;
            else if (buf == 2) bufSel = 2;
            else               bufSel = 3; // 3
            SendMessageW(g_hwndCmbBuffer, CB_SETCURSEL, bufSel, 0);
        }

        // -------- Volume / Mute ----------
        const int  percent = RegistryManager::OV_GetVolume(g_curOverride);
        const BOOL isMuted = RegistryManager::OV_GetMute(g_curOverride);
        CustomSlider::SetValue(g_hwndSlider, percent, FALSE);
        UpdateValueText(percent);
        UpdateVolumeIcon(percent, isMuted);
    }

    // App caption/icon helpers ------------------------------------------------

    static bool GetFileDescription(const std::wstring& path, std::wstring& out) {
        DWORD handle = 0;
        DWORD sz = GetFileVersionInfoSizeW(path.c_str(), &handle);
        if (!sz) return false;

        std::vector<BYTE> buf(sz);
        if (!GetFileVersionInfoW(path.c_str(), 0, sz, buf.data())) return false;

        struct LANGANDCODEPAGE { WORD wLanguage; WORD wCodePage; } *lpTranslate;
        UINT cbTranslate = 0;
        if (!VerQueryValueW(buf.data(), L"\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, &cbTranslate) || !cbTranslate)
            return false;

        wchar_t subblock[64];
        wsprintfW(subblock, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
        LPWSTR val = NULL; UINT cch = 0;
        if (!VerQueryValueW(buf.data(), subblock, (LPVOID*)&val, &cch) || !val || !*val) return false;
        out.assign(val, val + lstrlenW(val));
        return true;
    }

    static HICON GetSmallIconForFile(const std::wstring& path) {
        SHFILEINFOW sfi = {};
        if (SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) {
            return sfi.hIcon; // caller must DestroyIcon
        }
        return NULL;
    }

    static std::wstring FileNameFromPath(const std::wstring& p) {
        wchar_t buf[MAX_PATH]; lstrcpynW(buf, p.c_str(), MAX_PATH);
        return std::wstring(PathFindFileNameW(buf));
    }

    // Owner-draw for app combo -------------------------------------------------

    static void ClearAppItems() {
        for (auto* it : g_items) {
            if (it->hIcon) DestroyIcon(it->hIcon);
            delete it;
        }
        g_items.clear();
    }

    static void PopulateAppCombo() {
        SendMessageW(g_hwndCmbApp, CB_RESETCONTENT, 0, 0);
        ClearAppItems();

        // 0: Global
        {
            std::wstring tmp;
            AppItem* ai = new AppItem();
            ai->guid = L"";
            ai->exePath.clear();
            ai->display = UILang::Get(L"UI_APP_GLOBAL", tmp);
            ai->hIcon = (HICON)LoadImageW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDI_APPICON),
                IMAGE_ICON, L.appIconW, L.appIconH, LR_DEFAULTCOLOR);
            g_items.push_back(ai);
            int idx = (int)SendMessageW(g_hwndCmbApp, CB_ADDSTRING, 0, (LPARAM)ai->display.c_str());
            SendMessageW(g_hwndCmbApp, CB_SETITEMDATA, idx, (LPARAM)ai);
        }

        // live apps
        std::vector<RegistryManager::AppEntry> apps;
        RegistryManager::EnumApps(apps, g_showAllApps ? FALSE : TRUE);

        std::sort(apps.begin(), apps.end(),
            [](const RegistryManager::AppEntry& a, const RegistryManager::AppEntry& b) {
                if (a.live != b.live) return a.live > b.live;            // live first
                return a.lastSeen > b.lastSeen;                          // most recently seen first
            });

        for (auto& e : apps) {
            // When "Show all apps" is ON, hide items whose executable path does not exist.
            if (g_showAllApps) {
                if (e.exePath.empty() || !PathFileExistsW(e.exePath.c_str())) {
                    continue;
                }
            }

            AppItem* ai = new AppItem();
            ai->guid = e.guid;
            ai->exePath = e.exePath;

            // live -> "Caption (filename.exe)", non-live(pid==0) -> "filename.exe"
            std::wstring caption, filename;

            // filename from @ (exePath)
            filename = FileNameFromPath(e.exePath);
            if (!e.live || e.pid == 0) {
                ai->display = filename;  // non-live: show only filename
            }
            else {
                if (!GetFileDescription(e.exePath, caption)) {
                    caption = filename;
                    while (!caption.empty() && caption.back() == L'\0') caption.pop_back();
                }
                ai->display = caption + L" (" + filename + L")";
            }

            // icon
            ai->hIcon = GetSmallIconForFile(e.exePath);

            g_items.push_back(ai);
            int idx = (int)SendMessageW(g_hwndCmbApp, CB_ADDSTRING, 0, (LPARAM)ai->display.c_str());
            SendMessageW(g_hwndCmbApp, CB_SETITEMDATA, idx, (LPARAM)ai);
        }

        // Selective recovery logic
        int sel = -1;
        if (!g_currentGuid.empty()) {
            const int count = (int)SendMessageW(g_hwndCmbApp, CB_GETCOUNT, 0, 0);
            for (int i = 0; i < count; ++i) {
                AppItem* ai = (AppItem*)SendMessageW(g_hwndCmbApp, CB_GETITEMDATA, i, 0);
                if (ai && ai->guid == g_currentGuid) { sel = i; break; }
            }
        }
        if (sel < 0) {
            sel = 0;
            g_currentGuid.clear();
        }
        SendMessageW(g_hwndCmbApp, CB_SETCURSEL, sel, 0);

        // when Show All Apps is OFF, append a special tail item
        if (!g_showAllApps && IsWindow(g_hwndCmbApp)) {
            // Show All Apps...
            std::wstring tmp1, label1 = UILang::Get(L"UI_SHOW_ALL_APPS", tmp1);
            int tail1 = (int)SendMessageW(g_hwndCmbApp, CB_ADDSTRING, 0, (LPARAM)label1.c_str());
            if (tail1 >= 0) {
                AppItem* sai = new AppItem();
                sai->isSpecial = TRUE;
                sai->guid = L"#SHOW_ALL_APPS#";
                sai->display = label1;
                SendMessageW(g_hwndCmbApp, CB_SETITEMDATA, tail1, (LPARAM)sai);
                g_items.push_back(sai);
            }

            // Patch .exe file (for Wine)
            std::wstring tmp2, label2 = UILang::Get(L"UI_PATCH_FOR_WINE", tmp2);
            int tail2 = (int)SendMessageW(g_hwndCmbApp, CB_ADDSTRING, 0, (LPARAM)label2.c_str());
            if (tail2 >= 0) {
                AppItem* wpi = new AppItem();
                wpi->isSpecial = TRUE;
                wpi->guid = L"#PATCH_FOR_WINE#";
                wpi->display = label2;
                // no icon for sentinel
                SendMessageW(g_hwndCmbApp, CB_SETITEMDATA, tail2, (LPARAM)wpi);
                g_items.push_back(wpi);
            }
        }
    }

    // Sync all controls from current target
    static void UpdateDisplayAll() {
        if (!IsWindow(g_hwndPanel)) return;
        SyncControlsFromTarget();
        TrayIcon::RefreshFromRegistry();
    }

    static void CloseAllDropdownsAndBlurToPanel(HWND hwndPanel) {
        if (g_hwndCmbApp && SendMessageW(g_hwndCmbApp, CB_GETDROPPEDSTATE, 0, 0))
            SendMessageW(g_hwndCmbApp, CB_SHOWDROPDOWN, FALSE, 0);
        if (g_hwndCmbEngine && SendMessageW(g_hwndCmbEngine, CB_GETDROPPEDSTATE, 0, 0))
            SendMessageW(g_hwndCmbEngine, CB_SHOWDROPDOWN, FALSE, 0);
        if (g_hwndCmbBuffer && SendMessageW(g_hwndCmbBuffer, CB_GETDROPPEDSTATE, 0, 0))
            SendMessageW(g_hwndCmbBuffer, CB_SHOWDROPDOWN, FALSE, 0);
        if (IsWindow(hwndPanel)) SetFocus(hwndPanel);
    }

    // Drawing for owner-draw combo
    static void DrawComboItem(const DRAWITEMSTRUCT* dis) {
        if (!dis || dis->hwndItem != g_hwndCmbApp) return;

        HDC hdc = dis->hDC;
        RECT rc = dis->rcItem;

        BOOL isSpecial = FALSE;

        // Resolve AppItem* safely
        AppItem* ai = nullptr;
        if (dis->itemID != (UINT)-1) {
            ai = (AppItem*)dis->itemData;
            if (ai == (AppItem*)0xFFFFFFFF) ai = nullptr;
        }
        else {
            int sel = (int)SendMessageW(g_hwndCmbApp, CB_GETCURSEL, 0, 0);
            if (sel >= 0) {
                ai = (AppItem*)SendMessageW(g_hwndCmbApp, CB_GETITEMDATA, sel, 0);
                if (ai == (AppItem*)0xFFFFFFFF) ai = nullptr;
            }
        }
        if (ai) {
            if (ai->isSpecial) isSpecial = TRUE;
            else if (ai->guid == L"#SHOW_ALL_APPS#" || ai->guid == L"#PATCH_FOR_WINE#") isSpecial = TRUE;
        }

        HFONT hOldFont = NULL;
        if (isSpecial && g_hFontSpecial) {
            HFONT hOldFont = (HFONT)SelectObject(hdc, g_hFontSpecial);
        }
        
        // use system colors so it matches other standard combos
        const BOOL isEditField = (dis->itemID == (UINT)-1);
        COLORREF bg = isEditField
            ? GetSysColor(COLOR_WINDOW)
            : ((dis->itemState & ODS_SELECTED) ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW));
        COLORREF fg = isEditField
            ? GetSysColor(COLOR_WINDOWTEXT)
            : ((dis->itemState & ODS_SELECTED) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT));

        if (!isEditField && isSpecial) {
            if (dis->itemState & ODS_SELECTED) {
                bg = RGB(43, 43, 43);
                fg = RGB(240, 240, 240);
            }
            else {
                bg = RGB(200, 200, 200);
                fg = RGB(43, 43, 43);
            }
        }
        HBRUSH hb = CreateSolidBrush(bg);
        FillRect(hdc, &rc, hb);
        DeleteObject(hb);

        // layout
        int x = rc.left + 4;
        int y = rc.top + ((rc.bottom - rc.top) - L.appIconH) / 2;

        if (isSpecial) {
            x += L.appIconGap * 2;
        }
        else {
            if (ai && ai->hIcon) {
                DrawIconEx(hdc, x, y, ai->hIcon, L.appIconW, L.appIconH, 0, NULL, DI_NORMAL);
            }
            x += L.appIconW + L.appIconGap;
        }

        // text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, fg);

        const wchar_t* text = L"";
        std::wstring tmp;
        if (ai) text = ai->display.c_str();
        else {
            text = UILang::Get(L"UI_APP_GLOBAL", tmp);
            if (!text || !*text) text = L"(unknown)";
        }
        RECT textRect = { x, rc.top, rc.right - 4, rc.bottom };
        DrawTextW(hdc, text, -1, &textRect,
            DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

        if (hOldFont) SelectObject(hdc, hOldFont);
    }

    static void MeasureComboItem(MEASUREITEMSTRUCT* mis) {
        if (mis->CtlID != IDC_APP_COMBO) return;
        mis->itemHeight = (L.comboH > (L.appIconH + 4)) ? L.comboH : (L.appIconH + 4);
    }

    // Return TRUE if pt (client coords) is inside rc
    static BOOL PtIn(const RECT& rc, POINT pt) {
        return (pt.x >= rc.left && pt.x < rc.right && pt.y >= rc.top && pt.y < rc.bottom);
    }

    // Get child window rect in client coords (returns FALSE if hwnd invalid)
    static BOOL GetChildRectClient(HWND hParent, HWND hChild, RECT& outRc) {
        if (!IsWindow(hParent) || !IsWindow(hChild)) return FALSE;
        RECT r; if (!GetWindowRect(hChild, &r)) return FALSE;
        POINT tl = { r.left, r.top }, br = { r.right, r.bottom };
        ScreenToClient(hParent, &tl); ScreenToClient(hParent, &br);
        outRc.left = tl.x; outRc.top = tl.y; outRc.right = br.x; outRc.bottom = br.y;
        return TRUE;
    }

    // Decide if a drag should start from this client point
    static BOOL ShouldStartDragFrom(POINT ptClient) {
        // Determine dynamic title band height = bottom of title buttons (+ small margin)
        int titleBottom = 0;
        RECT rcBtn;
        if (GetChildRectClient(g_hwndPanel, g_hwndBtnClose, rcBtn)) {
            if (rcBtn.bottom > titleBottom) titleBottom = rcBtn.bottom;
        }
        if (titleBottom == 0) {
            // no buttons yet; fall back to a thin top band
            titleBottom = 22;
        }
        titleBottom += 2; // small margin below buttons

        RECT rcDrag = { 0, 0, L.width, titleBottom };

        // Exclude the interactive button rectangles
        RECT rcClose;
        if (GetChildRectClient(g_hwndPanel, g_hwndBtnClose, rcClose) && PtIn(rcClose, ptClient)) return FALSE;

        // Do not start drag when clicking on any child within the band (e.g., if a control overlaps)
        HWND hChild = ChildWindowFromPointEx(g_hwndPanel, ptClient, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
        if (hChild && hChild != g_hwndPanel && PtIn(rcDrag, ptClient)) {
            // allow drag only if the hit is the panel itself (not a child)
            POINT pt2 = ptClient;
            HWND hHit = ChildWindowFromPointEx(g_hwndPanel, pt2, CWP_ALL);
            if (hHit && hHit != g_hwndPanel) return FALSE;
        }

        return PtIn(rcDrag, ptClient) ? TRUE : FALSE;
    }

    // Label subclass procedure (for dragging IDC_LABEL_APP)
    static LRESULT CALLBACK LabelDragSubclassProc(
        HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam,
        UINT_PTR id, DWORD_PTR data)
    {
        switch (msg) {
        case WM_LBUTTONDOWN: {
            HWND hParent = GetParent(hWnd);
            if (IsWindow(hParent)) {
                POINT pt; GetCursorPos(&pt); // screen coords
                ReleaseCapture();
                SendMessageW(hParent, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(pt.x, pt.y));
                POINT pt2; GetCursorPos(&pt2);
                g_lastShowPt.x += pt2.x - pt.x;
                g_lastShowPt.y += pt2.y - pt.y;
                return 0;
            }
            break;
        }
        }
        return DefSubclassProc(hWnd, msg, wParam, lParam);
    }

    // Window procedure --------------------------------------------------------

    LRESULT CALLBACK PanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            g_hwndPanel = hwnd;
            HFONT hFont = CreateUIFont(-13, FW_MEDIUM);
            HFONT hLabelFont = CreateUIFont(-15, FW_BOLD);
            HFONT hNumFont = CreateUIFont(-18, FW_BOLD);
            {
                LOGFONTW lf = {};
                lf.lfHeight = -13;
                lf.lfWeight = FW_BOLD;
                lf.lfItalic = true;
                lstrcpyW(lf.lfFaceName, L"Segoe UI");
                g_hFontSpecial = CreateFontIndirectW(&lf);
            }

            // load speaker, min, close icons
            {
                HINSTANCE hInst = GetModuleHandleW(NULL);
                g_hIconMute  = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_MUTE), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl1  = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL1), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl2  = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL2), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl3  = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL3), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconClose = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_BTN_CLOSE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
            }

            // Create custom title buttons at top-right
            constexpr int btnW = 20;
            constexpr int btnH = 20;
            constexpr int topY = 6;

            g_hwndBtnClose = CreateWindowExW(0, L"BUTTON", L"",
                WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                (L.width - topY - btnW), topY, btnW, btnH,
                hwnd, (HMENU)IDC_BTN_CLOSE, GetModuleHandle(NULL), NULL);

            int x = L.padX;
            int y = L.padY;
            std::wstring tmp;

            // Row 1: App label + (icon + combo)
            g_hwndLblApp = CreateWindowExW(0, L"STATIC",
                SimpleUiText(L"UI_APP_SCOPE", tmp),
                WS_CHILD | WS_VISIBLE, x, y, L.labelW, L.rowH,
                hwnd, (HMENU)IDC_LABEL_APP, GetModuleHandle(NULL), NULL);
            if (hLabelFont) SendMessageW(g_hwndLblApp, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
            y += L.rowH;

            // owner-draw combo for app list
            g_hwndCmbApp = CreateWindowExW(0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | CBS_OWNERDRAWFIXED | WS_VSCROLL | CBS_HASSTRINGS,
                x, y, L.comboW, L.comboH + 240,
                hwnd, (HMENU)IDC_APP_COMBO, GetModuleHandle(NULL), NULL);
            if (hFont) SendMessageW(g_hwndCmbApp, WM_SETFONT, (WPARAM)hFont, TRUE);

            {
                HWND hLblApp = GetDlgItem(hwnd, IDC_LABEL_APP);
                if (IsWindow(hLblApp)) {
                    // make sure it will send/receive mouse messages as a child control
                    LONG st = GetWindowLongW(hLblApp, GWL_STYLE);
                    st |= SS_NOTIFY; // optional but safe to enable
                    SetWindowLongW(hLblApp, GWL_STYLE, st);

                    // install subclass to start window drag on LButtonDown
                    SetWindowSubclass(hLblApp, LabelDragSubclassProc, kSubId_LabelDrag, 0);
                }
            }

            PopulateAppCombo();

            y += L.comboH + L.gapY;

            // Row 2: Engine label
            g_hwndLblEngine = CreateWindowExW(0, L"STATIC",
                SimpleUiText(L"UI_ENGINE", tmp),
                WS_CHILD | WS_VISIBLE, x, y, L.labelW, L.rowH,
                hwnd, (HMENU)IDC_LABEL_ENGINE, GetModuleHandle(NULL), NULL);
            if (hLabelFont) SendMessageW(g_hwndLblEngine, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
            y += L.rowH;

            g_hwndCmbEngine = CreateWindowExW(0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                x, y, L.comboW, L.comboH + 100,
                hwnd, (HMENU)IDC_ENGINE_COMBO, GetModuleHandle(NULL), NULL);
            if (hFont) SendMessageW(g_hwndCmbEngine, WM_SETFONT, (WPARAM)hFont, TRUE);

            y += L.comboH + L.gapY;

            // Row 3: Buffer label
            g_hwndLblBuffer = CreateWindowExW(0, L"STATIC",
                SimpleUiText(L"UI_BUFFER_MODE", tmp),
                WS_CHILD | WS_VISIBLE, x, y, L.labelW, L.rowH,
                hwnd, (HMENU)IDC_LABEL_BUFFER, GetModuleHandle(NULL), NULL);
            if (hLabelFont) SendMessageW(g_hwndLblBuffer, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
            y += L.rowH;

            g_hwndCmbBuffer = CreateWindowExW(0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
                x, y, L.comboW, L.comboH + 120,
                hwnd, (HMENU)IDC_BUFFER_COMBO, GetModuleHandle(NULL), NULL);
            if (hFont) SendMessageW(g_hwndCmbBuffer, WM_SETFONT, (WPARAM)hFont, TRUE);

            RebuildCombosForScope();

            // Bottom: Volume row
            y = L.height - (L.rowH + L.volTrackH + L.gapY / 2);

            g_hwndLblVolume = CreateWindowExW(0, L"STATIC",
                SimpleUiText(L"UI_VOLUME_CTRL", tmp),
                WS_CHILD | WS_VISIBLE, x, y, L.labelW, L.rowH,
                hwnd, (HMENU)IDC_LABEL_VOLUME, GetModuleHandle(NULL), NULL);
            if (hLabelFont) SendMessageW(g_hwndLblVolume, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

            g_hwndIcon = CreateWindowExW(0, L"STATIC", NULL,
                WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY,
                x, y + L.rowH + 4, L.volIconW, L.volIconH,
                hwnd, (HMENU)IDC_VOLUMEICON, GetModuleHandle(NULL), NULL);

            g_hwndValue = CreateWindowExW(0, L"STATIC", L"100",
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                x + L.comboW - L.volNumberW, y + L.rowH + 1, L.volNumberW, L.volIconH + 6,
                hwnd, (HMENU)IDC_VOLUMETEXT, GetModuleHandle(NULL), NULL);
            if (hNumFont) SendMessageW(g_hwndValue, WM_SETFONT, (WPARAM)hNumFont, TRUE);

            {
                int trackX = x + L.volIconW + 12;
                int trackW = (x + L.comboW - L.volNumberW) - trackX - 8;
                int trackY = y + L.rowH + 4 - (L.volTrackH - L.volIconH) / 2;
                g_hwndSlider = CustomSlider::Create(GetModuleHandle(NULL), hwnd, trackX, trackY, trackW, L.volTrackH, IDC_VOLUMESLIDER);
                ApplyThemeToSlider(g_hwndSlider);
                CustomSlider::SetRange(g_hwndSlider, 0, 100);
            }

            // Initial sync
            SyncControlsFromTarget();
            return 0;
        }

        case WM_MEASUREITEM:
            MeasureComboItem((MEASUREITEMSTRUCT*)lParam);
            return TRUE;

        case WM_DRAWITEM: {
            const DRAWITEMSTRUCT* dis = (const DRAWITEMSTRUCT*)lParam;
            if (!dis) break;

            // existing owner-draw for app combo
            if (dis->CtlID == IDC_APP_COMBO) {
                DrawComboItem(dis);
                return TRUE;
            }

            // owner-draw for custom title buttons
            if (dis->CtlType == ODT_BUTTON && dis->CtlID == IDC_BTN_CLOSE) {

                HDC hdc = dis->hDC;
                RECT rc = dis->rcItem;

                // base colors
                COLORREF bg = RGB(66, 66, 66);
                COLORREF hover = RGB(90, 90, 90);
                COLORREF down = RGB(120, 120, 120);

                // pick state color
                COLORREF fill = bg;
                if (dis->itemState & ODS_SELECTED) fill = down;
                else if (dis->itemState & ODS_HOTLIGHT) fill = hover;

                HBRUSH hb = CreateSolidBrush(fill);
                FillRect(hdc, &rc, hb);
                DeleteObject(hb);

                // subtle frame
                HPEN pen = CreatePen(PS_SOLID, 1, RGB(142, 142, 142));
                HPEN old = (HPEN)SelectObject(hdc, pen);
                Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);
                SelectObject(hdc, old);
                DeleteObject(pen);

                // draw icon centered
                int ix = rc.left + (rc.right - rc.left - 16) / 2;
                int iy = rc.top + (rc.bottom - rc.top - 16) / 2;
                HICON hi = g_hIconClose;
                if (hi) DrawIconEx(hdc, ix, iy, hi, 16, 16, 0, NULL, DI_NORMAL);

                return TRUE;
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_PAINT: {
            PAINTSTRUCT ps; HDC dc = BeginPaint(hwnd, &ps);
            PaintBackground(hwnd, dc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        // empty area click -> close all dropdowns and blur focus
        case WM_LBUTTONDOWN: {
            POINT pt = {
                ((int)(short)((WORD)(((DWORD_PTR)(lParam)) & 0xffff))),
                ((int)(short)((WORD)((((DWORD_PTR)(lParam)) >> 16) & 0xffff)))
            };
            if (ShouldStartDragFrom(pt)) {
                ReleaseCapture();
                // Convert to screen coords for WM_NCLBUTTONDOWN
                POINT scr = pt; ClientToScreen(hwnd, &scr);
                POINT pt1; GetCursorPos(&pt1); // screen coords
                SendMessageW(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, MAKELPARAM(scr.x, scr.y));
                POINT pt2; GetCursorPos(&pt2);
                g_lastShowPt.x += pt2.x - pt1.x;
                g_lastShowPt.y += pt2.y - pt1.y;
                return 0;
            }
            CloseAllDropdownsAndBlurToPanel(hwnd);
            return 0;
        }
        case WM_RBUTTONDOWN:
            CloseAllDropdownsAndBlurToPanel(hwnd);
            return 0;

        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int v = CustomSlider::GetValue(g_hwndSlider);
            if (delta > 0) v += 2; else if (delta < 0) v -= 2;
            CustomSlider::SetValue(g_hwndSlider, v, TRUE);
            return 0;
        }

        case WM_KEYDOWN: {
            int v = CustomSlider::GetValue(g_hwndSlider);
            if (wParam == VK_LEFT || wParam == VK_DOWN) v -= 1;
            else if (wParam == VK_RIGHT || wParam == VK_UP) v += 1;
            else break;
            CustomSlider::SetValue(g_hwndSlider, v, TRUE);
            return 0;
        }

        case WM_COMMAND: {
            const int id = LOWORD(wParam);
            const int code = HIWORD(wParam);

            // title buttons: keep current UX as hide
            if (id == IDC_BTN_CLOSE && code == BN_CLICKED) {
                ShowWindow(hwnd, SW_HIDE);
                return 0;
            }

            if (id == IDC_APP_COMBO && code == CBN_DROPDOWN) {
                // Adjust the list to show at 8-rows now.
                SendMessageW(g_hwndCmbApp, CB_SETMINVISIBLE, 8, 0);
                return 0;
            }

            // speaker icon click -> toggle mute on target override
            if (id == IDC_VOLUMEICON && code == STN_CLICKED) {
                DWORD ov = ReadTargetOverride();
                BOOL m = RegistryManager::OV_GetMute(ov);
                ov = RegistryManager::OV_WithMute(ov, m ? FALSE : TRUE);
                WriteTargetOverride(ov);
                UpdateVolumeIcon(RegistryManager::OV_GetVolume(ov), RegistryManager::OV_GetMute(ov));
                return 0;
            }

            // App combo selection -> switch target; update app icon and controls
            if (id == IDC_APP_COMBO && code == CBN_SELCHANGE) {
                int sel = (int)SendMessageW(g_hwndCmbApp, CB_GETCURSEL, 0, 0);
                AppItem* ai = (AppItem*)SendMessageW(g_hwndCmbApp, CB_GETITEMDATA, sel, 0);
                if (ai) {
                    // Special tail handlers
                    if (ai->isSpecial) {
                        if (ai->guid == L"#SHOW_ALL_APPS#") {
                            TrayIcon::SetShowAllApps(TRUE);
                        }
                        else if (ai->guid == L"#PATCH_FOR_WINE#") {
                            // run Wine patch flow; do not change selection/target
                            WineCompat::SelectFile();

                            // restore panel after the dialog
                            if (IsWindow(g_hwndPanel)) {
                                VolumeSlider::Show(g_hwndPanel, g_lastShowPt);
                            }
                        }
                        return 0; // IMPORTANT: stop here
                    }
                    // Normal row: switch target
                    g_currentGuid = ai->guid;
                    RebuildCombosForScope();
                    SyncControlsFromTarget();
                    TrayIcon::RefreshForTarget(g_currentGuid);
                }
                return 0;
            }

            // Engine combo
            if (id == IDC_ENGINE_COMBO && code == CBN_SELCHANGE) {
                int sel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                const BOOL isVista = RegistryManager::IsVistaOrLater();
                const BOOL isGlobal = g_currentGuid.empty() ? TRUE : FALSE;

                int fieldEngine = 0; // default

                if (!isVista) {
                    // XP
                    if (isGlobal) {
                        // [ DS(Default)=idx0->field0, WaveOut=idx1->field3 ]
                        fieldEngine = (sel == 1) ? 3 : 0;
                    }
                    else {
                        // [ Follow=0->0, DS=1->1, WaveOut=2->3 ]
                        if (sel == 0) fieldEngine = 0;
                        else if (sel == 1) fieldEngine = 1;
                        else               fieldEngine = 3;
                    }
                }
                else {
                    // Vista+
                    if (isGlobal) {
                        // [ WASAPI(Default)=0->0, DS=1->1, WaveOut=2->3 ]
                        fieldEngine = (sel == 1) ? 1 : (sel == 2 ? 3 : 0);
                    }
                    else {
                        // [ Follow=0->0, WASAPI=1->2, DS=2->1, WaveOut=3->3 ]
                        if (sel == 0) fieldEngine = 0;
                        else if (sel == 1) fieldEngine = 2;
                        else if (sel == 2) fieldEngine = 1;
                        else               fieldEngine = 3;
                    }
                }

                DWORD ov = ReadTargetOverride();
                ov = RegistryManager::OV_WithEngine(ov, fieldEngine);
                WriteTargetOverride(ov);

                SendMessageW((HWND)lParam, CB_SHOWDROPDOWN, FALSE, 0);
                SetFocus(hwnd);
                return 0;
            }

            // Buffer combo
            if (id == IDC_BUFFER_COMBO && code == CBN_SELCHANGE) {
                int sel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                const BOOL isGlobal = g_currentGuid.empty() ? TRUE : FALSE;
                int fieldBuf = 0;

                if (isGlobal) {
                    fieldBuf = (sel == 0) ? 0 : (sel == 1 ? 2 : 3);
                }
                else {
                    fieldBuf = sel;
                }
                DWORD ov = ReadTargetOverride();
                ov = RegistryManager::OV_WithBuffer(ov, fieldBuf);
                WriteTargetOverride(ov);
                SendMessageW((HWND)lParam, CB_SHOWDROPDOWN, FALSE, 0);
                SetFocus(hwnd);
                return 0;
            }
            break;
        }

        case WM_CUST_SLIDER_CHANGED: {
            // write volume, also unmute if currently muted
            const int percent = (int)lParam;
            DWORD ov = ReadTargetOverride();
            ov = RegistryManager::OV_WithVolume(ov, percent);
            if (RegistryManager::OV_GetMute(ov)) {
                ov = RegistryManager::OV_WithMute(ov, FALSE);
            }
            WriteTargetOverride(ov);
            UpdateValueText(percent);
            UpdateVolumeIcon(percent, FALSE);
            return 0;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, RGB(230, 230, 230));
            SetBkMode(hdc, OPAQUE);
            static HBRUSH hbr = NULL;
            if (!hbr) hbr = CreateSolidBrush(RGB(66, 66, 66));
            SetBkColor(hdc, RGB(66, 66, 66));
            return (LRESULT)hbr;
        }

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                ShowWindow(hwnd, SW_HIDE);
            }
            return 0;

        case WM_DESTROY:
            // remove subclass on IDC_LABEL_APP if present
            HWND hLblApp = GetDlgItem(hwnd, IDC_LABEL_APP);
            if (IsWindow(hLblApp)) {
                RemoveWindowSubclass(hLblApp, LabelDragSubclassProc, kSubId_LabelDrag);
            }

            ClearAppItems();
            g_hwndPanel = g_hwndIcon = g_hwndValue = g_hwndSlider = NULL;
            g_hwndLblApp = g_hwndCmbApp = NULL;
            g_hwndLblVolume = NULL;
            g_hwndLblEngine = g_hwndCmbEngine = NULL;
            g_hwndLblBuffer = g_hwndCmbBuffer = NULL;
            g_hwndBtnClose = NULL;
            g_hIconClose = g_hIconMute = g_hIconLvl1 = g_hIconLvl2 = g_hIconLvl3 = NULL;
            if (g_hFontSpecial) { DeleteObject(g_hFontSpecial); g_hFontSpecial = NULL; }
            return 0;
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }

} // anon namespace

// Public API ---------------------------------------------------------------

namespace VolumeSlider {

    BOOL RegisterWindowClass(HINSTANCE hInstance) {
        if (!CustomSlider::RegisterSliderClass(hInstance)) return FALSE;

        WNDCLASSEXW wcex = { sizeof(wcex) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = PanelWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL; // owner draw
        wcex.lpszClassName = SLIDER_WINDOW_CLASS;
        return RegisterClassExW(&wcex) != 0;
    }

    HWND Create(HINSTANCE hInstance, HWND hParent) {
        if (g_hwndPanel) return g_hwndPanel;

        g_hwndPanel = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            SLIDER_WINDOW_CLASS,
            L"WinMM Stubs Volume Panel",
            WS_POPUP,
            0, 0, L.width, L.height,
            hParent, NULL, hInstance, NULL);

        return g_hwndPanel;
    }

    void Show(HWND hwndSlider, POINT pt) {
        if (!hwndSlider || !IsWindow(hwndSlider)) return;

        // refresh list whenever showing (captures newly live apps)
        PopulateAppCombo();
        SyncControlsFromTarget();

        RECT rc; GetWindowRect(hwndSlider, &rc);
        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;

        HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfoW(hMon, &mi);

        int x = pt.x - width / 2;
        int y = pt.y - height - 10;

        if (x < mi.rcWork.left) x = mi.rcWork.left;
        if (x + width > mi.rcWork.right) x = mi.rcWork.right - width;
        if (y < mi.rcWork.top) y = pt.y + 10;
        if (y + height > mi.rcWork.bottom) y = mi.rcWork.bottom - height;

        SetWindowPos(hwndSlider, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        SetForegroundWindow(hwndSlider);
        SetFocus(hwndSlider);

        g_lastShowPt = pt;
        TrayIcon::RefreshForTarget(g_currentGuid);
    }

    void UpdateDisplay(HWND hwndSlider) {
        UNREFERENCED_PARAMETER(hwndSlider);
        SyncControlsFromTarget();
    }

    void SetShowAllApps(BOOL enable) {
        g_showAllApps = enable ? TRUE : FALSE;

        if (IsWindow(g_hwndPanel)) {
            std::wstring prev = g_currentGuid;
            PopulateAppCombo();
            if (!prev.empty()) {
                int count = (int)SendMessageW(g_hwndCmbApp, CB_GETCOUNT, 0, 0);
                for (int i = 0; i < count; ++i) {
                    AppItem* ai = (AppItem*)SendMessageW(g_hwndCmbApp, CB_GETITEMDATA, i, 0);
                    if (ai && ai->guid == prev) {
                        SendMessageW(g_hwndCmbApp, CB_SETCURSEL, i, 0);
                        g_currentGuid = prev;
                        break;
                    }
                }
            }
            else {
                SendMessageW(g_hwndCmbApp, CB_SETCURSEL, 0, 0); // Global
                g_currentGuid.clear();
            }
            SyncControlsFromTarget();
        }
    }

    void SelectGlobal() {
        g_currentGuid.clear();
        if (IsWindow(g_hwndCmbApp)) {
            SendMessageW(g_hwndCmbApp, CB_SETCURSEL, 0, 0);
        }
        // Synchronization the UI/Tray
        SyncControlsFromTarget();
        TrayIcon::RefreshForTarget(g_currentGuid);
    }

    void EnsureSelectionLiveOrGlobal() {
        // If no selection, nothing to do
        if (g_currentGuid.empty()) return;

        // When "Show all apps" is enabled, allow selecting non-live apps.
        // Do not force back to Global in this mode.
        if (g_showAllApps) return;

        // In normal mode (live-only), fall back to Global if the app is not live.
        if (!RegistryManager::IsAppLive(g_currentGuid)) {
            if (IsWindow(g_hwndPanel)) {
                PopulateAppCombo(); // rebuild list without dead app
            }
            SelectGlobal();
        }
    }

    std::wstring GetCurrentGuid() {
        return g_currentGuid; // empty = Global
    }

    // allow external components to set the current target guid
    // - If panel window exists: rebuild app combo, select the guid if present,
    //   then sync all controls and refresh tray.
    // - If panel not created yet: just cache guid; first Show() will build UI.
    void SetCurrentGuid(const std::wstring& guid) {
        g_currentGuid = guid; // empty => Global

        // If UI not created yet, defer UI work
        if (!IsWindow(g_hwndPanel)) {
            return;
        }

        // Rebuild combo to ensure the item list is up to date
        PopulateAppCombo();

        // Try to select the requested guid in the combo
        int sel = -1;
        const int count = (int)SendMessageW(g_hwndCmbApp, CB_GETCOUNT, 0, 0);
        for (int i = 0; i < count; ++i) {
            AppItem* ai = (AppItem*)SendMessageW(g_hwndCmbApp, CB_GETITEMDATA, i, 0);
            if (ai && !ai->isSpecial && ai->guid == g_currentGuid) {
                sel = i;
                break;
            }
        }

        if (sel >= 0) {
            SendMessageW(g_hwndCmbApp, CB_SETCURSEL, sel, 0);
        }
        else {
            // fallback to Global
            g_currentGuid.clear();
            if (count > 0) SendMessageW(g_hwndCmbApp, CB_SETCURSEL, 0, 0);
        }

        // Rebuild engine/buffer combos for the new scope and sync all controls
        RebuildCombosForScope();
        SyncControlsFromTarget();

        // Notify tray to reflect the current target (caption/icon state etc.)
        TrayIcon::RefreshForTarget(g_currentGuid);
    }

} // namespace VolumeSlider
