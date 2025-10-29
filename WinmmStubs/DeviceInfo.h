#pragma once
#include "GlobalDefinitions.h"

// Maximum number of MCI devices manageable within one process (Increase if needed)
#ifndef DT_MAX_DEVICES
#define DT_MAX_DEVICES  16
#endif

// Default time format for CD devices: Start with MSF (Minute:Second:Frame)
#ifndef DT_DEFAULT_TIMEFORMAT
#define DT_DEFAULT_TIMEFORMAT  MCI_FORMAT_MSF
#endif

// Basic CD Device Information
struct IDevice {
    BOOL      fillData;       // Flag to check if data has been filled
    WORD      wMid;           // Manufacturer ID
    WORD      wPid;           // Product Identifier ID
    MMVERSION vDriverVersion; // Driver Version
    CHAR      szPnameA[MAXPNAMELEN]; // Device Name (ASCII version)
    WCHAR     szPnameW[MAXPNAMELEN]; // Device Name (Unicode version)
    WORD      wTechnology;    // Device Type
    DWORD     dwSupport;      // Device capabilities
    CHAR      upcA[13];       // CD Unique Code (12 chars, ASCII version)
    WCHAR     upcW[13];       // CD Unique Code (Unicode version)
    WCHAR     idW[17];        // Device identity (Unicode version)
};

// Device Context
struct DeviceContext {
    MCIDEVICEID deviceId;      // Device ID used by MCI
    MCIDEVICEID requestId;     // Last requested ID
    BOOL  inUse;               // Is this slot in use?
    BOOL  isOpen;              // Is in OPEN state?
    BOOL  isCDA;               // Is this a cdaudio device (virtual CD)?
    DWORD openFlags;           // MCI_OPEN flags cache
    DWORD elementId;           // For storing element id (if MCI_OPEN_ELEMENT_ID is used)

    WCHAR element[64];         // element (mciSendString open element)
    WCHAR alias[64];           // Alias (mciSendString open alias)
    UINT  timeFormat;          // MCI_FORMAT_MSF/TMSF/MILLISECONDS
    HWND  notifyHwnd;          // MCI_NOTIFY target HWND (based on last request)

    // Thread info (for reference)
    DWORD hCreatorThreadId;
    DWORD hOpeningThreadId;
};

// --- Time conversion utils for devices ---

// Time conversion M:S:F(75fps) utils
inline DWORD MakeMSF(BYTE m, BYTE s, BYTE f) {
    return MCI_MAKE_MSF(m, s, f);
}
inline void SplitMSF(DWORD msf, BYTE* m, BYTE* s, BYTE* f) {
    if (m) *m = MCI_MSF_MINUTE(msf);
    if (s) *s = MCI_MSF_SECOND(msf);
    if (f) *f = MCI_MSF_FRAME(msf);
}

// Time conversion T:M:S:F utils
inline DWORD MakeTMSF(BYTE t, BYTE m, BYTE s, BYTE f) {
    return MCI_MAKE_TMSF(t, m, s, f);
}
inline void SplitTMSF(DWORD tmsf, BYTE* t, BYTE* m, BYTE* s, BYTE* f) {
    if (t) *t = MCI_TMSF_TRACK(tmsf);
    if (m) *m = MCI_TMSF_MINUTE(tmsf);
    if (s) *s = MCI_TMSF_SECOND(tmsf);
    if (f) *f = MCI_TMSF_FRAME(tmsf);
}

// Time conversion milliseconds <-> redbook frame(1/75s)
inline DWORD MillisecondsToFrames(DWORD ms) {
    return static_cast<DWORD>((static_cast<UINT64>(ms) * 75 + 999) / 1000); // Always round up
}
inline DWORD FramesToMilliseconds(DWORD frames) {
    return static_cast<DWORD>((static_cast<UINT64>(frames) * 1000 + 37) / 75); // 0.5 frame adjustment
}

namespace DeviceInfo {
    // Initialize/Shutdown
    BOOL Initialize(void);
    void Shutdown(void);

    // Create/Destroy (Device is created at OPEN, destroyed at CLOSE)
    BOOL CreateDevice(LPCWSTR aliasOpt, MCIDEVICEID* outDeviceId);
    BOOL Destroy(MCIDEVICEID deviceId);

    // Find
    DeviceContext* FindByDeviceID(MCIDEVICEID deviceId);
    DeviceContext* FindByAlias(LPCWSTR alias);
    DeviceContext* FindByElement(LPCWSTR element);

    // Set/Get Properties
    BOOL SetElement(MCIDEVICEID deviceId, LPCWSTR element);
    BOOL SetAlias(MCIDEVICEID deviceId, LPCWSTR alias);
    BOOL SetNotifyHWND(MCIDEVICEID deviceId, HWND hwnd);
    HWND GetNotifyHWND(MCIDEVICEID deviceId);

    BOOL SetDeviceTimeFormat(MCIDEVICEID deviceId, UINT timeFormat);
    UINT GetDeviceTimeFormat(MCIDEVICEID deviceId);

    // Status indicators
    BOOL SetOpenState(MCIDEVICEID deviceId, BOOL isOpen);
    BOOL GetOpenState(MCIDEVICEID deviceId, BOOL* outOpen);

    BOOL SetOpenFlags(MCIDEVICEID deviceId, DWORD flags);
    DWORD GetOpenFlags(MCIDEVICEID deviceId);

    BOOL SetElementId(MCIDEVICEID deviceId, DWORD elementId);
    DWORD GetElementId(MCIDEVICEID deviceId);

    // Utils
    // Get the first in-use CDA context
    DeviceContext* GetFirstDevice(void);

    // Get the default device info struct
    IDevice* GetDeviceInfo(void);
}
