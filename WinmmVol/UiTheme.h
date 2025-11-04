#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace UiTheme {
    // Colors
    inline COLORREF Bg() { return RGB(252, 252, 252); }
    inline COLORREF Text() { return RGB(33, 33, 33); }

    // Simple UI font (9~10pt class)
    inline HFONT CreateUIFont(BOOL bold) {
        LOGFONTW lf = {};
        lf.lfHeight = -14; // approx 9pt on 96dpi
        lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
        // Language-aware face
        LANGID lid = GetUserDefaultUILanguage();
        if (PRIMARYLANGID(lid) == LANG_KOREAN) {
            lstrcpynW(lf.lfFaceName, L"Malgun Gothic", LF_FACESIZE);
        }
        else {
            lstrcpynW(lf.lfFaceName, L"Segoe UI", LF_FACESIZE);
        }
        // XP/legacy fallback
        if (lf.lfFaceName[0] == L'\0') {
            lstrcpynW(lf.lfFaceName, L"Tahoma", LF_FACESIZE);
        }
        lf.lfQuality = CLEARTYPE_NATURAL_QUALITY;
        return CreateFontIndirectW(&lf);
    }
}
