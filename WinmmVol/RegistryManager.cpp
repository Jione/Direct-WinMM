#include "RegistryManager.h"

// --- Configuration (from DLL context) ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
const wchar_t* const VOLUME_VALUE_NAME = L"MasterVolume";
const wchar_t* const MUTE_VALUE_NAME = L"isMute";
const wchar_t* const BUFFER_MODE_VALUE_NAME = L"isFullBuffer";
const DWORD DEFAULT_VOLUME_DWORD = 65535; // Default to 100%
const DWORD DEFAULT_MUTE_DWORD = 0; // Default to not muted
const DWORD DEFAULT_BUFFER_MODE_DWORD = 0;

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
            // Handle error (e.g., log)
            g_hRegKey = NULL;
            return FALSE;
        }

        // Ensure the value exists, create with default if not
        DWORD dwVolume;
        DWORD dwSize = sizeof(dwVolume);
        DWORD dwType;
        status = RegQueryValueExW(g_hRegKey, VOLUME_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwVolume, &dwSize);
        if (status == ERROR_FILE_NOT_FOUND) {
            dwVolume = DEFAULT_VOLUME_DWORD;
            RegSetValueExW(g_hRegKey, VOLUME_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwVolume, sizeof(dwVolume));
        }
        else if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            // Error reading or wrong type, overwrite with default
            dwVolume = DEFAULT_VOLUME_DWORD;
            RegSetValueExW(g_hRegKey, VOLUME_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwVolume, sizeof(dwVolume));
        }

        // Ensure the mute value exists
        DWORD dwMute;
        dwSize = sizeof(dwMute);
        status = RegQueryValueExW(g_hRegKey, MUTE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwMute, &dwSize);
        if (status == ERROR_FILE_NOT_FOUND || status != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwMute = DEFAULT_MUTE_DWORD;
            RegSetValueExW(g_hRegKey, MUTE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwMute, sizeof(dwMute));
        }
        
        // Ensure the buffer mode value exists
        DWORD dwBufferMode;
        dwSize = sizeof(dwBufferMode);
        status = RegQueryValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwBufferMode, &dwSize);
        if (status == ERROR_FILE_NOT_FOUND || status != ERROR_SUCCESS || dwType != REG_DWORD) {
            dwBufferMode = DEFAULT_BUFFER_MODE_DWORD;
            RegSetValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwBufferMode, sizeof(dwBufferMode));
        }

        return TRUE;
    }

    void Close() {
        if (g_hRegKey) {
            RegCloseKey(g_hRegKey);
            g_hRegKey = NULL;
        }
    }

    DWORD GetVolumeDword() {
        if (!g_hRegKey) return DEFAULT_VOLUME_DWORD;

        DWORD dwVolume = DEFAULT_VOLUME_DWORD;
        DWORD dwSize = sizeof(dwVolume);
        DWORD dwType;
        LSTATUS status = RegQueryValueExW(g_hRegKey, VOLUME_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwVolume, &dwSize);

        if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            return DEFAULT_VOLUME_DWORD; // Return default on error
        }
        return dwVolume;
    }

    BOOL SetVolumeDword(DWORD dwVolume) {
        if (!g_hRegKey) return FALSE;

        if (dwVolume > 65535) dwVolume = 65535; // Clamp

        LSTATUS status = RegSetValueExW(g_hRegKey, VOLUME_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwVolume, sizeof(dwVolume));

        return (status == ERROR_SUCCESS);
    }

    int GetVolumePercent() {
        return VolumeDwordToPercent(GetVolumeDword());
    }

    BOOL SetVolumePercent(int percent) {
        return SetVolumeDword(VolumePercentToDword(percent));
    }

    BOOL GetMute() {
        if (!g_hRegKey) return (DEFAULT_MUTE_DWORD == 1);

        DWORD dwMute = DEFAULT_MUTE_DWORD;
        DWORD dwSize = sizeof(dwMute);
        DWORD dwType;
        LSTATUS status = RegQueryValueExW(g_hRegKey, MUTE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwMute, &dwSize);

        if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            return (DEFAULT_MUTE_DWORD == 1); // Return default on error
        }
        return (dwMute == 1);
    }

    BOOL SetMute(BOOL isMute) {
        if (!g_hRegKey) return FALSE;

        DWORD dwMute = isMute ? 1 : 0;
        LSTATUS status = RegSetValueExW(g_hRegKey, MUTE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwMute, sizeof(dwMute));

        return (status == ERROR_SUCCESS);
    }

    BOOL GetBufferMode() {
        if (!g_hRegKey) return (DEFAULT_BUFFER_MODE_DWORD == 1);

        DWORD dwBufferMode = DEFAULT_BUFFER_MODE_DWORD;
        DWORD dwSize = sizeof(dwBufferMode);
        DWORD dwType;
        LSTATUS status = RegQueryValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwBufferMode, &dwSize);

        if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
            return (DEFAULT_BUFFER_MODE_DWORD == 1); // Return default on error
        }
        return (dwBufferMode == 1);
    }

    BOOL SetBufferMode(BOOL isFullBuffer) {
        if (!g_hRegKey) return FALSE;

        DWORD dwBufferMode = isFullBuffer ? 1 : 0;
        LSTATUS status = RegSetValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwBufferMode, sizeof(dwBufferMode));

        return (status == ERROR_SUCCESS);
    }
} // namespace RegistryManager
