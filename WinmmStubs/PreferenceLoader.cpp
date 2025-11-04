#include "PreferenceLoader.h"
#include "AudioEngineAdapter.h" // To get/set master volume, buffer mode, AND engine mode
#include "DeviceInfo.h"         // To potentially link initialization
#include <windows.h>
#include <regstr.h> // For REGSTR_PATH_SOFTWARE etc. (optional, can build string manually)
#include <process.h> // For _beginthreadex

// --- Configuration ---
const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs"; // Relative to HKCU

// Consolidated to one registry value
const wchar_t* const PLAYER_MODE_VALUE_NAME = L"PlayerMode";

const wchar_t* const EXE_NAME = L"WinmmVol.exe";
const wchar_t* const MUTEX_NAME = L"WinMM-Stubs Volume Control";
const wchar_t* const EXE_WINDOW_CLASS = L"WinMMStubsMainMsgWindowClass";
const UINT WM_EXIT_APP = WM_APP + 1; // Custom message to signal the EXE

// --- PlayerMode Bitmask Definitions (Self-contained) ---
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

    // Converts 7-bit percent (0-100) to float (0.0 - 1.0).
    static float VolumePercentToFloat(int percent) {
        if ((DWORD)percent <= 0) return 0.0f;
        if ((DWORD)percent >= 100) return 100.0f;
        return (float)percent / 100.0f;
    }

    // The background thread function that monitors registry changes.
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
                    // Read the single PlayerMode DWORD
                    DWORD dwPlayerMode = 0;
                    DWORD dwSize = sizeof(dwPlayerMode);
                    DWORD dwType = 0;
                    LSTATUS status = RegQueryValueExW(g_hRegKey,
                        PLAYER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwPlayerMode, &dwSize);

                    // Use default on failure
                    if (status != ERROR_SUCCESS || dwType != REG_DWORD) {
                        dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
                    }

                    // --- Extract values from bitmask ---
                    int nVolume = (int)((dwPlayerMode & PM_VOL_MASK) >> PM_VOL_SHIFT); // 7-bit int
                    BOOL bMute = (dwPlayerMode & PM_MUTE_MASK) != 0;
                    int nBufferMode = (int)((dwPlayerMode & PM_BUFFER_MASK) >> PM_BUFFER_SHIFT);
                    int nEngineMode = (int)((dwPlayerMode & PM_ENGINE_MASK) >> PM_ENGINE_SHIFT);

                    // --- Apply all settings ---

                    // Apply Volume Logic
                    float finalVolume;
                    if (bMute) {
                        finalVolume = 0.0f;
                        dprintf(L"Registry change detected. PlayerMode=0x%X. Setting Mute=1.", dwPlayerMode);
                    }
                    else {
                        finalVolume = VolumePercentToFloat(nVolume); // Convert 0-100 to 0.0-1.0
                        dprintf(L"Registry change detected. PlayerMode=0x%X. Setting Vol=%f.", dwPlayerMode, finalVolume);
                    }
                    AudioEngine::SetMasterVolume(finalVolume);

                    // Apply Buffer Mode Logic
                    dprintf(L"Registry change detected. Setting BufferMode=%d.", nBufferMode);
                    AudioEngine::SetBufferMode(nBufferMode);

                    // Apply Engine Mode Logic
                    dprintf(L"Registry change detected. Setting EngineMode=%d.", nEngineMode);
                    AudioEngine::SetEngineOverride(nEngineMode);
                }
                ResetEvent(g_hRegChangeEvent); // Reset the event *before* re-registering
                Unlock();
            }
            else {
                dprintf(L"WaitForMultipleObjects failed in MonitorThreadProc: %d", GetLastError());
                Sleep(500); // Avoid tight loop on error
            }
        } // while(true)

        return 0;
    }

    // Tries to find the main window of the WinmmVol.exe process.
    static HWND FindExeWindow() {
        HWND hwnd = FindWindowW(EXE_WINDOW_CLASS, NULL);
        return hwnd;
    }

} // namespace


namespace PreferenceLoader {

    BOOL Initialize() {
        if (g_bInitialized) return TRUE;

        InitializeCriticalSection(&g_cs);
        Lock();

        if (g_bInitialized) {
            Unlock();
            return TRUE;
        }

        bool success = true;

        HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
        bool alreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

        if (!alreadyRunning && hMutex) {
            dprintf(L"Mutex created. Attempting to launch %s...", EXE_NAME);
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            wchar_t exePath[MAX_PATH];
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
            GetModuleFileNameW(NULL, exePath, MAX_PATH);
            wchar_t* lastSlash = wcsrchr(exePath, L'\\');
            if (lastSlash) {
                *(lastSlash + 1) = L'\0';
                lstrcatW(exePath, EXE_NAME);
                if (CreateProcessW(NULL, exePath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
                    dprintf(L"Launched %s successfully.", EXE_NAME);
                    g_hExeProcess = pi.hProcess;
                    CloseHandle(pi.hThread);
                    Sleep(500);
                    g_hExeWnd = FindExeWindow();
                }
                else {
                    dprintf(L"Failed to launch %s. Error: %d", EXE_NAME, GetLastError());
                }
            }
            else {
                dprintf(L"Could not determine path to launch %s.", EXE_NAME);
            }
        }
        else if (alreadyRunning) {
            dprintf(L"%s seems to be already running (Mutex exists).", EXE_NAME);
            if (hMutex) CloseHandle(hMutex);
            g_hExeWnd = FindExeWindow();
        }
        else {
            dprintf(L"Failed to create or open mutex '%s'. Error: %d", MUTEX_NAME, GetLastError());
        }


        // Open/Create Registry Key
        LSTATUS regStatus = RegCreateKeyExW(HKEY_CURRENT_USER,
            REGISTRY_PATH,
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_READ | KEY_WRITE | KEY_NOTIFY,
            NULL, &g_hRegKey, NULL);

        if (regStatus != ERROR_SUCCESS) {
            dprintf(L"Failed to open/create registry key '%s'. Error: %d", REGISTRY_PATH, regStatus);
            success = false;
        }
        else {
            // Check/Create the single PlayerMode Value
            DWORD dwPlayerMode = 0;
            DWORD dwSize = sizeof(dwPlayerMode);
            DWORD dwType = 0;
            regStatus = RegQueryValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, NULL, &dwType, (LPBYTE)&dwPlayerMode, &dwSize);

            if (regStatus == ERROR_FILE_NOT_FOUND) {
                dprintf(L"Registry value '%s' not found. Creating with default 0x%X", PLAYER_MODE_VALUE_NAME, DEFAULT_PLAYER_MODE_DWORD);
                dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
                regStatus = RegSetValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwPlayerMode, sizeof(dwPlayerMode));
                if (regStatus != ERROR_SUCCESS) {
                    dprintf(L"Failed to create registry value '%s'. Error: %d", PLAYER_MODE_VALUE_NAME, regStatus);
                    success = false;
                }
            }
            else if (regStatus != ERROR_SUCCESS || dwType != REG_DWORD) {
                dprintf(L"Failed to query registry value '%s' or wrong type. Overwriting...", PLAYER_MODE_VALUE_NAME);
                dwPlayerMode = DEFAULT_PLAYER_MODE_DWORD;
                RegSetValueExW(g_hRegKey, PLAYER_MODE_VALUE_NAME, 0, REG_DWORD, (const BYTE*)&dwPlayerMode, sizeof(dwPlayerMode));
            }
            // At this point, dwPlayerMode holds the last saved (or default) settings

            // --- Extract and apply all initial settings from the DWORD ---
            // Extract values
            int nVolume = (int)((dwPlayerMode & PM_VOL_MASK) >> PM_VOL_SHIFT); // 7-bit int
            BOOL bMute = (dwPlayerMode & PM_MUTE_MASK) != 0;
            int nBufferMode = (int)((dwPlayerMode & PM_BUFFER_MASK) >> PM_BUFFER_SHIFT);
            int nEngineMode = (int)((dwPlayerMode & PM_ENGINE_MASK) >> PM_ENGINE_SHIFT);

            // Apply Initial Volume Based on Mute State
            if (bMute) {
                AudioEngine::SetMasterVolume(0.0f);
                dprintf(L"Initial state: Muted. (PlayerMode=0x%X)", dwPlayerMode);
            }
            else {
                float initialVolume = VolumePercentToFloat(nVolume); // Convert 0-100 to 0.0-1.0
                AudioEngine::SetMasterVolume(initialVolume);
                dprintf(L"Initial state: Unmuted. Set volume to %f. (PlayerMode=0x%X)", initialVolume, dwPlayerMode);
            }

            // Apply Initial Buffer Mode
            AudioEngine::SetBufferMode(nBufferMode);
            dprintf(L"Initial state: BufferMode=%d. (PlayerMode=0x%X)", nBufferMode, dwPlayerMode);

            // Apply Initial Engine Mode
            AudioEngine::SetEngineOverride(nEngineMode);
            dprintf(L"Initial state: EngineMode=%d. (PlayerMode=0x%X)", nEngineMode, dwPlayerMode);


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
        }

        g_bInitialized = success;
        Unlock();
        return g_bInitialized;
    }

    void Shutdown() {
        if (!g_bInitialized) return;

        Lock();
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

        if (g_hExeWnd && IsWindow(g_hExeWnd)) {
            dprintf(L"Posting exit message (0x%X) to HWND 0x%p", WM_EXIT_APP, g_hExeWnd);
            PostMessageW(g_hExeWnd, WM_EXIT_APP, 0, 0);
        }
        else {
            HWND hwnd = FindExeWindow();
            if (hwnd && IsWindow(hwnd)) {
                dprintf(L"Posting exit message (0x%X) to newly found HWND 0x%p", WM_EXIT_APP, hwnd);
                PostMessageW(hwnd, WM_EXIT_APP, 0, 0);
            }
            else {
                dprintf(L"Could not find window for %s to send exit message.", EXE_NAME);
            }
        }
        if (g_hExeProcess) {
            CloseHandle(g_hExeProcess);
            g_hExeProcess = NULL;
        }

        g_bInitialized = FALSE;
        Unlock();

        DeleteCriticalSection(&g_cs);
        dprintf(L"PreferenceLoader shutdown complete.");
    }

} // namespace PreferenceLoader
