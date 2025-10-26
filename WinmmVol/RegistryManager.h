#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace RegistryManager {
    BOOL Initialize();
    void Close();

    // Gets volume as 0-65535 DWORD
    DWORD GetVolumeDword();
    // Sets volume from 0-65535 DWORD
    BOOL SetVolumeDword(DWORD dwVolume);

    // Helper: Gets volume as percentage 0-100
    int GetVolumePercent();
    // Helper: Sets volume from percentage 0-100
    BOOL SetVolumePercent(int percent);

} // namespace RegistryManager
