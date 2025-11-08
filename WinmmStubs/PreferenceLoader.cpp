#include "PreferenceLoader.h"
#include "AudioEngineAdapter.h" // To get/set master volume, buffer mode, AND engine mode
#include <iostream>
#include <shlwapi.h>
#include <tlhelp32.h>
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shlwapi.lib")

// Configuration
static const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
static const wchar_t* const SUBKEY_GLOBAL = L"Global";
static const wchar_t* const VAL_OVERRIDE = L"Override";
static const wchar_t* const VAL_EXE = L"";             // default value "@" (REG_SZ)
static const wchar_t* const VAL_PID = L"ProcessID";    // REG_DWORD
static const wchar_t* const VAL_LASTSEEN = L"LastSeen";     // REG_QWORD

const wchar_t* const EXE_NAME = L"WinmmVol.exe";
const wchar_t* const MUTEX_NAME = L"WinMM-Stubs Volume Control";
const wchar_t* const EXE_WINDOW_CLASS = L"WinMMStubsMainMsgWindowClass";
const UINT WM_EXIT_APP = WM_APP + 1; // Custom message to signal the EXE

// Bit layout
constexpr DWORD OV_VOL_SHIFT = 0;                // 0..100
constexpr DWORD OV_VOL_MASK = 0x7Fu << OV_VOL_SHIFT;

constexpr DWORD OV_MUTE_SHIFT = 7;                // 0/1
constexpr DWORD OV_MUTE_MASK = 1u << OV_MUTE_SHIFT;

constexpr DWORD OV_BUFFER_SHIFT = 8;                // 0=Auto,1=Streaming,2=Full,3=Resample
constexpr DWORD OV_BUFFER_MASK = 3u << OV_BUFFER_SHIFT;

constexpr DWORD OV_ENGINE_SHIFT = 10;               // 0=Auto,1=DS,2=WASAPI
constexpr DWORD OV_ENGINE_MASK = 3u << OV_ENGINE_SHIFT;

// Default = 0x00000064 (vol=100, mute=0, buffer=0, engine=0)
constexpr DWORD OV_DEFAULT = 0x00000064;


namespace {

    static HKEY  gRoot = NULL;   // HKCU\Software\WinmmStubs
    static HKEY  gGlobal = NULL;   // HKCU\...\Global
    static HKEY  gApp = NULL;   // HKCU\...\{GUID}
    static HANDLE gThread = NULL;
    static HANDLE gStop = NULL;

    static std::wstring gGuidKey;      // "{GUID}" as registry subkey name
    static std::wstring gExeFullPath;  // current exe full path
    static DWORD        gPid = 0;

    static int gBufferMode = 0;
    static int gEngineMode = 0;
    static int gVolume = 100;
    static BOOL gMute = FALSE;

    // simple 64-bit FNV-1a to derive a deterministic "GUID-like" key
    static ULONGLONG Fnv1a64(const void* data, size_t len) {
        const unsigned char* p = (const unsigned char*)data;
        ULONGLONG h = 1469598103934665603ULL;
        for (size_t i = 0; i < len; ++i) {
            h ^= (ULONGLONG)p[i];
            h *= 1099511628211ULL;
        }
        return h;
    }

    static std::wstring MakePseudoGuidFromPath(const std::wstring& path) {
        std::wstring lower = path;
        CharLowerBuffW(&lower[0], (DWORD)lower.size());
        ULONGLONG h1 = Fnv1a64(lower.c_str(), lower.size() * sizeof(wchar_t));
        ULONGLONG h2 = Fnv1a64(&h1, sizeof(h1));
        wchar_t buf[64];
        // 8-4-4-4-12 style (pseudo)
        wsprintfW(buf, L"%08X-%04X-%04X-%04X-%012I64X",
            (UINT)(h1 & 0xFFFFFFFFu),
            (UINT)((h1 >> 32) & 0xFFFFu),
            (UINT)((h1 >> 48) & 0xFFFFu),
            (UINT)(h2 & 0xFFFFu),
            (ULONGLONG)(h2 >> 16));
        return std::wstring(buf);
    }

    static inline ULONGLONG NowFileTimeQword() {
        FILETIME ft; ::GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER u; u.LowPart = ft.dwLowDateTime; u.HighPart = ft.dwHighDateTime;
        return u.QuadPart;
    }

    static bool OpenOrCreateKey(HKEY parent, LPCWSTR sub, REGSAM sam, HKEY& out) {
        out = NULL;
        return RegCreateKeyExW(parent, sub, 0, NULL, REG_OPTION_NON_VOLATILE, sam, NULL, &out, NULL) == ERROR_SUCCESS;
    }
    static bool OpenKey(HKEY parent, LPCWSTR sub, REGSAM sam, HKEY& out) {
        out = NULL;
        return RegOpenKeyExW(parent, sub, 0, sam, &out) == ERROR_SUCCESS;
    }
    static bool ReadDWORD(HKEY h, LPCWSTR name, DWORD& out) {
        DWORD type = 0, cb = sizeof(out);
        return (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)&out, &cb) == ERROR_SUCCESS && type == REG_DWORD);
    }
    static bool WriteDWORD(HKEY h, LPCWSTR name, DWORD v) {
        return RegSetValueExW(h, name, 0, REG_DWORD, (const BYTE*)&v, sizeof(v)) == ERROR_SUCCESS;
    }
    static bool ReadQWORD(HKEY h, LPCWSTR name, ULONGLONG& out) {
        DWORD type = 0, cb = sizeof(out);
        return (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)&out, &cb) == ERROR_SUCCESS && type == REG_QWORD);
    }
    static bool WriteQWORD(HKEY h, LPCWSTR name, ULONGLONG v) {
        return RegSetValueExW(h, name, 0, REG_QWORD, (const BYTE*)&v, sizeof(v)) == ERROR_SUCCESS;
    }
    static bool ReadSZ(HKEY h, LPCWSTR name, std::wstring& out) {
        DWORD type = 0, cb = 0;
        if (RegQueryValueExW(h, name, NULL, &type, NULL, &cb) != ERROR_SUCCESS) return false;
        if (type != REG_SZ && type != REG_EXPAND_SZ) return false;
        std::wstring buf; buf.resize(cb / sizeof(wchar_t));
        if (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)buf.data(), &cb) != ERROR_SUCCESS) return false;
        if (!buf.empty() && buf.back() == L'\0') buf.pop_back();
        out.swap(buf); return true;
    }
    static bool WriteSZ(HKEY h, LPCWSTR name, const std::wstring& v) {
        DWORD cb = (DWORD)((v.size() + 1) * sizeof(wchar_t));
        return RegSetValueExW(h, name, 0, REG_SZ, (const BYTE*)v.c_str(), cb) == ERROR_SUCCESS;
    }

    // field helpers
    static inline int  OV_GetVolume(DWORD ov) { return (int)((ov & OV_VOL_MASK) >> OV_VOL_SHIFT); }
    static inline BOOL OV_GetMute(DWORD ov) { return (ov & OV_MUTE_MASK) ? TRUE : FALSE; }
    static inline int  OV_GetBuffer(DWORD ov) { return (int)((ov & OV_BUFFER_MASK) >> OV_BUFFER_SHIFT); }
    static inline int  OV_GetEngine(DWORD ov) { return (int)((ov & OV_ENGINE_MASK) >> OV_ENGINE_SHIFT); }
    static inline DWORD OV_WithVolume(DWORD ov, int v) {
        if (v < 0) v = 0; if (v > 100) v = 100;
        return (ov & ~OV_VOL_MASK) | ((DWORD)v << OV_VOL_SHIFT);
    }

    // compute effective from two overrides
    static DWORD ComputeEffective(DWORD g, DWORD a) {
        int eng = OV_GetEngine(a); if (eng == 0) eng = OV_GetEngine(g);
        int buf = OV_GetBuffer(a); if (buf == 0) buf = OV_GetBuffer(g);
        BOOL mt = OV_GetMute(g) || OV_GetMute(a);
        int vg = OV_GetVolume(g), va = OV_GetVolume(a);
        int ve = (vg * va + 50) / 100;

        DWORD eff = 0;
        eff |= ((DWORD)eng << OV_ENGINE_SHIFT) & OV_ENGINE_MASK;
        eff |= ((DWORD)buf << OV_BUFFER_SHIFT) & OV_BUFFER_MASK;
        if (mt) eff |= OV_MUTE_MASK;
        eff = OV_WithVolume(eff, ve);
        return eff;
    }

    // WinmmVol.exe launcher / graceful close
    static BOOL IsProcessAlive(DWORD pid) {
        if (!pid) return FALSE;
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!h) return FALSE;
        DWORD code = 0; BOOL ok = GetExitCodeProcess(h, &code);
        CloseHandle(h);
        return ok && code == STILL_ACTIVE;
    }

    static BOOL LaunchWinmmVolIfExists() {
        HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
        bool alreadyRunning = (GetLastError() == ERROR_ALREADY_EXISTS);

        if (!alreadyRunning && hMutex) {
            dprintf(L"Mutex created. Attempting to launch %s...", EXE_NAME);
            STARTUPINFOW si = { sizeof(si) };
            PROCESS_INFORMATION pi = { 0 };
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
            wchar_t dir[MAX_PATH]; GetModuleFileNameW(NULL, dir, MAX_PATH);
            PathRemoveFileSpecW(dir);
            wchar_t exe[MAX_PATH]; lstrcpyW(exe, dir);
            PathAppendW(exe, EXE_NAME);
            if (!PathFileExistsW(exe)) {
                dprintf(L"Could not determine path to launch %s.", EXE_NAME);
                return FALSE;
            }
            std::wstring cmd = L"\"";
            cmd += exe;
            cmd += L"\" --spawned";
            BOOL ok = CreateProcessW(NULL, &cmd[0], NULL, NULL, FALSE, 0, NULL, dir, &si, &pi);
            if (ok) {
                dprintf(L"Launched %s successfully.", EXE_NAME);
                CloseHandle(pi.hThread);
                Sleep(500);
                return TRUE;
            }
            else {
                dprintf(L"Failed to launch %s. Error: %d", EXE_NAME, GetLastError());
            }
        }
        else if (alreadyRunning) {
            dprintf(L"%s seems to be already running (Mutex exists).", EXE_NAME);
            if (hMutex) CloseHandle(hMutex);
        }
        else {
            dprintf(L"Failed to create or open mutex '%s'. Error: %d", MUTEX_NAME, GetLastError());
        }
        return FALSE;
    }

    // -----------------------------------------------------------------------------
    // Registry init / touch
    // -----------------------------------------------------------------------------
    static BOOL EnsureRootAndGlobal() {
        if (!gRoot) {
            if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL,
                REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &gRoot, NULL) != ERROR_SUCCESS)
                return FALSE;
        }
        if (!gGlobal) {
            if (!OpenOrCreateKey(gRoot, SUBKEY_GLOBAL, KEY_READ | KEY_WRITE, gGlobal)) return FALSE;
            DWORD ov = 0; DWORD type = 0, cb = sizeof(ov);
            if (RegQueryValueExW(gGlobal, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD) {
                WriteDWORD(gGlobal, VAL_OVERRIDE, OV_DEFAULT);
            }
        }
        return TRUE;
    }

    static BOOL EnsureAppKey() {
        if (gApp) return TRUE;
        if (!OpenOrCreateKey(gRoot, gGuidKey.c_str(), KEY_READ | KEY_WRITE, gApp)) return FALSE;

        // @ exe path
        std::wstring cur;
        if (!ReadSZ(gApp, VAL_EXE, cur) || cur != gExeFullPath) WriteSZ(gApp, VAL_EXE, gExeFullPath);
        // PID
        DWORD pid = 0; if (!ReadDWORD(gApp, VAL_PID, pid) || pid != gPid) WriteDWORD(gApp, VAL_PID, gPid);
        // Override default
        DWORD ov = 0; DWORD type = 0, cb = sizeof(ov);
        if (RegQueryValueExW(gApp, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD) {
            WriteDWORD(gApp, VAL_OVERRIDE, OV_DEFAULT);
        }
        // LastSeen create
        ULONGLONG ls = 0; if (!ReadQWORD(gApp, VAL_LASTSEEN, ls)) WriteQWORD(gApp, VAL_LASTSEEN, NowFileTimeQword());

        return TRUE;
    }

    static void TouchLastSeen() {
        if (!gApp) return;
        WriteQWORD(gApp, VAL_LASTSEEN, NowFileTimeQword());
    }

    // read helpers
    static DWORD ReadGlobalOverride() {
        DWORD ov = OV_DEFAULT, type = 0, cb = sizeof(ov);
        if (!gGlobal) return OV_DEFAULT;
        if (RegQueryValueExW(gGlobal, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD)
            ov = OV_DEFAULT;
        return ov;
    }
    static DWORD ReadAppOverride() {
        DWORD ov = OV_DEFAULT, type = 0, cb = sizeof(ov);
        if (!gApp) return OV_DEFAULT;
        if (RegQueryValueExW(gApp, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD)
            ov = OV_DEFAULT;
        return ov;
    }

    // -----------------------------------------------------------------------------
    // Engine/apply
    // -----------------------------------------------------------------------------
    static float VolumePercentToFloat(int percent) {
        if ((DWORD)percent <= 0) return 0.0f;
        if ((DWORD)percent >= 100) return 1.0f;
        return (float)percent / 100.0f;
    }

    static void ApplyEffectiveOverride(DWORD eff) {
        int nBufferMode = OV_GetBuffer(eff);
        int nEngineMode = OV_GetEngine(eff);
        int nVolume = OV_GetVolume(eff);
        BOOL bMute = OV_GetMute(eff);

        if (gBufferMode != nBufferMode) {
            AudioEngine::SetBufferMode(nBufferMode);
            gBufferMode = nBufferMode;
            dprintf(L"Registry change detected. Setting BufferMode=%d.", nBufferMode);
        }
        if (gEngineMode != nEngineMode) {
            AudioEngine::SetEngineOverride(nEngineMode);
            gEngineMode = nEngineMode;
            dprintf(L"Registry change detected. Setting EngineMode=%d.", nEngineMode);
        }
        if ((gMute != bMute) || (gVolume != nVolume)) {
            float finalVolume = 0.0f;
            if (!bMute) {
                finalVolume = VolumePercentToFloat(nVolume);
            }
            AudioEngine::SetMasterVolume(finalVolume);
            gMute = bMute;
            gVolume = nVolume;
            dprintf(L"Registry change detected. Mute=%d Vol=%f", bMute, finalVolume);
        }
    }

    // -----------------------------------------------------------------------------
    // Watcher thread
    // -----------------------------------------------------------------------------
    static DWORD WINAPI WatcherThreadProc(LPVOID) {
        // open handles to watch
        HANDLE hEvtStop = gStop;

        HANDLE hEvtGlobal = CreateEventW(NULL, FALSE, FALSE, NULL);
        HANDLE hEvtApp = CreateEventW(NULL, FALSE, FALSE, NULL);

        auto ArmNotify = [](HKEY hKey, HANDLE hEvent) -> BOOL {
            if (!hKey || !hEvent) return FALSE;
            // subtree=FALSE (one key), change=LAST_SET, synchronous
            return RegNotifyChangeKeyValue(hKey, FALSE,
                REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                hEvent, TRUE) == ERROR_SUCCESS;
            };

        // initial arm
        ArmNotify(gGlobal, hEvtGlobal);
        ArmNotify(gApp, hEvtApp);

        // initial apply
        DWORD g = ReadGlobalOverride();
        DWORD a = ReadAppOverride();
        ApplyEffectiveOverride(ComputeEffective(g, a));

        // periodic LastSeen updater (10s)
        DWORD lastTick = GetTickCount();
        const DWORD beatMs = 10000;

        for (;;) {
            HANDLE waitList[3] = { hEvtStop, hEvtGlobal, hEvtApp };
            DWORD w = WaitForMultipleObjects(3, waitList, FALSE, 1000);
            if (w == WAIT_OBJECT_0) break; // stop
            else if (w == WAIT_OBJECT_0 + 1) {
                // Global changed
                ArmNotify(gGlobal, hEvtGlobal);
                g = ReadGlobalOverride();
                ApplyEffectiveOverride(ComputeEffective(g, a));
            }
            else if (w == WAIT_OBJECT_0 + 2) {
                // App changed
                ArmNotify(gApp, hEvtApp);
                a = ReadAppOverride();
                TouchLastSeen();
                ApplyEffectiveOverride(ComputeEffective(g, a));
            }
            else {
                // timeout: heartbeat
                DWORD now = GetTickCount();
                if (now - lastTick >= beatMs) {
                    TouchLastSeen();
                    lastTick = now;
                }
            }
        }

        CloseHandle(hEvtGlobal);
        CloseHandle(hEvtApp);
        return 0;
    }
}


namespace PreferenceLoader {
    
    BOOL Initialize() {
        if (gThread) return TRUE;

        // exe full path
        wchar_t path[MAX_PATH]; GetModuleFileNameW(NULL, path, MAX_PATH);
        gExeFullPath = path;
        // normalize to full path
        wchar_t full[MAX_PATH]; GetFullPathNameW(path, MAX_PATH, full, NULL);
        gExeFullPath = full;

        gPid = GetCurrentProcessId();
        gGuidKey = MakePseudoGuidFromPath(gExeFullPath);

        if (!EnsureRootAndGlobal()) return FALSE;
        if (!EnsureAppKey())       return FALSE;

        // ensure WinmmVol.exe is launched if exists
        LaunchWinmmVolIfExists();

        // create stop event and thread
        gStop = CreateEventW(NULL, TRUE, FALSE, NULL);
        if (!gStop) return FALSE;

        gThread = CreateThread(NULL, 0, WatcherThreadProc, NULL, 0, NULL);
        return gThread != NULL;
    }

    void Shutdown() {
        // mark ProcessID invalid, update LastSeen, close WinmmVol.exe if launched/live
        if (gApp) {
            WriteDWORD(gApp, VAL_PID, 0);
            WriteQWORD(gApp, VAL_LASTSEEN, NowFileTimeQword());
        }

        // stop thread
        if (gStop) {
            SetEvent(gStop);
            WaitForSingleObject(gThread, 3000);
        }

        if (gThread) { CloseHandle(gThread); gThread = NULL; }
        if (gStop) { CloseHandle(gStop);   gStop = NULL; }
        if (gApp) { RegCloseKey(gApp);    gApp = NULL; }
        if (gGlobal) { RegCloseKey(gGlobal); gGlobal = NULL; }
        if (gRoot) { RegCloseKey(gRoot);   gRoot = NULL; }
    }
} // namespace PreferenceLoader
