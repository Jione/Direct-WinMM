#include "PreferenceLoader.h"
#include "AudioEngineAdapter.h" // To get/set master volume
#include "DeviceInfo.h"         // To potentially link initialization
#include <windows.h>
#include <regstr.h> // For REGSTR_PATH_SOFTWARE etc. (optional, can build string manually)
#include <process.h> // For _beginthreadex

// --- Configuration ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs"; // Relative to HKCU
const wchar_t* const VOLUME_VALUE_NAME = L"MasterVolume";
const wchar_t* const MUTE_VALUE_NAME = L"isMute";
const wchar_t* const BUFFER_MODE_VALUE_NAME = L"isFullBuffer";
const wchar_t* const EXE_NAME = L"WinmmVol.exe";
const wchar_t* const MUTEX_NAME = L"WinMM-Stubs Volume Control";
const wchar_t* const EXE_WINDOW_CLASS = L"WinMMStubsMainMsgWindowClass";
const UINT WM_EXIT_APP = WM_APP + 1; // Custom message to signal the EXE
const DWORD DEFAULT_MUTE_DWORD = 0;
const DWORD DEFAULT_BUFFER_MODE_DWORD = 0;

namespace {
    // --- Internal State ---
    static CRITICAL_SECTION g_cs;
    static BOOL g_bInitialized = FALSE;
    static HANDLE g_hMonitorThread = NULL;
    static HANDLE g_hQuitEvent = NULL;
    static HKEY g_hRegKey = NULL;
    static HANDLE g_hRegChangeEvent = NULL;
    static HANDLE g_hExeProcess = NULL; // Keep track
    static HWND g_hExeWnd = NULL;       // Cache the window handle

    // --- Internal Helpers ---
    static void Lock() { EnterCriticalSection(&g_cs); }
    static void Unlock() { LeaveCriticalSection(&g_cs); }

    // Convert float [0.0, 1.0] to DWORD [0, 65535]
    static DWORD VolumeFloatToDword(float vol) {
        if (vol < 0.0f) vol = 0.0f;
        if (vol > 1.0f) vol = 1.0f;
        return (DWORD)(vol * 65535.0f + 0.5f);
    }

    // Convert DWORD [0, 65535] to float [0.0, 1.0]
    static float VolumeDwordToFloat(DWORD dwVol) {
        if (dwVol > 65535) dwVol = 65535;
        return (float)dwVol / 65535.0f;
    }

    /**
     * @brief The background thread function that monitors registry changes.
     */
    static unsigned int __stdcall MonitorThreadProc(void* /*pParam*/) {
        while (true) {
            // Register for change notification
            if (g_hRegKey && g_hRegChangeEvent) {
                RegNotifyChangeKeyValue(g_hRegKey,
                    FALSE, // Don't watch subtree
                    REG_NOTIFY_CHANGE_LAST_SET, // Watch value changes
                    g_hRegChangeEvent,
                    TRUE); // Asynchronous
            }
            else {
                // Should not happen if initialized correctly
                Sleep(1000);
                continue;
            }

            // Wait for registry change or quit signal
            HANDLE waitHandles[] = { g_hQuitEvent, g_hRegChangeEvent };
            DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);

            if (waitResult == WAIT_OBJECT_0) {
                // Quit event signaled
                break;
            }
            else if (waitResult == WAIT_OBJECT_0 + 1) {
                Lock();
                if (g_hRegKey) {
                    // Read both values to determine the final volume
                    DWORD dwVolume = 0;
                    DWORD dwSizeVol = sizeof(dwVolume);
                    DWORD dwTypeVol = 0;
                    LSTATUS volStatus = RegQueryValueExW(g_hRegKey,
                        VOLUME_VALUE_NAME, NULL, &dwTypeVol, (LPBYTE)&dwVolume, &dwSizeVol);

                    DWORD dwMute = 0;
                    DWORD dwSizeMute = sizeof(dwMute);
                    DWORD dwTypeMute = 0;
                    LSTATUS muteStatus = RegQueryValueExW(g_hRegKey,
                        MUTE_VALUE_NAME, NULL, &dwTypeMute, (LPBYTE)&dwMute, &dwSizeMute);

                    DWORD dwBufferMode = 0;
                    DWORD dwSizeBuffer = sizeof(dwBufferMode);
                    DWORD dwTypeBuffer = 0;
                    LSTATUS bufferStatus = RegQueryValueExW(g_hRegKey,
                        BUFFER_MODE_VALUE_NAME, NULL, &dwTypeBuffer, (LPBYTE)&dwBufferMode, &dwSizeBuffer);

                    // Use defaults on failure (though Initialize should prevent this)
                    if (volStatus != ERROR_SUCCESS || dwTypeVol != REG_DWORD) {
                        dwVolume = VolumeFloatToDword(0.0f); // Fallback
                    }
                    if (muteStatus != ERROR_SUCCESS || dwTypeMute != REG_DWORD) {
                        dwMute = DEFAULT_MUTE_DWORD; // 0
                    }
                    if (bufferStatus != ERROR_SUCCESS || dwTypeBuffer != REG_DWORD) {
                        dwBufferMode = DEFAULT_BUFFER_MODE_DWORD; // 0
                    }

                    // Apply the logic: if muted, volume is 0. If not, use master volume.
                    float finalVolume;
                    if (dwMute == 1) {
                        finalVolume = 0.0f;
                        dprintf(L"Registry change detected. isMute=1. Setting engine volume to 0.0.");
                    }
                    else {
                        finalVolume = VolumeDwordToFloat(dwVolume);
                        dprintf(L"Registry change detected. isMute=0. Setting engine volume to %f (0x%X).", finalVolume, dwVolume);
                    }
                    AudioEngine::SetMasterVolume(finalVolume);

                    // Apply Buffer Mode Logic
                    BOOL bFullBuffer = (dwBufferMode == 1);
                    dprintf(L"Registry change detected. isFullBuffer=%d. Setting buffer mode.", dwBufferMode);
                    AudioEngine::SetBufferMode(bFullBuffer); // This will stop/restart if playing
                }
                ResetEvent(g_hRegChangeEvent); // Reset the event *before* re-registering
                Unlock();
            }
            else {
                // Wait failed?
                dprintf(L"WaitForMultipleObjects failed in MonitorThreadProc: %d", GetLastError());
                Sleep(500); // Avoid tight loop on error
            }
        } // while(true)

        return 0;
    }

    /**
     * @brief Tries to find the main window of the WinmmVol.exe process.
     * @return HWND if found, NULL otherwise.
     */
    static HWND FindExeWindow() {
        // Try finding by title first
        HWND hwnd = FindWindowW(EXE_WINDOW_CLASS, NULL);
        // Add more specific methods if needed (e.g., specific class name)
        return hwnd;
    }

} // namespace


namespace PreferenceLoader {

    BOOL Initialize() {
        if (g_bInitialized) return TRUE;

        InitializeCriticalSection(&g_cs);
        Lock();

        // Check if already initialized by another thread while waiting for lock
        if (g_bInitialized) {
            Unlock();
            return TRUE;
        }

        bool success = true;

        // Check/Launch External EXE using Mutex
        HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
        bool alreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

        if (!alreadyRunning && hMutex) {
            // try to launch the EXE
            dprintf(L"Mutex created. Attempting to launch %s...", EXE_NAME);
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            wchar_t exePath[MAX_PATH];

            // Release the mutex immediately after checking/launching
            ReleaseMutex(hMutex);
            CloseHandle(hMutex); // Close our handle to it

            // Assuming EXE is in the same directory as DLL for simplicity
            GetModuleFileNameW(NULL, exePath, MAX_PATH); // Get host EXE path
            wchar_t* lastSlash = wcsrchr(exePath, L'\\');
            if (lastSlash) {
                *(lastSlash + 1) = L'\0'; // Truncate to directory
                lstrcatW(exePath, EXE_NAME);

                if (CreateProcessW(NULL, exePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                    dprintf(L"Launched %s successfully.", EXE_NAME);
                    g_hExeProcess = pi.hProcess; // Store handle if needed for termination
                    CloseHandle(pi.hThread);
                    // Give it a moment to start and create its window
                    Sleep(500);
                    g_hExeWnd = FindExeWindow(); // Try to find the window
                }
                else {
                    dprintf(L"Failed to launch %s. Error: %d", EXE_NAME, GetLastError());
                    // Continue initialization even if EXE fails to launch
                }
            }
            else {
                dprintf(L"Could not determine path to launch %s.", EXE_NAME);
            }
        }
        else if (alreadyRunning) {
            dprintf(L"%s seems to be already running (Mutex exists).", EXE_NAME);
            if (hMutex) CloseHandle(hMutex);
            g_hExeWnd = FindExeWindow(); // Try to find window of existing process
        }
        else {
            dprintf(L"Failed to create or open mutex '%s'. Error: %d", MUTEX_NAME, GetLastError());
            // Continue initialization despite mutex error
        }


        // Open/Create Registry Key
        LSTATUS regStatus = RegCreateKeyExW(HKEY_CURRENT_USER,
            REGISTRY_PATH,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE | KEY_NOTIFY,
            NULL, &g_hRegKey, NULL);

        if (regStatus != ERROR_SUCCESS) {
            dprintf(L"Failed to open/create registry key '%s'. Error: %d", REGISTRY_PATH, regStatus);
            success = false; // Cannot proceed without registry key
        }
        else {
            // Check/Create Volume Value and Set Initial Volume
            DWORD dwVolume = 0;
            DWORD dwSize = sizeof(dwVolume);
            DWORD dwType = 0;
            regStatus = RegQueryValueExW(g_hRegKey, VOLUME_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwVolume, &dwSize);

            if (regStatus == ERROR_FILE_NOT_FOUND) {
                // Value doesn't exist, create it with current engine volume
                dprintf(L"Registry value '%s' not found. Creating...", VOLUME_VALUE_NAME);
                float currentVolume = AudioEngine::GetMasterVolume();
                dwVolume = VolumeFloatToDword(currentVolume);
                regStatus = RegSetValueExW(g_hRegKey, VOLUME_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwVolume, sizeof(dwVolume));
                if (regStatus != ERROR_SUCCESS) {
                    dprintf(L"Failed to create registry value '%s'. Error: %d", VOLUME_VALUE_NAME, regStatus);
                    success = false;
                }
                else {
                    dprintf(L"Created registry value '%s' with initial volume %f (0x%X)", VOLUME_VALUE_NAME, currentVolume, dwVolume);
                }
            }
            else if (regStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
                // Error reading value or wrong type, overwrite with default
                dprintf(L"Failed to query registry value '%s' or wrong type. Overwriting...", VOLUME_VALUE_NAME);
                float currentVolume = AudioEngine::GetMasterVolume();
                dwVolume = VolumeFloatToDword(currentVolume);
                RegSetValueExW(g_hRegKey, VOLUME_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwVolume, sizeof(dwVolume));
            }
            // At this point, dwVolume holds the last saved master volume

            // Check/Create Mute Value
            DWORD dwMute = 0;
            dwSize = sizeof(dwMute);
            regStatus = RegQueryValueExW(g_hRegKey, MUTE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwMute, &dwSize);

            if (regStatus == ERROR_FILE_NOT_FOUND || regStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
                dprintf(L"Registry value '%s' not found or invalid. Creating...", MUTE_VALUE_NAME);
                dwMute = DEFAULT_MUTE_DWORD; // 0 (not muted)
                RegSetValueExW(g_hRegKey, MUTE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwMute, sizeof(dwMute));
            }
            // At this point, dwMute holds the last saved mute state

            // Check/Create Buffer Mode Value
            DWORD dwBufferMode = 0;
            dwSize = sizeof(dwBufferMode);
            regStatus = RegQueryValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwBufferMode, &dwSize);

            if (regStatus == ERROR_FILE_NOT_FOUND || regStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
                dprintf(L"Registry value '%s' not found or invalid. Creating...", BUFFER_MODE_VALUE_NAME);
                dwBufferMode = DEFAULT_BUFFER_MODE_DWORD; // 0 (Streaming)
                RegSetValueExW(g_hRegKey, BUFFER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwBufferMode, sizeof(dwBufferMode));
            }
            // At this point, dwBufferMode holds the last saved buffer mode

            // Apply Initial Volume Based on Mute State
            if (dwMute == 1) {
                AudioEngine::SetMasterVolume(0.0f);
                dprintf(L"Initial state: Muted. Set volume to 0.0. (Master level is %f)", VolumeDwordToFloat(dwVolume));
            }
            else {
                float initialVolume = VolumeDwordToFloat(dwVolume);
                AudioEngine::SetMasterVolume(initialVolume);
                dprintf(L"Initial state: Unmuted. Set volume to %f.", initialVolume);
            }
            
            // Apply Initial Buffer Mode
            BOOL bFullBuffer = (dwBufferMode == 1);
            AudioEngine::SetBufferMode(bFullBuffer);
            dprintf(L"Initial state: BufferMode=%s. Set mode.", bFullBuffer ? L"FullBuffer" : L"Streaming");

            // Create Events and Start Monitor Thread (only if registry key is valid)
            if (success) {
                g_hQuitEvent = CreateEventW(NULL, TRUE, FALSE, NULL); // Manual reset
                g_hRegChangeEvent = CreateEventW(NULL, FALSE, FALSE, NULL); // Auto reset
                if (!g_hQuitEvent || !g_hRegChangeEvent) {
                    dprintf(L"Failed to create events. Error: %d", GetLastError());
                    success = false;
                }
                else {
                    unsigned int threadId;
                    g_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadProc, NULL, 0, &threadId);
                    if (!g_hMonitorThread) {
                        dprintf(L"Failed to create monitor thread. Error: %d", GetLastError());
                        success = false;
                    }
                    else {
                        dprintf(L"Registry monitor thread started.");
                    }
                }
            }
        }

        // Cleanup on failure
        if (!success) {
            if (g_hMonitorThread) { CloseHandle(g_hMonitorThread); g_hMonitorThread = NULL; }
            if (g_hQuitEvent) { CloseHandle(g_hQuitEvent); g_hQuitEvent = NULL; }
            if (g_hRegChangeEvent) { CloseHandle(g_hRegChangeEvent); g_hRegChangeEvent = NULL; }
            if (g_hRegKey) { RegCloseKey(g_hRegKey); g_hRegKey = NULL; }
            // Don't close g_hExeProcess here, let Shutdown handle it if needed
        }

        g_bInitialized = success;
        Unlock();
        return g_bInitialized;
    }

    void Shutdown() {
        if (!g_bInitialized) return;

        Lock();
        // Check again inside lock
        if (!g_bInitialized) {
            Unlock();
            return;
        }

        dprintf(L"Shutting down PreferenceLoader...");

        // Signal and wait for monitor thread
        if (g_hQuitEvent) {
            SetEvent(g_hQuitEvent);
        }
        if (g_hMonitorThread) {
            WaitForSingleObject(g_hMonitorThread, 5000); // 5 sec timeout
            CloseHandle(g_hMonitorThread);
            g_hMonitorThread = NULL;
        }

        // Close handles
        if (g_hQuitEvent) { CloseHandle(g_hQuitEvent); g_hQuitEvent = NULL; }
        if (g_hRegChangeEvent) { CloseHandle(g_hRegChangeEvent); g_hRegChangeEvent = NULL; }
        if (g_hRegKey) { RegCloseKey(g_hRegKey); g_hRegKey = NULL; }

        // Signal External EXE to close/cleanup
        if (g_hExeWnd && IsWindow(g_hExeWnd)) {
            dprintf(L"Posting exit message (0x%X) to HWND 0x%p", WM_EXIT_APP, g_hExeWnd);
            // PostMessage is non-blocking and safer from DLL context
            PostMessageW(g_hExeWnd, WM_EXIT_APP, 0, 0);
        }
        else {
            // Try finding it again
            HWND hwnd = FindExeWindow();
            if (hwnd && IsWindow(hwnd)) {
                dprintf(L"Posting exit message (0x%X) to newly found HWND 0x%p", WM_EXIT_APP, hwnd);
                PostMessageW(hwnd, WM_EXIT_APP, 0, 0);
            }
            else {
                dprintf(L"Could not find window for %s to send exit message.", EXE_NAME);
                // Be careful as this is not graceful.
                // if (g_hExeProcess) TerminateProcess(g_hExeProcess, 0);
            }
        }
        if (g_hExeProcess) {
            CloseHandle(g_hExeProcess); // Close our handle to the process
            g_hExeProcess = NULL;
        }

        g_bInitialized = FALSE;
        Unlock();

        // Delete CS *after* unlocking
        DeleteCriticalSection(&g_cs);
        dprintf(L"PreferenceLoader shutdown complete.");
    }

} // namespace PreferenceLoader
