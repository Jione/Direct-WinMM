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

    static HICON g_hIconSpeaker = NULL;
    static HICON g_hIconSpeakerMute = NULL;
    static BOOL g_isMuted = FALSE;

    // Layout constants (edit in one place)
    struct Layout {
        int width = 260;
        int height = 60;
        int padL = 10;
        int iconY = 21;  // Icon top
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

    static void UpdateMuteIcon(BOOL isMuted) {
        if (!g_hwndIcon) return;
        HICON hIcon = isMuted ? g_hIconSpeakerMute : g_hIconSpeaker;
        SendMessageW(g_hwndIcon, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
        g_isMuted = isMuted;
    }

    LRESULT CALLBACK SliderWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_CREATE: {
            HFONT hFont = CreateUiFont();

            // Icon
            g_hwndIcon = CreateWindowExW(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_ICON | SS_NOTIFY,
                L.padL, L.iconY, L.iconW, L.iconH, hwnd, (HMENU)IDC_VOLUMEICON, GetModuleHandle(NULL), NULL);
            if (g_hwndIcon) {
                g_hIconSpeaker = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_VOLUME_SPEAKER),
                    IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
                g_hIconSpeakerMute = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_VOLUME_SPEAKER_MUTE),
                    IMAGE_ICON, L.iconW, L.iconH, LR_DEFAULTCOLOR);
            }

            // Right-side number
            int valX = L.width - L.padL - L.valW;
            int valY = (L.height - L.valH) / 2;
            g_hwndValue = CreateWindowExW(0, L"STATIC", L"100",
                WS_CHILD | WS_VISIBLE | SS_RIGHT,
                valX, valY, L.valW, L.valH, hwnd, (HMENU)IDC_VOLUMETEXT, GetModuleHandle(NULL), NULL);
            if (g_hwndValue && hFont) SendMessage(g_hwndValue, WM_SETFONT, (WPARAM)hFont, TRUE);

            // Center custom slider
            int ctrlX = L.padL + L.iconW + L.gap;
            int ctrlW = (L.width - L.padL - L.valW - L.gap) - ctrlX;
            g_hwndCtrl = CustomSlider::Create(GetModuleHandle(NULL), hwnd, ctrlX, (L.height - L.ctrlH) / 2, ctrlW, L.ctrlH, IDC_VOLUMESLIDER);
            ApplyThemeToCustomSlider(g_hwndCtrl);
            CustomSlider::SetRange(g_hwndCtrl, 0, 100);

            // Reflect initial value
            int percent = RegistryManager::GetVolumePercent();
            CustomSlider::SetValue(g_hwndCtrl, percent, FALSE);
            UpdateValueText(percent);
            UpdateMuteIcon(RegistryManager::GetMute());
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

            CustomSlider::SetValue(g_hwndCtrl, newPercent, TRUE);
            return 0;
        }
        case WM_KEYDOWN: {
            int currentPercent = CustomSlider::GetValue(g_hwndCtrl);
            int newPercent = currentPercent;

            if (wParam == VK_LEFT || wParam == VK_DOWN) { newPercent -= 1; }
            else if (wParam == VK_RIGHT || wParam == VK_UP) { newPercent += 1; }
            else { break; }

            CustomSlider::SetValue(g_hwndCtrl, newPercent, TRUE);
            return 0;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_VOLUMEICON && HIWORD(wParam) == STN_CLICKED) {
                BOOL newMuteState = !g_isMuted;
                RegistryManager::SetMute(newMuteState);
                UpdateMuteIcon(newMuteState);
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
            g_hwndSlider = g_hwndIcon = g_hwndValue = g_hwndCtrl = NULL;
            g_hIconSpeakerMute = g_hIconSpeaker = NULL;
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
} // namespace


namespace VolumeSlider {
    BOOL RegisterWindowClass(HINSTANCE hInstance) {
        // Register custom slider class first
        if (!CustomSlider::RegisterClass(hInstance)) return FALSE;

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
            0, 0, 260, 60,
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
        if (!g_hwndCtrl || !g_hwndValue) return;

        const int percent = RegistryManager::GetVolumePercent();
        CustomSlider::SetValue(g_hwndCtrl, percent, FALSE);
        UpdateValueText(percent);
        UpdateMuteIcon(RegistryManager::GetMute());
    }
}
