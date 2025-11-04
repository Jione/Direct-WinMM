#include "AboutDialog.h"
#include "resource.h"
#include "UiTheme.h"
#include "UiLang.h"

#include <commctrl.h>
#include <uxtheme.h>   // SetWindowTheme (modern scrollbar on Vista+)
#include <string>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "UxTheme.lib")

// Single edit control showing Usage + 2 blank lines + License.
namespace {
    static HWND   s_hDlg = NULL;
    static HWND   s_hEdit = NULL;
    static HFONT  s_hFont = NULL;
    static HBRUSH s_hBg = NULL;
    static BOOL   s_isOpen = FALSE;

    // Load UTF-16LE RCDATA to std::wstring (skip BOM if present).
    static bool LoadUtf16RsrcToString(HINSTANCE hInst, UINT id, std::wstring& out) {
        out.clear();
        HRSRC hRes = FindResourceW(hInst, MAKEINTRESOURCEW(id), RT_RCDATA);
        if (!hRes) return false;
        HGLOBAL hData = LoadResource(hInst, hRes);
        if (!hData) return false;
        const wchar_t* w = (const wchar_t*)LockResource(hData);
        DWORD sz = SizeofResource(hInst, hRes);
        if (!w || sz < sizeof(wchar_t)) return false;
        size_t n = sz / sizeof(wchar_t);
        size_t i = 0;
        if (n && w[0] == 0xFEFF) { ++i; }
        out.assign(w + i, w + n);
        return true;
    }

    // Choose usage id by UI language.
    static UINT SelectUsageId() {
        LANGID lid = GetUserDefaultUILanguage();
        switch (PRIMARYLANGID(lid)) {
        case LANG_KOREAN:
            return IDR_USAGE_KO;
        default:
            return IDR_USAGE_EN;
        }
    }

    // Build final text: Usage + "\r\n\r\n" + License.
    static void BuildAboutText(HINSTANCE hInst, std::wstring& out) {
        std::wstring usage, lic;
        LoadUtf16RsrcToString(hInst, SelectUsageId(), usage);

        // Prefer single-file license. If not found, concatenate fallbacks.
        static const UINT kParts[] = {
            IDR_LICENSE_LIBOGG, IDR_LICENSE_LIBVORBIS, IDR_LICENSE_LIBFLAC, IDR_LICENSE_MINIMP3
        };
        std::wstring tmp;
        lic.append(L"\r\n================================================\r\nOpen Source License Information\r\n================================================\r\n");
        for (UINT id : kParts) {
            if (LoadUtf16RsrcToString(hInst, id, tmp)) {
                if (!lic.empty()) lic.append(L"\r\n\r\n");
                lic.append(tmp);
            }
        }

        out.clear();
        if (!usage.empty()) out.append(usage);
        if (!usage.empty() && !lic.empty()) out.append(L"\r\n\r\n"); // 2-line gap
        if (!lic.empty())   out.append(lic);
    }

    static void CreateEditAndFill(HWND hDlg) {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hDlg, GWLP_HINSTANCE);

        // Theme objects
        if (!s_hFont) s_hFont = UiTheme::CreateUIFont(FALSE);
        if (!s_hBg)   s_hBg = CreateSolidBrush(UiTheme::Bg());

        // Client rect
        RECT rc; GetClientRect(hDlg, &rc);

        // Borderless multi-line read-only edit with VSCROLL
        s_hEdit = CreateWindowExW(
            0, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
            rc.left + 8, rc.top + 8,
            rc.right - rc.left - 16,
            rc.bottom - rc.top - 16,
            hDlg, (HMENU)1001, hInst, NULL);

        SendMessageW(s_hEdit, WM_SETFONT, (WPARAM)s_hFont, TRUE);
        SendMessageW(s_hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));

        // Try to apply modern theme scrollbars where available (Vista+).
        SetWindowTheme(s_hEdit, L"Explorer", NULL);

        // Compose text
        std::wstring text;
        BuildAboutText(hInst, text);
        SetWindowTextW(s_hEdit, text.c_str());
    }

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INITDIALOG: {
            s_isOpen = TRUE;
            std::wstring t;
            SetWindowTextW(hDlg, UILang::Get(L"ABOUT_TITLE", t));
            s_hDlg = hDlg;
            if (!s_hBg) s_hBg = CreateSolidBrush(UiTheme::Bg());
            CreateEditAndFill(hDlg);
            return TRUE;
        }
        case WM_SIZE: {
            if (s_hEdit) {
                RECT rc; GetClientRect(hDlg, &rc);
                SetWindowPos(s_hEdit, NULL, rc.left + 8, rc.top + 8,
                    rc.right - rc.left - 16, rc.bottom - rc.top - 16,
                    SWP_NOZORDER | SWP_NOACTIVATE);
            }
            return 0;
        }
        // Paint theme on dialog and child
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetBkColor(hdc, UiTheme::Bg());
            SetTextColor(hdc, UiTheme::Text());
            return (INT_PTR)s_hBg;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                EndDialog(hDlg, IDOK);
                return 0;
            }
            return 0;
        case WM_DESTROY:
            s_isOpen = FALSE;
            if (s_hFont) { DeleteObject(s_hFont); s_hFont = NULL; }
            if (s_hBg) { DeleteObject(s_hBg);   s_hBg = NULL; }
            s_hEdit = NULL; s_hDlg = NULL;
            return 0;
        }
        return FALSE;
    }
}

namespace AboutDialog {
    void Show(HINSTANCE hInstance, HWND hParent) {
        // If already open, just bring it to front
        if (s_isOpen && s_hDlg && IsWindow(s_hDlg)) {
            ShowWindow(s_hDlg, SW_SHOWNORMAL);
            SetForegroundWindow(s_hDlg);
            return;
        }
        // Otherwise, open the modal dialog once
        DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_ABOUT), hParent, DlgProc, 0);
    }
}
