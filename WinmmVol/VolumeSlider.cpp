#include "VolumeSlider.h"
#include "RegistryManager.h"
#include "resource.h"
#include "CustomSlider.h"
#include "TrayIcon.h"
#include "UiLang.h"
#include <commctrl.h>
#include <shlwapi.h>
#include <shellapi.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Version.lib")

namespace {

    static const wchar_t* SLIDER_WINDOW_CLASS = L"WinMMStubsVolumePanel";

    // Window + child handles
    static HWND g_hwndPanel = NULL;
    static HWND g_hwndIcon = NULL; // speaker icon
    static HWND g_hwndValue = NULL;
    static HWND g_hwndSlider = NULL; // CustomSlider
    static HWND g_hwndLblApp = NULL;
    static HWND g_hwndCmbApp = NULL;
    static HWND g_hwndLblVolume = NULL;
    static HWND g_hwndLblEngine = NULL;
    static HWND g_hwndCmbEngine = NULL;
    static HWND g_hwndLblBuffer = NULL;
    static HWND g_hwndCmbBuffer = NULL;

    // Icons
    static HICON g_hIconMute = NULL;
    static HICON g_hIconLvl1 = NULL;
    static HICON g_hIconLvl2 = NULL;
    static HICON g_hIconLvl3 = NULL;

    static BOOL g_isMuted = FALSE;
    static BOOL g_showAllApps = FALSE;

    // current target (empty = Global, otherwise GUID)
    static std::wstring g_currentGuid;
    static DWORD g_curOverride = RegistryManager::OV_DEFAULT; // cache of current target's override

    // App combo item payload
    struct AppItem {
        std::wstring guid;       // empty for Global
        std::wstring exePath;    // for app rows
        std::wstring display;    // caption(filename)
        HICON        hIcon;      // per-row small icon
        AppItem() : hIcon(NULL) {}
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
        int labelW = 220;  // label width
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
        if (refreshTray) TrayIcon::RefreshFromRegistry();
    }

    // sync UI controls (engine/buffer/slider/mute) from current target override
    static void SyncControlsFromTarget() {
        g_curOverride = ReadTargetOverride();

        // Engine
        int engine = RegistryManager::OV_GetEngine(g_curOverride);
        // UI order: Auto(0), WASAPI(1), DirectSound(2)
#if !defined(_DEBUG)
        if (!RegistryManager::IsVistaOrLater() && engine == 2) {
            // XP display as DS when registry says WASAPI (effective fallback)
            engine = 1; // map to DS in UI
        }
#endif
        int engineSel = 0;
        if (engine == 2) engineSel = 1;  // WASAPI
        else if (engine == 1) engineSel = 2; // DS
        SendMessageW(g_hwndCmbEngine, CB_SETCURSEL, engineSel, 0);

        // Buffer (0..3) order same as UI items
        int buf = RegistryManager::OV_GetBuffer(g_curOverride);
        SendMessageW(g_hwndCmbBuffer, CB_SETCURSEL, buf, 0);

        // Volume / Mute
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
            SendMessageW(g_hwndCmbApp, CB_SETCURSEL, idx, 0);
            g_currentGuid.clear();
        }

        // live apps
        std::vector<RegistryManager::AppEntry> apps;
        RegistryManager::EnumApps(apps, g_showAllApps ? FALSE : TRUE);
        for (auto& e : apps) {
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

        // use system colors so it matches other standard combos
        const BOOL isEditField = (dis->itemID == (UINT)-1);
        COLORREF bg = isEditField
            ? GetSysColor(COLOR_WINDOW)
            : ((dis->itemState & ODS_SELECTED) ? GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_WINDOW));
        COLORREF fg = isEditField
            ? GetSysColor(COLOR_WINDOWTEXT)
            : ((dis->itemState & ODS_SELECTED) ? GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_WINDOWTEXT));

        HBRUSH hb = CreateSolidBrush(bg);
        FillRect(hdc, &rc, hb);
        DeleteObject(hb);

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

        // layout
        int x = rc.left + 4;
        int y = rc.top + ((rc.bottom - rc.top) - L.appIconH) / 2;

        if (ai && ai->hIcon) {
            DrawIconEx(hdc, x, y, ai->hIcon, L.appIconW, L.appIconH, 0, NULL, DI_NORMAL);
        }
        x += L.appIconW + L.appIconGap;

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
    }

    static void MeasureComboItem(MEASUREITEMSTRUCT* mis) {
        if (mis->CtlID != IDC_APP_COMBO) return;
        mis->itemHeight = (L.comboH > (L.appIconH + 4)) ? L.comboH : (L.appIconH + 4);
    }

    // Window procedure --------------------------------------------------------

    LRESULT CALLBACK PanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            g_hwndPanel = hwnd;
            HFONT hFont = CreateUIFont(-13, FW_MEDIUM);
            HFONT hLabelFont = CreateUIFont(-15, FW_BOLD);
            HFONT hNumFont = CreateUIFont(-18, FW_BOLD);

            // load speaker icons
            {
                HINSTANCE hInst = GetModuleHandleW(NULL);
                g_hIconMute = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_MUTE), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl1 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL1), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl2 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL2), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
                g_hIconLvl3 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL3), IMAGE_ICON, L.volIconW, L.volIconH, LR_DEFAULTCOLOR);
            }

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
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS,
                x, y, L.comboW, L.comboH + 240,
                hwnd, (HMENU)IDC_APP_COMBO, GetModuleHandle(NULL), NULL);
            if (hFont) SendMessageW(g_hwndCmbApp, WM_SETFONT, (WPARAM)hFont, TRUE);

            PopulateAppCombo();

            y += L.comboH + L.gapY;

            // Row 2: Engine label + combo
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

            // Order: Auto, WASAPI, DirectSound (UI)
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_ENGINE_AUTO", tmp));
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_ENGINE_WASAPI", tmp));
            SendMessageW(g_hwndCmbEngine, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_ENGINE_DS", tmp));

#if !defined(_DEBUG)
            // XP gating: disable Engine combo on XP (Vista+ only)
            if (!RegistryManager::IsVistaOrLater()) {
                EnableWindow(g_hwndCmbEngine, FALSE);
            }
#endif
            y += L.comboH + L.gapY;

            // Row 3: Buffer label + combo
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

            // Buffer items: Auto, Streaming, Full, Resample
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_BUFFER_AUTO", tmp));
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_BUFFER_STREAMING", tmp));
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_BUFFER_FULL", tmp));
            SendMessageW(g_hwndCmbBuffer, CB_ADDSTRING, 0, (LPARAM)SimpleUiText(L"UI_BUFFER_RESAMPLE", tmp));

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
            if (dis && dis->CtlID == IDC_APP_COMBO) {
                DrawComboItem(dis);
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
        case WM_LBUTTONDOWN:
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
                    g_currentGuid = ai->guid; // empty if Global
                    SyncControlsFromTarget();
                }
                return 0;
            }

            // Engine combo -> set field in target override; close dropdown & blur
            if (id == IDC_ENGINE_COMBO && code == CBN_SELCHANGE) {
                if (IsWindowEnabled(g_hwndCmbEngine)) {
                    int sel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                    // UI order: Auto(0), WASAPI(1), DirectSound(2) -> engine: 0/2/1
                    int engine = 0;
                    if (sel == 1) engine = 2;       // WASAPI
                    else if (sel == 2) engine = 1;  // DirectSound
                    DWORD ov = ReadTargetOverride();
                    ov = RegistryManager::OV_WithEngine(ov, engine);
                    WriteTargetOverride(ov);
                }
                SendMessageW((HWND)lParam, CB_SHOWDROPDOWN, FALSE, 0);
                SetFocus(hwnd);
                return 0;
            }

            // Buffer combo -> set field; close dropdown & blur
            if (id == IDC_BUFFER_COMBO && code == CBN_SELCHANGE) {
                int sel = (int)SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
                DWORD ov = ReadTargetOverride();
                ov = RegistryManager::OV_WithBuffer(ov, sel); // UI order == field value
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
            ClearAppItems();
            g_hwndPanel = g_hwndIcon = g_hwndValue = g_hwndSlider = NULL;
            g_hwndLblApp = g_hwndCmbApp = NULL;
            g_hwndLblVolume = NULL;
            g_hwndLblEngine = g_hwndCmbEngine = NULL;
            g_hwndLblBuffer = g_hwndCmbBuffer = NULL;
            g_hIconMute = g_hIconLvl1 = g_hIconLvl2 = g_hIconLvl3 = NULL;
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
    }

    void UpdateDisplay(HWND hwndSlider) {
        UNREFERENCED_PARAMETER(hwndSlider);
        SyncControlsFromTarget();
    }

    void VolumeSlider::SetShowAllApps(BOOL enable) {
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

} // namespace VolumeSlider
