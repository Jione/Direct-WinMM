#include "RegistryManager.h"

// --- Configuration (from DLL context) ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
const wchar_t* const PLAYER_MODE_VALUE_NAME = L"PlayerMode";

// --- PlayerMode Bitmask Definitions ---
// DWORD (32-bit) layout:
// [31-20] Reserved (12 bits)
// [19-18] Engine Mode (2 bits: 00=Auto, 01=DS, 10=WASAPI)
// [17]    Buffer Mode (1 bit: 0=Streaming, 1=FullBuffer)
// [16]    Mute State (1 bit: 0=Unmuted, 1=Muted)
// [15-0]  Volume (16 bits: 0-65535)

// Volume (16 bits)
const DWORD PM_VOL_SHIFT = 0;
const DWORD PM_VOL_MASK = 0xFFFF << PM_VOL_SHIFT;

// Mute (1 bit)
const DWORD PM_MUTE_SHIFT = 16;
const DWORD PM_MUTE_MASK = 1 << PM_MUTE_SHIFT;

// Buffer Mode (1 bit)
const DWORD PM_BUFFER_SHIFT = 17;
const DWORD PM_BUFFER_MASK = 1 << PM_BUFFER_SHIFT;

// Engine Mode (2 bits)
const DWORD PM_ENGINE_SHIFT = 18;
const DWORD PM_ENGINE_MASK = 3 << PM_ENGINE_SHIFT;

// Default value: Volume=100% (0xFFFF), Mute=0, Buffer=0, Engine=0
const DWORD DEFAULT_PLAYER_MODE_DWORD = 0x0000FFFF;


namespace {
    HKEY g_hRegKey = NULL;

    // Convert DWORD [0, 65535] to percent [0, 100]
    int VolumeDwordToPercent(DWORD dwVol) {
        if (dwVol > 65535) dwVol = 65535;
        // Use floating point for better rounding
        return (int)((float)dwVol / 65535.0f * 100.0f + 0.5f);
    }

    // Convert percent [0, 100] to DWORD [0, 65535]
    DWORD VolumePercentToDword(int percent) {
        if (percent < 0) percent = 0;
        if (percent > 100) percent = 100;
        return (DWORD)((float)percent / 100.0f * 65535.0f + 0.5f);
    }
}

namespace RegistryManager {

    BOOL Initialize() {
        if (g_hRegKey) return TRUE; // Already initialized

        LSTATUS status = RegCreateKeyExW(HKEY_CURRENT_USER,
            REGISTRY_PATH,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE,
            NULL, &g_hRegKey, NULL);

        if (status != ERROR_SUCCESS) {
            g_hRegKey = NULL;
            return FALSE;
        }

        // Ensure the consolidated PlayerMode value exists
        DWORD dwPlayerMode;
        DWORD dwSize = sizeof(dwPlayerMode);
        DWORD dwType;
        status = RegQueryValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwPlayerMode, &dwSize);

        if (status == ERROR_FILE_NOT_FOUND) {
            // Not found, create with default
            dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
            RegSetValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwPlayerMode, sizeof(dwPlayerMode));
        }
        else if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            // Error or wrong type, overwrite
            dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
            RegSetValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwPlayerMode, sizeof(dwPlayerMode));
        }

        return TRUE;
    }

    void Close() {
        if (g_hRegKey) {
            RegCloseKey(g_hRegKey);
            g_hRegKey = NULL;
        }
    }

    // --- Raw DWORD Access ---
    DWORD GetPlayerMode() {
        if (!g_hRegKey) return DEFAULT_PLAYER_MODE_DWORD;

        DWORD dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
        DWORD dwSize = sizeof(dwPlayerMode);
        DWORD dwType;
        LSTATUS status = RegQueryValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwPlayerMode, &dwSize);

        if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            return DEFAULT_PLAYER_MODE_DWORD; // Return default on error
        }
        return dwPlayerMode;
    }

    BOOL SetPlayerMode(DWORD mode) {
        if (!g_hRegKey) return FALSE;

        LSTATUS status = RegSetValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&mode, sizeof(mode));
        return (status == ERROR_SUCCESS);
    }

    // --- (NEW) Bitmask Helper Implementations ---

    DWORD GetVolume() {
        return (GetPlayerMode() & PM_VOL_MASK) >> PM_VOL_SHIFT;
    }

    BOOL SetVolume(DWORD dwVolume) {
        if (dwVolume > 0xFFFF) dwVolume = 0xFFFF; // Clamp to 16-bit
        DWORD mode = GetPlayerMode();
        mode = (mode & ~PM_VOL_MASK) | (dwVolume << PM_VOL_SHIFT);
        return SetPlayerMode(mode);
    }

    BOOL GetMute() {
        return (GetPlayerMode() & PM_MUTE_MASK) != 0;
    }

    BOOL SetMute(BOOL isMute) {
        DWORD mode = GetPlayerMode();
        DWORD dwMute = isMute ? 1 : 0;
        mode = (mode & ~PM_MUTE_MASK) | (dwMute << PM_MUTE_SHIFT);
        return SetPlayerMode(mode);
    }

    BOOL GetBufferMode() {
        return (GetPlayerMode() & PM_BUFFER_MASK) != 0;
    }

    BOOL SetBufferMode(BOOL isFullBuffer) {
        DWORD mode = GetPlayerMode();
        DWORD dwBufferMode = isFullBuffer ? 1 : 0;
        mode = (mode & ~PM_BUFFER_MASK) | (dwBufferMode << PM_BUFFER_SHIFT);
        return SetPlayerMode(mode);
    }

    int GetEngineMode() {
        return (int)((GetPlayerMode() & PM_ENGINE_MASK) >> PM_ENGINE_SHIFT);
    }

    BOOL SetEngineMode(int engineMode) {
        if (engineMode < 0 || engineMode > 3) engineMode = 0; // Clamp to 2 bits (0-3)
        DWORD mode = GetPlayerMode();
        DWORD dwEngineMode = (DWORD)engineMode;
        mode = (mode & ~PM_ENGINE_MASK) | (dwEngineMode << PM_ENGINE_SHIFT);
        return SetPlayerMode(mode);
    }

    // --- (UPDATED) Percentage Helpers ---

    int GetVolumePercent() {
        return VolumeDwordToPercent(GetVolume());
    }

    BOOL SetVolumePercent(int percent) {
        return SetVolume(VolumePercentToDword(percent));
    }

} // namespace RegistryManager
