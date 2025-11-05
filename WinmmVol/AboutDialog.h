#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace AboutDialog {
    void ShowUsage(HINSTANCE hInstance, HWND hParent);
    void ShowLicense(HINSTANCE hInstance, HWND hParent);
} // namespace AboutDialog
