#include "CustomSlider.h"
#include <algorithm>

struct SliderState {
    int minVal;
    int maxVal;
    int value;
    bool dragging;
    CustomSlider::Theme theme;
};

static const wchar_t* CLS_SLIDER = L"WinMMStubs_CustomSlider";

static SliderState* GetState(HWND hWnd) {
    return reinterpret_cast<SliderState*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
}

BOOL CustomSlider::RegisterSliderClass(HINSTANCE hInst) {
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = CustomSlider::WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // owner-draw
    wc.lpszClassName = CLS_SLIDER;
    return RegisterClassExW(&wc) != 0;
}

HWND CustomSlider::Create(HINSTANCE hInst, HWND hParent, int x, int y, int w, int h, int controlId) {
    HWND hWnd = CreateWindowExW(0, CLS_SLIDER, L"", WS_CHILD | WS_VISIBLE,
        x, y, w, h, hParent, (HMENU)(INT_PTR)controlId, hInst, NULL);
    if (!hWnd) return NULL;

    // allocate state
    SliderState* st = new SliderState();
    st->minVal = 0; st->maxVal = 100; st->value = 100; st->dragging = false;
    st->theme.bg = RGB(43, 43, 43);
    st->theme.trackBg = RGB(70, 70, 70);
    st->theme.trackFill = RGB(200, 200, 200);
    st->theme.thumb = RGB(240, 240, 240);
    st->theme.trackH = 4;
    st->theme.thumbR = 6;
    st->theme.paddingX = 6;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)st);
    return hWnd;
}

void CustomSlider::SetRange(HWND hWnd, int minVal, int maxVal) {
    if (SliderState* st = GetState(hWnd)) {
        st->minVal = minVal; st->maxVal = maxVal;
        if (st->value < minVal) st->value = minVal;
        if (st->value > maxVal) st->value = maxVal;
        InvalidateRect(hWnd, NULL, TRUE);
    }
}

void CustomSlider::SetValue(HWND hWnd, int value, BOOL notify) {
    if (SliderState* st = GetState(hWnd)) {
        int v = std::max(st->minVal, std::min(value, st->maxVal));
        if (v != st->value) {
            st->value = v;
            InvalidateRect(hWnd, NULL, FALSE);
            if (notify) {
                HWND parent = GetParent(hWnd);
                if (parent) PostMessage(parent, WM_CUST_SLIDER_CHANGED, (WPARAM)hWnd, (LPARAM)st->value);
            }
        }
    }
}

int CustomSlider::GetValue(HWND hWnd) {
    if (SliderState* st = GetState(hWnd)) return st->value;
    return 0;
}

void CustomSlider::SetTheme(HWND hWnd, const Theme& t) {
    if (SliderState* st = GetState(hWnd)) {
        st->theme = t;
        InvalidateRect(hWnd, NULL, TRUE);
    }
}

static int XToValue(const SliderState* st, RECT rc, int x) {
    int L = rc.left + st->theme.paddingX;
    int R = rc.right - st->theme.paddingX;
    if (x < L) x = L; if (x > R) x = R;
    double t = (double)(x - L) / (double)(R - L);
    return (int)(st->minVal + t * (st->maxVal - st->minVal) + 0.5);
}

LRESULT CALLBACK CustomSlider::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SliderState* st = GetState(hWnd);
    switch (msg) {
    case WM_ERASEBKGND:
        return 1; // paint everything
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC dc = BeginPaint(hWnd, &ps);
        RECT rc; GetClientRect(hWnd, &rc);

        // bg
        HBRUSH bg = CreateSolidBrush(st->theme.bg);
        FillRect(dc, &rc, bg); DeleteObject(bg);

        // track rect
        int cy = (rc.bottom - rc.top) / 2;
        RECT trc;
        trc.left = rc.left + st->theme.paddingX;
        trc.right = rc.right - st->theme.paddingX;
        trc.top = cy - st->theme.trackH / 2;
        trc.bottom = trc.top + st->theme.trackH;

        // track bg
        HBRUSH tbg = CreateSolidBrush(st->theme.trackBg);
        FillRect(dc, &trc, tbg); DeleteObject(tbg);

        // fill width
        int range = (st->maxVal - st->minVal);
        int px = trc.left;
        if (range > 0) {
            double ratio = (double)(st->value - st->minVal) / (double)range;
            px = trc.left + (int)((trc.right - trc.left) * ratio + 0.5);
        }
        RECT frc = trc; frc.right = px;
        HBRUSH tfill = CreateSolidBrush(st->theme.trackFill);
        FillRect(dc, &frc, tfill); DeleteObject(tfill);

        // thumb
        int r = st->theme.thumbR;
        int cx = px;
        int cyThumb = cy;
        HBRUSH th = CreateSolidBrush(st->theme.thumb);
        HBRUSH old = (HBRUSH)SelectObject(dc, th);
        HPEN pen = CreatePen(PS_SOLID, 1, RGB(20, 20, 20));
        HPEN oldp = (HPEN)SelectObject(dc, pen);
        Ellipse(dc, cx - r, cyThumb - r, cx + r + 1, cyThumb + r + 1);
        SelectObject(dc, oldp); DeleteObject(pen);
        SelectObject(dc, old); DeleteObject(th);

        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        SetCapture(hWnd);
        st->dragging = true;
        RECT rc; GetClientRect(hWnd, &rc);
        int x = ((int)(short)LOWORD(lParam));
        SetValue(hWnd, XToValue(st, rc, x), TRUE);
        return 0;
    }
    case WM_MOUSEMOVE:
        if (st->dragging) {
            RECT rc; GetClientRect(hWnd, &rc);
            int x = ((int)(short)LOWORD(lParam));
            SetValue(hWnd, XToValue(st, rc, x), TRUE);
        }
        return 0;
    case WM_LBUTTONUP:
        if (st->dragging) {
            ReleaseCapture();
            st->dragging = false;
        }
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_LEFT || wParam == VK_DOWN) { SetValue(hWnd, st->value - 1, TRUE); return 0; }
        if (wParam == VK_RIGHT || wParam == VK_UP) { SetValue(hWnd, st->value + 1, TRUE); return 0; }
        return 0;
    case WM_DESTROY: {
        SliderState* s = GetState(hWnd);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
        delete s;
        return 0;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
