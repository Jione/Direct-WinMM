#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace RegistryManager {
    BOOL Initialize();
    void Close();

    // Gets PlayerMode DWORD data
    DWORD GetPlayerMode();
    // Sets PlayerMode raw data
    BOOL SetPlayerMode(DWORD mode);

    // Helper: Gets volume as percentage 0-100
    int GetVolumePercent();
    // Helper: Sets volume from percentage 0-100
    BOOL SetVolumePercent(int percent);

    // Gets mute state (TRUE if muted, FALSE otherwise)
    BOOL GetMute();
    // Sets mute state
    BOOL SetMute(BOOL isMute);

    // Gets buffer mode state (TRUE if FullBuffer, FALSE if Streaming)
    int GetBufferMode();
    // Sets buffer mode state
    BOOL SetBufferMode(int bufferMode);

    // Gets Engine mode state
    int GetEngineMode();
    // Sets Engine mode state
    BOOL SetEngineMode(int engineMode);

} // namespace RegistryManager
