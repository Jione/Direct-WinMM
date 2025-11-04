#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

namespace UILang {
    // Initialize and load best-matching language pack from resources.
    // Call once early (e.g., before showing About).
    void Init(HINSTANCE hInst);

    // Get translated string by key. Returns pointer to out.c_str().
    // If key not found, returns empty string.
    const wchar_t* Get(const wchar_t* key, std::wstring& out);
}
