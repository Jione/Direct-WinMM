#pragma once
#include <windows.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

// px ������ ��Ʈ�� ����� ��Ʈ�ѿ� ����
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
    lf.lfHeight = -MulDiv(px, dpi, 96);        // px �� ������ ��ȯ
    lf.lfWeight = FW_NORMAL;
    lstrcpyW(lf.lfFaceName, L"Segoe UI");      // XP�� Tahoma�� �ڵ� �����
    return CreateFontIndirectW(&lf);
}

// ����(�Ǵ� üũ�ڽ�) ��Ʈ���� ���� ũ�� ��� + ����
inline void ResizeRadioToFont(HWND hRadio, int fontPx, int extraPaddingX = 6, int extraPaddingY = 4) {
    HWND parent = GetParent(hRadio);
    HFONT hFont = MakeUIFontForPixels(parent ? parent : hRadio, fontPx);
    SendMessageW(hRadio, WM_SETFONT, (WPARAM)hFont, TRUE); // hFont�� ��Ʈ���� ����

    // �ؽ�Ʈ �� ����
    wchar_t text[256]; GetWindowTextW(hRadio, text, 256);
    HDC dc = GetDC(hRadio);
    HFONT old = (HFONT)SelectObject(dc, hFont);
    SIZE sz{}; GetTextExtentPoint32W(dc, text, lstrlenW(text), &sz);
    SelectObject(dc, old); ReleaseDC(hRadio, dc);

    // �ε�������(����)�� ũ��
    int indW = GetSystemMetrics(SM_CXMENUCHECK);
    int indH = GetSystemMetrics(SM_CYMENUCHECK);

    // �׸��� ������ ���� ��Ʈ ũ�� ��� (���� �� ��Ȯ)
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

    // ���� ũ�� ���: [������] [����] [�ؽ�Ʈ] + ���� �е�
    RECT rc{}; GetWindowRect(hRadio, &rc);
    MapWindowPoints(NULL, parent, (LPPOINT)&rc, 2);

    int gap = 6;
    int width = partW + gap + sz.cx + extraPaddingX * 2;
    int height = max(partH, sz.cy) + extraPaddingY * 2;

    SetWindowPos(hRadio, NULL, rc.left, rc.top, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}