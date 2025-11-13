#include "RegistryManager.h"
#include <algorithm>

namespace {
    // Registry paths/values
    const wchar_t* const REGISTRY_PATH = L"Software\\WinmmStubs";
    const wchar_t* const SUBKEY_GLOBAL = L"Global";
    const wchar_t* const VAL_OVERRIDE = L"Override";
    const wchar_t* const VAL_EXE = L"";            // default value "@" (REG_SZ)
    const wchar_t* const VAL_PID = L"ProcessID";   // REG_DWORD
    const wchar_t* const VAL_LASTSEEN = L"LastSeen";    // REG_QWORD

    HKEY gRoot = NULL;

    inline ULONGLONG NowFileTimeQword() {
        FILETIME ft; ::GetSystemTimeAsFileTime(&ft);
        ULARGE_INTEGER u; u.LowPart = ft.dwLowDateTime; u.HighPart = ft.dwHighDateTime;
        return u.QuadPart;
    }

    inline bool ReadDWORD(HKEY h, LPCWSTR name, DWORD& out) {
        DWORD type = 0, cb = sizeof(out);
        if (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)&out, &cb) == ERROR_SUCCESS && type == REG_DWORD) return true;
        return false;
    }
    inline bool ReadQWORD(HKEY h, LPCWSTR name, ULONGLONG& out) {
        DWORD type = 0, cb = sizeof(out);
        if (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)&out, &cb) == ERROR_SUCCESS && type == REG_QWORD) return true;
        return false;
    }
    inline bool ReadSZ(HKEY h, LPCWSTR name, std::wstring& out) {
        DWORD type = 0, cb = 0;
        if (RegQueryValueExW(h, name, NULL, &type, NULL, &cb) != ERROR_SUCCESS || (type != REG_SZ && type != REG_EXPAND_SZ)) return false;
        std::wstring buf; buf.resize(cb / sizeof(wchar_t));
        if (RegQueryValueExW(h, name, NULL, &type, (LPBYTE)buf.data(), &cb) != ERROR_SUCCESS) return false;
        if (!buf.empty() && buf.back() == L'\0') buf.pop_back();
        out.swap(buf); return true;
    }
    inline bool WriteDWORD(HKEY h, LPCWSTR name, DWORD v) {
        return RegSetValueExW(h, name, 0, REG_DWORD, (const BYTE*)&v, sizeof(v)) == ERROR_SUCCESS;
    }
    inline bool WriteQWORD(HKEY h, LPCWSTR name, ULONGLONG v) {
        return RegSetValueExW(h, name, 0, REG_QWORD, (const BYTE*)&v, sizeof(v)) == ERROR_SUCCESS;
    }
    inline bool WriteSZ(HKEY h, LPCWSTR name, const std::wstring& v) {
        DWORD cb = (DWORD)((v.size() + 1) * sizeof(wchar_t));
        return RegSetValueExW(h, name, 0, REG_SZ, (const BYTE*)v.c_str(), cb) == ERROR_SUCCESS;
    }
    inline bool OpenSub(HKEY parent, LPCWSTR sub, REGSAM sam, HKEY& out, bool createIfMissing) {
        out = NULL;
        if (createIfMissing) {
            return RegCreateKeyExW(parent, sub, 0, NULL, REG_OPTION_NON_VOLATILE, sam, NULL, &out, NULL) == ERROR_SUCCESS;
        }
        return RegOpenKeyExW(parent, sub, 0, sam, &out) == ERROR_SUCCESS;
    }

    inline bool IsProcessAlive(DWORD pid) {
        if (!pid) return false;
        HANDLE h = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
        if (!h) return false;
        DWORD code = 0; BOOL ok = ::GetExitCodeProcess(h, &code);
        CloseHandle(h);
        return ok && code == STILL_ACTIVE;
    }

} // anon

namespace RegistryManager {

    BOOL Initialize() {
        if (gRoot) return TRUE;
        if (RegCreateKeyExW(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &gRoot, NULL) != ERROR_SUCCESS) {
            gRoot = NULL; return FALSE;
        }
        // Ensure Global\Override exists
        HKEY hGlobal = NULL;
        if (OpenSub(gRoot, SUBKEY_GLOBAL, KEY_READ | KEY_WRITE, hGlobal, true)) {
            DWORD ov = 0; DWORD type = 0, cb = sizeof(ov);
            if (RegQueryValueExW(hGlobal, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD) {
                ov = OV_DEFAULT;
                WriteDWORD(hGlobal, VAL_OVERRIDE, ov);
            }
            RegCloseKey(hGlobal);
        }
        IsVistaOrLater();
        return TRUE;
    }

    void Close() {
        if (gRoot) { RegCloseKey(gRoot); gRoot = NULL; }
    }

    BOOL IsVistaOrLater() {
        static int cached = -1;
        if (cached != -1) return cached ? TRUE : FALSE;

        HMODULE hNt = GetModuleHandleW(L"ntdll.dll");
        if (hNt) {
            typedef LONG(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
            RtlGetVersion_t p = (RtlGetVersion_t)GetProcAddress(hNt, "RtlGetVersion");
            if (p) {
                RTL_OSVERSIONINFOW vi = { sizeof(vi) };
                if (p(&vi) == 0) { cached = (vi.dwMajorVersion >= 6) ? 1 : 0; return cached ? TRUE : FALSE; }
            }
        }
        OSVERSIONINFOEXW os = { sizeof(os) }; GetVersionExW((OSVERSIONINFOW*)&os);
        cached = (os.dwMajorVersion >= 6) ? 1 : 0;
        return cached ? TRUE : FALSE;
    }

    BOOL HasAnyLiveApp() {
        std::vector<AppEntry> apps;
        if (!EnumApps(apps, TRUE)) return FALSE; // onlyLive = TRUE
        return !apps.empty();
    }

    BOOL IsAppLive(const std::wstring& guid) {
        if (!gRoot || guid.empty()) return FALSE;
        HKEY hApp = NULL;
        if (RegOpenKeyExW(gRoot, guid.c_str(), 0, KEY_READ, &hApp) != ERROR_SUCCESS) return FALSE;
        DWORD pid = 0, type = 0, cb = sizeof(pid);
        RegQueryValueExW(hApp, VAL_PID, NULL, &type, (LPBYTE)&pid, &cb);
        RegCloseKey(hApp);
        return IsProcessAlive(pid) ? TRUE : FALSE; // anon namespace helper already exists
    }

    // Global -------------------------------------------------------------------

    DWORD GetGlobalOverride() {
        if (!gRoot) return OV_DEFAULT;
        HKEY hGlobal = NULL;
        if (!OpenSub(gRoot, SUBKEY_GLOBAL, KEY_READ, hGlobal, false)) return OV_DEFAULT;
        DWORD ov = OV_DEFAULT, type = 0, cb = sizeof(ov);
        if (RegQueryValueExW(hGlobal, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD)
            ov = OV_DEFAULT;
        RegCloseKey(hGlobal);
        return ov;
    }

    BOOL SetGlobalOverride(DWORD ov) {
        if (!gRoot) return FALSE;
        HKEY hGlobal = NULL;
        if (!OpenSub(gRoot, SUBKEY_GLOBAL, KEY_READ | KEY_WRITE, hGlobal, true)) return FALSE;
        BOOL ok = WriteDWORD(hGlobal, VAL_OVERRIDE, ov);
        RegCloseKey(hGlobal);
        return ok;
    }

    // Apps ---------------------------------------------------------------------

    BOOL GetAppOverride(const std::wstring& guid, DWORD& ov) {
        ov = OV_DEFAULT;
        if (!gRoot || guid.empty()) return FALSE;
        HKEY hApp = NULL;
        if (!OpenSub(gRoot, guid.c_str(), KEY_READ, hApp, false)) return FALSE;
        DWORD type = 0, cb = sizeof(ov);
        if (RegQueryValueExW(hApp, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD)
            ov = OV_DEFAULT;
        RegCloseKey(hApp);
        return TRUE;
    }

    BOOL SetAppOverride(const std::wstring& guid, DWORD ov) {
        if (!gRoot || guid.empty()) return FALSE;
        HKEY hApp = NULL;
        if (!OpenSub(gRoot, guid.c_str(), KEY_READ | KEY_WRITE, hApp, true)) return FALSE;
        BOOL ok = WriteDWORD(hApp, VAL_OVERRIDE, ov);
        WriteQWORD(hApp, VAL_LASTSEEN, NowFileTimeQword());
        RegCloseKey(hApp);
        return ok;
    }

    BOOL ResetAllSettings() {
        if (!gRoot) return FALSE;

        // delete non-live app keys
        {
            DWORD idx = 0; wchar_t name[256]; DWORD cch; FILETIME ft = {};
            for (;;) {
                cch = _countof(name);
                LONG r = RegEnumKeyExW(gRoot, idx, name, &cch, NULL, NULL, NULL, &ft);
                if (r == ERROR_NO_MORE_ITEMS) break;
                if (r != ERROR_SUCCESS) { ++idx; continue; }
                if (lstrcmpiW(name, SUBKEY_GLOBAL) == 0) { ++idx; continue; }

                // read PID
                HKEY hApp = NULL;
                if (RegOpenKeyExW(gRoot, name, 0, KEY_READ, &hApp) == ERROR_SUCCESS) {
                    DWORD pid = 0, type = 0, cb = sizeof(pid);
                    RegQueryValueExW(hApp, VAL_PID, NULL, &type, (LPBYTE)&pid, &cb);
                    RegCloseKey(hApp);

                    if (!IsProcessAlive(pid)) {
                        // not live -> delete subkey
                        RegDeleteKeyW(gRoot, name); // keys contain only values; no subkeys expected
                        // do not ++idx here (current index now points to next item)
                        continue;
                    }
                }
                ++idx;
            }
        }

        // reset Global\Override
        if (!SetGlobalOverride(OV_DEFAULT)) return FALSE;

        // reset Override of live apps
        {
            DWORD idx = 0; wchar_t name[256]; DWORD cch; FILETIME ft = {};
            for (;;) {
                cch = _countof(name);
                LONG r = RegEnumKeyExW(gRoot, idx++, name, &cch, NULL, NULL, NULL, &ft);
                if (r == ERROR_NO_MORE_ITEMS) break;
                if (r != ERROR_SUCCESS) continue;
                if (lstrcmpiW(name, SUBKEY_GLOBAL) == 0) continue;

                HKEY hApp = NULL;
                if (RegOpenKeyExW(gRoot, name, 0, KEY_READ | KEY_WRITE, &hApp) != ERROR_SUCCESS) continue;

                // live check again (race-safe)
                DWORD pid = 0, type = 0, cb = sizeof(pid);
                RegQueryValueExW(hApp, VAL_PID, NULL, &type, (LPBYTE)&pid, &cb);
                if (IsProcessAlive(pid)) {
                    // reset override + last seen update
                    RegSetValueExW(hApp, VAL_OVERRIDE, 0, REG_DWORD, (const BYTE*)&OV_DEFAULT, sizeof(OV_DEFAULT));
                    ULONGLONG now;
                    {
                        FILETIME ftNow; GetSystemTimeAsFileTime(&ftNow);
                        ULARGE_INTEGER u; u.LowPart = ftNow.dwLowDateTime; u.HighPart = ftNow.dwHighDateTime; now = u.QuadPart;
                    }
                    RegSetValueExW(hApp, VAL_LASTSEEN, 0, REG_QWORD, (const BYTE*)&now, sizeof(now));
                }
                RegCloseKey(hApp);
            }
        }
        return TRUE;
    }

    BOOL TouchAppKey(const std::wstring& guid, const std::wstring& exeFullPath, DWORD pid) {
        if (!gRoot || guid.empty()) return FALSE;
        HKEY hApp = NULL;
        if (!OpenSub(gRoot, guid.c_str(), KEY_READ | KEY_WRITE, hApp, true)) return FALSE;

        // @ (exe path)
        std::wstring cur;
        if (!ReadSZ(hApp, VAL_EXE, cur) || cur != exeFullPath)
            WriteSZ(hApp, VAL_EXE, exeFullPath);

        // ProcessID
        DWORD curPid = 0;
        if (!ReadDWORD(hApp, VAL_PID, curPid) || curPid != pid)
            WriteDWORD(hApp, VAL_PID, pid);

        // Override default
        DWORD ov = 0, type = 0, cb = sizeof(ov);
        if (RegQueryValueExW(hApp, VAL_OVERRIDE, NULL, &type, (LPBYTE)&ov, &cb) != ERROR_SUCCESS || type != REG_DWORD)
            WriteDWORD(hApp, VAL_OVERRIDE, OV_DEFAULT);

        // LastSeen
        WriteQWORD(hApp, VAL_LASTSEEN, NowFileTimeQword());

        RegCloseKey(hApp);
        return TRUE;
    }

    void SweepAndInvalidateDeadPids() {
        if (!gRoot) return;
        DWORD idx = 0; wchar_t name[256]; DWORD cch = 256; FILETIME ft = {};
        while (true) {
            cch = _countof(name);
            LONG r = RegEnumKeyExW(gRoot, idx++, name, &cch, NULL, NULL, NULL, &ft);
            if (r == ERROR_NO_MORE_ITEMS) break;
            if (r != ERROR_SUCCESS) continue;
            if (lstrcmpiW(name, SUBKEY_GLOBAL) == 0) continue; // skip Global

            HKEY hApp = NULL;
            if (RegOpenKeyExW(gRoot, name, 0, KEY_READ | KEY_WRITE, &hApp) != ERROR_SUCCESS) continue;

            DWORD pid = 0;
            ReadDWORD(hApp, VAL_PID, pid);
            if (!IsProcessAlive(pid)) {
                WriteDWORD(hApp, VAL_PID, 0); // mark as invalid
                // still keep it; GC can remove later
            }
            RegCloseKey(hApp);
        }
    }

    BOOL EnumApps(std::vector<AppEntry>& out, BOOL onlyLive) {
        out.clear();
        if (!gRoot) return FALSE;

        DWORD idx = 0; wchar_t name[256]; DWORD cch = 256; FILETIME ft = {};
        while (true) {
            cch = _countof(name);
            LONG r = RegEnumKeyExW(gRoot, idx++, name, &cch, NULL, NULL, NULL, &ft);
            if (r == ERROR_NO_MORE_ITEMS) break;
            if (r != ERROR_SUCCESS) continue;
            if (lstrcmpiW(name, SUBKEY_GLOBAL) == 0) continue;

            HKEY hApp = NULL;
            if (RegOpenKeyExW(gRoot, name, 0, KEY_READ, &hApp) != ERROR_SUCCESS) continue;

            AppEntry e; e.guid = name;
            ReadSZ(hApp, VAL_EXE, e.exePath);
            DWORD pid = 0; ReadDWORD(hApp, VAL_PID, pid); e.pid = pid;
            e.live = IsProcessAlive(pid) ? TRUE : FALSE;
            ULONGLONG ls = 0; ReadQWORD(hApp, VAL_LASTSEEN, ls); e.lastSeen = ls;

            RegCloseKey(hApp);

            if (onlyLive && !e.live) continue;
            out.push_back(e);
        }
        return TRUE;
    }

    void GarbageCollectIfExceed(size_t maxCount) {
        if (!gRoot || maxCount == 0) return;

        // Gather all non-Global keys
        std::vector<AppEntry> apps;
        EnumApps(apps, FALSE);
        if (apps.size() <= maxCount) return;

        // Sort by LastSeen ascending (oldest first)
        std::sort(apps.begin(), apps.end(),
            [](const AppEntry& a, const AppEntry& b) { return a.lastSeen < b.lastSeen; });

        size_t toDelete = apps.size() - maxCount;
        for (size_t i = 0; i < toDelete; ++i) {
            RegDeleteKeyW(gRoot, apps[i].guid.c_str()); // ignore error
        }
    }

    // Effective -----------------------------------------------------------------

    DWORD ComputeEffective(const std::wstring& guid) {
        const DWORD g = GetGlobalOverride();

        if (guid.empty()) return g;

        DWORD a = OV_DEFAULT;
        if (!GetAppOverride(guid, a)) a = OV_DEFAULT;

        // Engine: app if not Auto else global
        int eng = OV_GetEngine(a); if (eng == 0) eng = OV_GetEngine(g);
        // Buffer: app if not Auto else global
        int buf = OV_GetBuffer(a); if (buf == 0) buf = OV_GetBuffer(g);
        // Mute: either
        BOOL mt = OV_GetMute(g) || OV_GetMute(a);
        // Volume: product
        int vg = OV_GetVolume(g); int va = OV_GetVolume(a);
        int ve = (vg * va + 50) / 100; // round

        DWORD eff = 0;
        eff = OV_WithEngine(eff, eng);
        eff = OV_WithBuffer(eff, buf);
        eff = OV_WithMute(eff, mt);
        eff = OV_WithVolume(eff, ve);
        return eff;
    }

    BOOL GetGuidByPid(DWORD pid, std::wstring& outGuid, BOOL onlyLive) {
        outGuid.clear();
        if (!gRoot || pid == 0) return FALSE;

        // Enumerate all app subkeys and pick the newest LastSeen that matches pid
        DWORD idx = 0; wchar_t name[256]; DWORD cch = 256; FILETIME ft = {};
        ULONGLONG bestLastSeen = 0;
        std::wstring bestGuid;

        while (true) {
            cch = _countof(name);
            LONG r = RegEnumKeyExW(gRoot, idx++, name, &cch, NULL, NULL, NULL, &ft);
            if (r == ERROR_NO_MORE_ITEMS) break;
            if (r != ERROR_SUCCESS) continue;
            if (lstrcmpiW(name, SUBKEY_GLOBAL) == 0) continue; // skip Global

            HKEY hApp = NULL;
            if (RegOpenKeyExW(gRoot, name, 0, KEY_READ, &hApp) != ERROR_SUCCESS) continue;

            DWORD vpid = 0;
            DWORD type = 0, cb = sizeof(vpid);
            RegQueryValueExW(hApp, VAL_PID, NULL, &type, (LPBYTE)&vpid, &cb);
            if (type != REG_DWORD) vpid = 0;

            ULONGLONG ls = 0;
            ReadQWORD(hApp, VAL_LASTSEEN, ls);

            RegCloseKey(hApp);

            if (vpid != pid) continue;

            if (onlyLive && !IsProcessAlive(vpid)) {
                // ignore stale
                continue;
            }

            if (ls >= bestLastSeen) {
                bestLastSeen = ls;
                bestGuid = name;
            }
        }

        if (bestGuid.empty()) return FALSE;
        outGuid = bestGuid;
        return TRUE;
    }

} // namespace RegistryManager
