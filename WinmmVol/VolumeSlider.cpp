#include "VolumeSlider.h"
#include "RegistryManager.h"
#include "resource.h"
#include "CustomSlider.h"
#include <commctrl.h>

namespace {
    static const wchar_t* SLIDER_WINDOW_CLASS = L"WinMMStubsVolumeSliderPopup";
    static HWND g_hwndSlider = NULL;
    static HWND g_hwndIcon = NULL;
    static HWND g_hwndValue = NULL;
    static HWND g_hwndCtrl = NULL; // CustomSlider
    static BOOL g_dragging = false;
    static HWND g_hwndTitle = NULL;

    static HICON g_hIconMute = NULL;
    static HICON g_hIconLvl1 = NULL;
    static HICON g_hIconLvl2 = NULL;
    static HICON g_hIconLvl3 = NULL;
    static BOOL g_isMuted = FALSE;

    // Layout constants (edit in one place)
    struct Layout {
        int width = 260;
        int height = 90;
        int topPad = 10;
        int titleH = 20;
        int controlAreaY = 30;
        int padL = 10;
        int iconY = controlAreaY + 21;  // Icon top
        int iconW = 24;
        int iconH = 18;
        int valW = 32;  // Right-side number width
        int valH = 18;
        int ctrlH = 28;  // Slider control height
        int gap = 8;   // Gap between icon-slider-number
    } L;

    static HFONT CreateUiFont() {
        LOGFONTW lf = { 0 };
        lf.lfHeight = -14; // Approx 9pt
        lf.lfWeight = FW_SEMIBOLD;
        lstrcpyW(lf.lfFaceName, L"Segoe UI");
        return CreateFontIndirectW(&lf);
    }

    static void PaintBackground(HWND hwnd, HDC hdc) {
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH bg = CreateSolidBrush(RGB(66, 66, 66));
        FillRect(hdc, &rc, bg);
        DeleteObject(bg);
        // Border (light line)
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(142, 142, 142));
        HPEN old = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, rc.left, rc.top, NULL);
        LineTo(hdc, rc.right - 1, rc.top);
        LineTo(hdc, rc.right - 1, rc.bottom - 1);
        LineTo(hdc, rc.left, rc.bottom - 1);
        LineTo(hdc, rc.left, rc.top);
        SelectObject(hdc, old); DeleteObject(pen);
    }

    static void UpdateValueText(int percent) {
        if (!g_hwndValue) return;
        wchar_t buf[8]; wsprintfW(buf, L"%d", percent);
        SetWindowTextW(g_hwndValue, buf);
    }

    static void ApplyThemeToCustomSlider(HWND hCtrl) {
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

    // Replaced UpdateMuteIcon with UpdateVolumeIcon
    static void UpdateVolumeIcon(int percent, BOOL isMuted) {
        if (!g_hwndIcon) return;

        HICON hIcon = NULL;
        if (isMuted || percent == 0) {
            hIcon = g_hIconMute;
        }
        else if (percent <= 32) {
            hIcon = g_hIconLvl1;
        }
        else if (percent <= 65) {
            hIcon = g_hIconLvl2;
        }
        else { // 66-100%
            hIcon = g_hIconLvl3;
        }

        SendMessageW(g_hwndIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
        g_isMuted = isMuted; // Store current mute state
    }

    LRESULT CALLBACK SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateUiFont();

            // Title
            g_hwndTitle = CreateWindowExW(0, L"STATIC", L"CD-Audio Volume Control",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                L.padL, L.topPad, L.width - (2 * L.padL), L.titleH,
                hwnd, (HMENU)IDC_VOLUMETITLE, GetModuleHandle(NULL), NULL);
            if (g_hwndTitle && hFont) SendMessage(g_hwndTitle, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Icon
            g_hwndIcon = CreateWindowExW(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY,
                L.padL, L.iconY, L.iconW, L.iconH, hwnd, (HMENU)IDC_VOLUMEICON, GetModuleHandle(NULL), NULL);
            // Load all 4 icons
            if (g_hwndIcon) {
                HINSTANCE hInst = GetModuleHandle(NULL);
                g_hIconMute = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_MUTE), IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
                g_hIconLvl1 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL1), IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
                g_hIconLvl2 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL2), IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
                g_hIconLvl3 = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_SPEAKER_LVL3), IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
            }

            // Right-side number
            int valX = L.width - L.padL - L.valW;
            int valY = L.iconY;
            g_hwndValue = CreateWindowExW(0, L"STATIC", L"100",
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                valX, valY, L.valW, L.valH, hwnd, (HMENU)IDC_VOLUMETEXT, GetModuleHandle(NULL), NULL);
            if (g_hwndValue && hFont) SendMessage(g_hwndValue, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Center custom slider
            int ctrlX = L.padL + L.iconW + L.gap;
            int ctrlW = (L.width - L.padL - L.valW - L.gap) - ctrlX;
            int ctrlY = L.controlAreaY + (L.height - L.controlAreaY - L.ctrlH) / 2;
            g_hwndCtrl = CustomSlider::Create(GetModuleHandle(NULL), hwnd, ctrlX, ctrlY, ctrlW, L.ctrlH, IDC_VOLUMESLIDER);
            ApplyThemeToCustomSlider(g_hwndCtrl);
            CustomSlider::SetRange(g_hwndCtrl, 0, 100);

            // Reflect initial value
            int percent = RegistryManager::GetVolumePercent();
            BOOL isMuted = RegistryManager::GetMute();
            CustomSlider::SetValue(g_hwndCtrl, percent, FALSE);
            UpdateValueText(percent);
            UpdateVolumeIcon(percent, isMuted);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            int currentPercent = CustomSlider::GetValue(g_hwndCtrl);
            int newPercent = currentPercent;

            if (delta > 0) {
                newPercent += 2; // Scroll up, increase volume
            }
            else if (delta < 0) {
                newPercent -= 2; // Scroll down, decrease volume
            }

            CustomSlider::SetValue(g_hwndCtrl, newPercent, TRUE); // This will trigger WM_CUST_SLIDER_CHANGED
            return 0;
        }
        case WM_KEYDOWN: {
            int currentPercent = CustomSlider::GetValue(g_hwndCtrl);
            int newPercent = currentPercent;

            if (wParam == VK_LEFT || wParam == VK_DOWN) { newPercent -= 1; }
            else if (wParam == VK_RIGHT || wParam == VK_UP) { newPercent += 1; }
            else { break; }

            CustomSlider::SetValue(g_hwndCtrl, newPercent, TRUE); // This will trigger WM_CUST_SLIDER_CHANGED
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_VOLUMEICON && HIWORD(wParam) == STN_CLICKED) {
                BOOL newMuteState = !g_isMuted;
                RegistryManager::SetMute(newMuteState);

                int percent = CustomSlider::GetValue(g_hwndCtrl);
                UpdateVolumeIcon(percent, newMuteState);
                return 0;
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
        case WM_CUST_SLIDER_CHANGED: {
            int percent = (int)lParam;
            RegistryManager::SetVolumePercent(percent);
            UpdateValueText(percent);

            if (g_isMuted) {
                RegistryManager::SetMute(FALSE);
                g_isMuted = FALSE;
            }

            // Update icon based on percentage change
            UpdateVolumeIcon(percent, g_isMuted);
            return 0;
        }
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE) {
                ShowWindow(hwnd, SW_HIDE);
            }
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            // Text color
            SetTextColor(hdc, RGB(230, 230, 230));

            // Set background to dark gray
            SetBkMode(hdc, OPAQUE);
            COLORREF bg = RGB(66, 66, 66);       // Same value as PaintBackground
            SetBkColor(hdc, bg);

            // Return a brush of the same color (paints the static control background with this color)
            static HBRUSH hbrBg = NULL;
            if (!hbrBg) hbrBg = CreateSolidBrush(bg);
            return (LRESULT)hbrBg;
        }
        case WM_DESTROY:
            g_hwndSlider = g_hwndIcon = g_hwndValue = g_hwndCtrl = g_hwndTitle = NULL;
            g_hIconMute = g_hIconLvl1 = g_hIconLvl2 = g_hIconLvl3 = NULL;
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
} // namespace


namespace VolumeSlider {
    BOOL RegisterWindowClass(HINSTANCE hInstance) {
        // Register custom slider class first
        if (!CustomSlider::RegisterSliderClass(hInstance)) return FALSE;

        WNDCLASSEXW wcex = { sizeof(wcex) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = SliderWndProc;
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
        wcex.hbrBackground = NULL; // owner-draw
        wcex.lpszClassName = SLIDER_WINDOW_CLASS;
        return RegisterClassExW(&wcex) != 0;
    }

    HWND Create(HINSTANCE hInstance, HWND hParent) {
        if (g_hwndSlider) return g_hwndSlider;

        g_hwndSlider = CreateWindowExW(
            WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
            SLIDER_WINDOW_CLASS,
            L"WinMM Stubs Volume Control",
            WS_POPUP,
            0, 0, L.width, L.height,
            hParent, NULL, hInstance, NULL);

        return g_hwndSlider;
    }

    void Show(HWND hwndSlider, POINT pt) {
        if (!hwndSlider || !IsWindow(hwndSlider)) return;

        UpdateDisplay(hwndSlider);

        RECT rc; GetWindowRect(hwndSlider, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

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
        if (!hwndSlider || !IsWindow(hwndSlider)) return;
        if (!g_hwndCtrl || !g_hwndValue || !g_hwndIcon || !g_hwndTitle) return;

        const int percent = RegistryManager::GetVolumePercent();
        const BOOL isMuted = RegistryManager::GetMute();

        CustomSlider::SetValue(g_hwndCtrl, percent, FALSE);
        UpdateValueText(percent);
        UpdateVolumeIcon(percent, isMuted);
    }
}
