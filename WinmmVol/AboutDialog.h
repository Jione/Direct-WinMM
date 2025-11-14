#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace AboutDialog {
    void ShowLicense(HINSTANCE hInstance, HWND hParent);
} // namespace AboutDialog
