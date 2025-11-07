#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>

// Base path: HKCU\Software\WinmmStubs
// Subkeys:
//   Global\Override (REG_DWORD)
//   {GUID}\@ (REG_SZ exe full path)
//   {GUID}\ProcessID (REG_DWORD)
//   {GUID}\Override  (REG_DWORD)
//   {GUID}\LastSeen  (REG_QWORD FILETIME UTC)

namespace RegistryManager {

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

    struct AppEntry {
        std::wstring guid;      // subkey name
        std::wstring exePath;   // default value "@"
        DWORD        pid;       // 0 if invalid
        BOOL         live;      // Is process still alive
        ULONGLONG    lastSeen;  // FILETIME (UTC)
    };

    // Init / fini --------------------------------------------------------------

    BOOL Initialize();         // ensure HKCU\Software\WinmmStubs and Global\Override exists
    void Close();

    // Environment --------------------------------------------------------------

    BOOL IsVistaOrLater();     // Vista+ == TRUE, XP == FALSE

    // Global override -----------------------------------------------------------

    DWORD GetGlobalOverride();
    BOOL  SetGlobalOverride(DWORD ov);

    // App override / heartbeat --------------------------------------------------

    BOOL GetAppOverride(const std::wstring& guid, DWORD& ov);
    BOOL SetAppOverride(const std::wstring& guid, DWORD ov);  // also updates LastSeen

    BOOL ResetAllSettings();

    // Ensure/Create and update app heartbeat (exe path, pid, last seen)
    BOOL TouchAppKey(const std::wstring& guid,
        const std::wstring& exeFullPath,
        DWORD pid);

    // Mark dead: if process is not alive, set ProcessID=0 (kept for history)
    void SweepAndInvalidateDeadPids();

    // Enumerate apps (optionally only live)
    BOOL EnumApps(std::vector<AppEntry>& out, BOOL onlyLive);

    // Capacity management: if #apps > maxCount, delete the oldest (by LastSeen)
    void GarbageCollectIfExceed(size_t maxCount);

    // Effective calculation -----------------------------------------------------
    // Merge Global and App according to rules:
    // - Engine: if app override != Auto, take app; else take global
    // - Buffer: if app override != Auto, take app; else take global
    // - Mute  : mute if either is muted
    // - Volume: product (global% * app% / 100)
    DWORD ComputeEffective(const std::wstring& guid);

    // Field helpers -------------------------------------------------------------

    inline int  OV_GetVolume(DWORD ov) { return (int)((ov & OV_VOL_MASK) >> OV_VOL_SHIFT); }
    inline BOOL OV_GetMute(DWORD ov) { return (ov & OV_MUTE_MASK) ? TRUE : FALSE; }
    inline int  OV_GetBuffer(DWORD ov) { return (int)((ov & OV_BUFFER_MASK) >> OV_BUFFER_SHIFT); }
    inline int  OV_GetEngine(DWORD ov) { return (int)((ov & OV_ENGINE_MASK) >> OV_ENGINE_SHIFT); }

    inline DWORD OV_WithVolume(DWORD ov, int v) {
        if (v < 0) v = 0; if (v > 100) v = 100;
        return (ov & ~OV_VOL_MASK) | ((DWORD)v << OV_VOL_SHIFT);
    }
    inline DWORD OV_WithMute(DWORD ov, BOOL m) {
        return m ? (ov | OV_MUTE_MASK) : (ov & ~OV_MUTE_MASK);
    }
    inline DWORD OV_WithBuffer(DWORD ov, int b) {
        if (b < 0) b = 0; if (b > 3) b = 0;
        return (ov & ~OV_BUFFER_MASK) | ((DWORD)b << OV_BUFFER_SHIFT);
    }
    inline DWORD OV_WithEngine(DWORD ov, int e) {
        if (e < 0) e = 0; if (e > 3) e = 0;
        return (ov & ~OV_ENGINE_MASK) | ((DWORD)e << OV_ENGINE_SHIFT);
    }

} // namespace RegistryManager
