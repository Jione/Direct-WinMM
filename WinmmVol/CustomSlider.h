#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef WM_CUST_SLIDER_CHANGED
#define WM_CUST_SLIDER_CHANGED (WM_APP + 201)
#endif

class CustomSlider {
public:
    static BOOL RegisterClass(HINSTANCE hInst);
    static HWND Create(HINSTANCE hInst, HWND hParent, int x, int y, int w, int h, int controlId);

    // API
    static void  SetRange(HWND hWnd, int minVal, int maxVal);
    static void  SetValue(HWND hWnd, int value, BOOL notify);
    static int   GetValue(HWND hWnd);

    // Theming helpers (runtime tweakable)
    struct Theme {
        COLORREF bg;        // window bg
        COLORREF trackBg;   // track background
        COLORREF trackFill; // track fill (left/progress)
        COLORREF thumb;     // thumb color
        int      trackH;    // track height (px)
        int      thumbR;    // thumb radius (px)
        int      paddingX;  // horizontal padding around track
    };
    static void  SetTheme(HWND hWnd, const Theme& t);

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
