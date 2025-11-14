#include "UiLang.h"
#include "resource.h"
#include <unordered_map>
#include <vector>

static std::unordered_map<std::wstring, std::wstring> g_kv, g_kv_fallback;
static HINSTANCE g_hInst = NULL;

// Choose UILANG id by UI language.
static UINT SelectUiLangId() {
    LANGID lid = GetUserDefaultUILanguage();
    switch (PRIMARYLANGID(lid)) {
    case LANG_KOREAN:
        return IDR_UILANG_KO;
    case LANG_CATALAN:
        return IDR_UILANG_CA;
    case LANG_CZECH:
        return IDR_UILANG_CS;
    case LANG_GERMAN:
        return IDR_UILANG_DE;
    case LANG_SPANISH:
        return IDR_UILANG_ES;
    case LANG_FRENCH:
        return IDR_UILANG_FR;
    case LANG_INDONESIAN:
        return IDR_UILANG_ID;
    case LANG_ITALIAN:
        return IDR_UILANG_IT;
    case LANG_JAPANESE:
        return IDR_UILANG_JA;
    case LANG_GEORGIAN:
        return IDR_UILANG_KA;
    case LANG_LITHUANIAN:
        return IDR_UILANG_LT;
    case LANG_NORWEGIAN: // Covers Norwegian Bokmal
        return IDR_UILANG_NB;
    case LANG_DUTCH:
        return IDR_UILANG_NL;
    case LANG_POLISH:
        return IDR_UILANG_PL;
    case LANG_RUSSIAN:
        return IDR_UILANG_RU;
    case LANG_THAI:
        return IDR_UILANG_TH;
    case LANG_TURKISH:
        return IDR_UILANG_TR;
    // Handle sub-languages for Portuguese
    case LANG_PORTUGUESE:
        if (SUBLANGID(lid) == SUBLANG_PORTUGUESE_BRAZILIAN) {
            return IDR_UILANG_PT_BR;
        }
        break; // Fall through to default if not Brazilian
    // Handle sub-languages for Chinese
    case LANG_CHINESE:
        switch (SUBLANGID(lid)) {
        case SUBLANG_CHINESE_SIMPLIFIED: // zh-CN
        case SUBLANG_CHINESE_SINGAPORE:
            return IDR_UILANG_ZH_CN;
        case SUBLANG_CHINESE_HONGKONG: // zh-HK
            return IDR_UILANG_ZH_HK;
        case SUBLANG_CHINESE_TRADITIONAL: // zh-TW
        case SUBLANG_CHINESE_MACAU:
            return IDR_UILANG_ZH_TW;
        }
        break; // Fall through to default if not a matched sub-lang
    }
    return IDR_UILANG_EN;
}

// Replace "\n" with newline. Other escapes are left as-is.
static void UnescapeNewlines(std::wstring& s) {
    std::wstring out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        wchar_t c = s[i];
        if (c == L'\\' && (i + 1) < s.size() && s[i + 1] == L'n') {
            out.push_back(L'\n');
            ++i; // skip 'n'
        }
        else {
            out.push_back(c);
        }
    }
    s.swap(out);
}

static BOOL LoadLangResource(UINT resId, std::unordered_map<std::wstring, std::wstring>& kv) {
    HRSRC hRes = FindResourceW(g_hInst, MAKEINTRESOURCEW(resId), RT_RCDATA);
    if (!hRes) return FALSE;
    HGLOBAL hData = LoadResource(g_hInst, hRes);
    if (!hData) return FALSE;
    const void* p = LockResource(hData);
    DWORD sz = SizeofResource(g_hInst, hRes);
    if (!p || sz < 2) return FALSE;

    const wchar_t* w = reinterpret_cast<const wchar_t*>(p);
    size_t wlen = sz / sizeof(wchar_t);

    size_t i = 0;
    if (wlen >= 1 && w[0] == 0xFEFF) i = 1; // skip BOM

    std::wstring line;
    for (; i < wlen; ++i) {
        wchar_t c = w[i];
        if (c == L'\r') continue;
        if (c == L'\n' || i == wlen - 1) {
            if (i == wlen - 1 && c != L'\n' && c != L'\r') line.push_back(c);
            if (!line.empty() && line[0] != L'#' && line.find(L'=') != std::wstring::npos) {
                size_t eq = line.find(L'=');
                std::wstring k = line.substr(0, eq);
                std::wstring v = line.substr(eq + 1);
                // trim
                auto trim = [](std::wstring& s) {
                    size_t b = s.find_first_not_of(L" \t");
                    size_t e = s.find_last_not_of(L" \t");
                    if (b == std::wstring::npos) { s.clear(); return; }
                    s = s.substr(b, e - b + 1);
                    };
                trim(k); trim(v);

                // unescape "\n" to newline
                UnescapeNewlines(v);
                if (!k.empty()) kv[k] = v;
            }
            line.clear();
        }
        else {
            line.push_back(c);
        }
    }
    return TRUE;
}

namespace UILang {
    void Init(HINSTANCE hInst) {
        g_hInst = hInst;
        g_kv.clear();
        g_kv_fallback.clear();

        LoadLangResource(SelectUiLangId(), g_kv);
        LoadLangResource(IDR_UILANG_EN, g_kv_fallback);
    }

    const wchar_t* Get(const wchar_t* key, std::wstring& out) {
        out.clear();
        if (!key) return out.c_str();

        auto it = g_kv.find(key);

        if (it != g_kv.end()) {
            out = it->second;
        }

        if (out.empty()) {
            it = g_kv_fallback.find(key);
            if (it != g_kv_fallback.end()) {
                out = it->second;
            }
            else {
                out.clear();
            }
        }

        return out.c_str();
    }
}
