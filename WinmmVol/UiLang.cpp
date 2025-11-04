#include "UiLang.h"
#include <unordered_map>
#include <vector>

static std::unordered_map<std::wstring, std::wstring> g_kv;
static HINSTANCE g_hInst = NULL;

static bool LoadLangResource(UINT resId) {
    HRSRC hRes = FindResourceW(g_hInst, MAKEINTRESOURCEW(resId), RT_RCDATA);
    if (!hRes) return false;
    HGLOBAL hData = LoadResource(g_hInst, hRes);
    if (!hData) return false;
    const void* p = LockResource(hData);
    DWORD sz = SizeofResource(g_hInst, hRes);
    if (!p || sz < 2) return false;

    // Expect UTF-16LE text with BOM. Parse as key=value per line.
    const wchar_t* w = reinterpret_cast<const wchar_t*>(p);
    size_t wlen = sz / sizeof(wchar_t);

    // Skip BOM if present
    size_t i = 0;
    if (wlen >= 1 && w[0] == 0xFEFF) i = 1;

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
                // Trim spaces
                auto trim = [](std::wstring& s) {
                    size_t b = s.find_first_not_of(L" \t");
                    size_t e = s.find_last_not_of(L" \t");
                    if (b == std::wstring::npos) { s.clear(); return; }
                    s = s.substr(b, e - b + 1);
                    };
                trim(k); trim(v);
                if (!k.empty()) g_kv[k] = v;
            }
            line.clear();
        }
        else {
            line.push_back(c);
        }
    }
    return true;
}

namespace UILang {
    void Init(HINSTANCE hInst) {
        g_hInst = hInst;
        g_kv.clear();

        // Choose resource by UI language
        LANGID lid = GetUserDefaultUILanguage();
        bool isKo = (PRIMARYLANGID(lid) == LANG_KOREAN);

        // Resource IDs defined in resource.h
        const UINT RES_EN = 9001;
        const UINT RES_KO = 9002;

        // Try chosen, fallback to EN
        if (isKo) {
            if (!LoadLangResource(RES_KO)) LoadLangResource(RES_EN);
        }
        else {
            if (!LoadLangResource(RES_EN)) LoadLangResource(RES_KO);
        }
    }

    const wchar_t* Get(const wchar_t* key, std::wstring& out) {
        out.clear();
        if (!key) return out.c_str();
        auto it = g_kv.find(key);
        if (it != g_kv.end()) {
            out = it->second;
        }
        else {
            out.clear();
        }
        return out.c_str();
    }
}
