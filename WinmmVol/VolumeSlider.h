#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace VolumeSlider {
    // Registers the window class for the slider pop-up
    BOOL RegisterWindowClass(HINSTANCE hInstance);

    // Creates the (initially hidden) slider window
    HWND Create(HINSTANCE hInstance, HWND hParent);

    // Shows the slider window near the specified point (usually cursor pos)
    void Show(HWND hwndSlider, POINT pt);

    // Updates the slider position and text based on registry value
    void UpdateDisplay(HWND hwndSlider);

} // namespace VolumeSlider
