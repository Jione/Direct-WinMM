#include "RegistryManager.h"

// --- Configuration (from DLL context) ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
const wchar_t* const VOLUME_VALUE_NAME = L"MasterVolume";
const DWORD DEFAULT_VOLUME_DWORD = 65535; // Default to 100%

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

} // namespace RegistryManager
