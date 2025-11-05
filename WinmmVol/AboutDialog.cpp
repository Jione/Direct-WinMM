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
    enum ABOUT_MODE {
        ABOUT_USAGE = 0,
        ABOUT_LICENSE = 1
    };

    static HWND   s_hDlg = NULL;
    static HWND   s_hEdit = NULL;
    static HFONT  s_hFont = NULL;
    static HBRUSH s_hBg = NULL;
    static BOOL   s_isOpen = FALSE;
    static ABOUT_MODE s_mode = ABOUT_USAGE;

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

    // Build usage-only
    static void BuildUsageText(HINSTANCE hInst, std::wstring& out) {
        out.clear();
        LoadUtf16RsrcToString(hInst, SelectUsageId(), out);
    }

    // Build license-only (concatenate existing parts)
    static void BuildLicenseText(HINSTANCE hInst, std::wstring& out) {
        out.clear();
        static const UINT kParts[] = {
            IDR_LICENSE_LIBOGG, IDR_LICENSE_LIBVORBIS, IDR_LICENSE_LIBFLAC, IDR_LICENSE_MINIMP3
        };
        std::wstring tmp;
        for (UINT id : kParts) {
            if (LoadUtf16RsrcToString(hInst, id, tmp)) {
                if (!out.empty()) out.append(L"\r\n\r\n");
                out.append(tmp);
            }
        }
    }

    static void CreateEditAndFill(HWND hDlg) {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hDlg, GWLP_HINSTANCE);

        if (!s_hFont) s_hFont = UiTheme::CreateUIFont(FALSE);
        if (!s_hBg)   s_hBg = CreateSolidBrush(UiTheme::Bg());

        RECT rc; GetClientRect(hDlg, &rc);

        if (!s_hEdit) {
            s_hEdit = CreateWindowExW(
                0, L"EDIT", L"",
                WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
                rc.left + 8, rc.top + 8,
                rc.right - rc.left - 16,
                rc.bottom - rc.top - 16,
                hDlg, (HMENU)1001, hInst, NULL);
            SendMessageW(s_hEdit, WM_SETFONT, (WPARAM)s_hFont, TRUE);
            SendMessageW(s_hEdit, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELPARAM(8, 8));
            SetWindowTheme(s_hEdit, L"Explorer", NULL);
        }
        else {
            SetWindowPos(s_hEdit, NULL, rc.left + 8, rc.top + 8,
                rc.right - rc.left - 16, rc.bottom - rc.top - 16,
                SWP_NOZORDER | SWP_NOACTIVATE);
        }

        std::wstring text;
        if (s_mode == ABOUT_USAGE) {
            BuildUsageText(hInst, text);
        }
        else {
            BuildLicenseText(hInst, text);
        }
        SetWindowTextW(s_hEdit, text.c_str());
    }

    // Load dialog icons (big/small) from IDI_APPICON
    static void SetDialogIcon(HWND hDlg) {
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtrW(hDlg, GWLP_HINSTANCE);

        int cxBig = GetSystemMetrics(SM_CXICON);
        int cyBig = GetSystemMetrics(SM_CYICON);
        int cxSmall = GetSystemMetrics(SM_CXSMICON);
        int cySmall = GetSystemMetrics(SM_CYSMICON);

        HICON hBig = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON),
            IMAGE_ICON, cxBig, cyBig, LR_DEFAULTCOLOR);
        HICON hSml = (HICON)LoadImageW(hInst, MAKEINTRESOURCEW(IDI_APPICON),
            IMAGE_ICON, cxSmall, cySmall, LR_DEFAULTCOLOR);

        if (hBig)  SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hBig);
        if (hSml)  SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hSml);

        // Store in GWLP_USERDATA or static, then destroy on WM_DESTROY
        // Here: store as properties for later destroy
        if (hBig)  SetPropW(hDlg, L"ABOUT_BIG_ICON", (HANDLE)hBig);
        if (hSml)  SetPropW(hDlg, L"ABOUT_SMALL_ICON", (HANDLE)hSml);
    }

    static void FreeDialogIcon(HWND hDlg) {
        HICON hBig = (HICON)GetPropW(hDlg, L"ABOUT_BIG_ICON");
        HICON hSml = (HICON)GetPropW(hDlg, L"ABOUT_SMALL_ICON");
        if (hBig) { RemovePropW(hDlg, L"ABOUT_BIG_ICON");   DestroyIcon(hBig); }
        if (hSml) { RemovePropW(hDlg, L"ABOUT_SMALL_ICON"); DestroyIcon(hSml); }
    }

    static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
        case WM_INITDIALOG: {
            s_isOpen = TRUE;
            s_hDlg = hDlg;

            if (!s_hBg)   s_hBg = CreateSolidBrush(UiTheme::Bg());
            if (!s_hFont) s_hFont = UiTheme::CreateUIFont(FALSE);

            // Set window title by mode (Usage / License)
            std::wstring t;
            if (s_mode == ABOUT_USAGE) {
                SetWindowTextW(hDlg, UILang::Get(L"ABOUT_TITLE_USAGE", t));
            }
            else {
                SetWindowTextW(hDlg, UILang::Get(L"ABOUT_TITLE_LICENSE", t));
            }

            CreateEditAndFill(hDlg);
            SetDialogIcon(hDlg);
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
            FreeDialogIcon(hDlg);
            return 0;
        }
        return FALSE;
    }
}

namespace AboutDialog {
    static void ShowInternal(HINSTANCE hInstance, HWND hParent, ABOUT_MODE requested) {
        // if already open
        if (s_isOpen && s_hDlg && IsWindow(s_hDlg)) {
            if (s_mode == requested) {
                ShowWindow(s_hDlg, SW_SHOWNORMAL);
                SetForegroundWindow(s_hDlg);
            }
            // different mode -> do nothing
            return;
        }
        // not open -> set mode and open
        s_mode = requested;
        DialogBoxParamW(hInstance, MAKEINTRESOURCEW(IDD_ABOUT), hParent, DlgProc, 0);
    }

    void ShowUsage(HINSTANCE hInstance, HWND hParent) { ShowInternal(hInstance, hParent, ABOUT_USAGE); }
    void ShowLicense(HINSTANCE hInstance, HWND hParent) { ShowInternal(hInstance, hParent, ABOUT_LICENSE); }
}
