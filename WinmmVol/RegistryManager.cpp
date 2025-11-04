#include "RegistryManager.h"

// --- Configuration (from DLL context) ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
const wchar_t* const PLAYER_MODE_VALUE_NAME = L"PlayerMode";

// --- PlayerMode Bitmask Definitions ---
// DWORD (32-bit) layout:
// [31-12] Reserved (20 bits)
// [11-10] Engine Mode (2 bits: 00=Auto, 01=DS, 10=WASAPI)
// [9-8]   Buffer Mode (2 bits: 00=Auto, 01=Streaming, 10=FullBuffer)
// [7]     Mute State (1 bit: 0=Unmuted, 1=Muted)
// [6-0]   Volume (7 bits: 0-100)

// Volume (7 bits)
const DWORD PM_VOL_SHIFT = 0;
const DWORD PM_VOL_MASK = 0x7F << PM_VOL_SHIFT; // 0x7F (0b1111111) for 7 bits

// Mute (1 bit)
const DWORD PM_MUTE_SHIFT = 7;
const DWORD PM_MUTE_MASK = 1 << PM_MUTE_SHIFT;

// Buffer Mode (2 bits)
const DWORD PM_BUFFER_SHIFT = 8;
const DWORD PM_BUFFER_MASK = 3 << PM_BUFFER_SHIFT; // 3 (0b11) for 2 bits

// Engine Mode (2 bits)
const DWORD PM_ENGINE_SHIFT = 10;
const DWORD PM_ENGINE_MASK = 3 << PM_ENGINE_SHIFT; // 3 (0b11) for 2 bits

// Default value: Volume=100 (0x64), Mute=0, Buffer=0, Engine=0
const DWORD DEFAULT_PLAYER_MODE_DWORD = 0x00000064;


namespace {
    HKEY g_hRegKey = NULL;
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

    // --- Bitmask Helper Implementations ---

    BOOL GetMute() {
        return (GetPlayerMode() & PM_MUTE_MASK) != 0;
    }

    BOOL SetMute(BOOL isMute) {
        DWORD mode = GetPlayerMode();
        DWORD dwMute = isMute ? 1 : 0;
        mode = (mode & ~PM_MUTE_MASK) | (dwMute << PM_MUTE_SHIFT);
        return SetPlayerMode(mode);
    }

    int GetBufferMode() {
        return (int)((GetPlayerMode() & PM_BUFFER_MASK) >> PM_BUFFER_SHIFT);
    }

    BOOL SetBufferMode(int bufferMode) {
        if (bufferMode < 0 || bufferMode > 3) bufferMode = 0; // Clamp to 2 bits (0-3)
        DWORD mode = GetPlayerMode();
        DWORD dwBufferMode = (DWORD)bufferMode;
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

    // --- Percentage Helpers ---

    int GetVolumePercent() {
        return (int)((GetPlayerMode() & PM_VOL_MASK) >> PM_VOL_SHIFT);
    }

    BOOL SetVolumePercent(int percent) {
        if ((DWORD)percent <= 0) percent = 0;
        else if ((DWORD)percent >= 100) percent = 100; // Clamp to 0-100 (fits in 7 bits)

        DWORD mode = GetPlayerMode();
        DWORD dwVolume = percent;
        mode = (mode & ~PM_VOL_MASK) | (dwVolume << PM_VOL_SHIFT);
        return SetPlayerMode(mode);
    }

} // namespace RegistryManager
