#pragma once
#include <windows.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

// px 높이의 폰트를 만들고 컨트롤에 적용
inline HFONT MakeUIFontForPixels(HWND hRefWnd, int px) {
    int dpi = 96;
#if defined(_WIN64) || _WIN32_WINNT >= 0x0603
    if (HMONITOR mon = MonitorFromWindow(hRefWnd, MONITOR_DEFAULTTONEAREST)) {
        // Win8.1+: Per-monitor DPI
        UINT dpiX = 96, dpiY = 96;
        typedef HRESULT(WINAPI* GetDpiForMonitor_t)(HMONITOR, int, UINT*, UINT*);
        static auto pGetDpiForMonitor = (GetDpiForMonitor_t)
            GetProcAddress(GetModuleHandleW(L"Shcore.dll"), "GetDpiForMonitor");
        if (pGetDpiForMonitor) {
            pGetDpiForMonitor(mon, 0, &dpiX, &dpiY);
            dpi = (int)dpiY;
        }
        else {
            HDC dc = GetDC(hRefWnd); dpi = GetDeviceCaps(dc, LOGPIXELSY); ReleaseDC(hRefWnd, dc);
        }
    }
#else
    HDC dc = GetDC(hRefWnd); dpi = GetDeviceCaps(dc, LOGPIXELSY); ReleaseDC(hRefWnd, dc);
#endif

    LOGFONTW lf{};
    lf.lfHeight = -MulDiv(px, dpi, 96);        // px → 논리단위 변환
    lf.lfWeight = FW_NORMAL;
    lstrcpyW(lf.lfFaceName, L"Segoe UI");      // XP면 Tahoma로 자동 폴백됨
    return CreateFontIndirectW(&lf);
}

// 라디오(또는 체크박스) 컨트롤의 권장 크기 계산 + 적용
inline void ResizeRadioToFont(HWND hRadio, int fontPx, int extraPaddingX = 6, int extraPaddingY = 4) {
    HWND parent = GetParent(hRadio);
    HFONT hFont = MakeUIFontForPixels(parent ? parent : hRadio, fontPx);
    SendMessageW(hRadio, WM_SETFONT, (WPARAM)hFont, TRUE); // hFont는 컨트롤이 소유

    // 텍스트 폭 측정
    wchar_t text[256]; GetWindowTextW(hRadio, text, 256);
    HDC dc = GetDC(hRadio);
    HFONT old = (HFONT)SelectObject(dc, hFont);
    SIZE sz{}; GetTextExtentPoint32W(dc, text, lstrlenW(text), &sz);
    SelectObject(dc, old); ReleaseDC(hRadio, dc);

    // 인디케이터(원형)의 크기
    int indW = GetSystemMetrics(SM_CXMENUCHECK);
    int indH = GetSystemMetrics(SM_CYMENUCHECK);

    // 테마가 있으면 실제 파트 크기 사용 (조금 더 정확)
    int partW = indW, partH = indH;
    HTHEME th = OpenThemeData(hRadio, L"BUTTON");
    if (th) {
        SIZE ideal{};
        // BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL = 1
        if (SUCCEEDED(GetThemePartSize(th, NULL, BP_RADIOBUTTON, RBS_UNCHECKEDNORMAL, NULL, TS_TRUE, &ideal))) {
            partW = ideal.cx; partH = ideal.cy;
        }
        CloseThemeData(th);
    }

    // 최종 크기 계산: [아이콘] [간격] [텍스트] + 여유 패딩
    RECT rc{}; GetWindowRect(hRadio, &rc);
    MapWindowPoints(NULL, parent, (LPPOINT)&rc, 2);

    int gap = 6;
    int width = partW + gap + sz.cx + extraPaddingX * 2;
    int height = max(partH, sz.cy) + extraPaddingY * 2;

    SetWindowPos(hRadio, NULL, rc.left, rc.top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}