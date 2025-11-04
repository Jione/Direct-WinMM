#include "DeviceInfo.h"
#include "PreferenceLoader.h"
#include <string.h> // memset, wcslen, lstrcpynW

namespace {
    // ===== Internal State =====
    static CRITICAL_SECTION gCs;
    static BOOL gInited = FALSE;

    static DeviceContext gTable[DT_MAX_DEVICES];
    static MCIDEVICEID   gDeviceID = 77; // Arbitrary start value (to avoid collisions)

    static IDevice       gDeviceInfo{ 0, };

    // ===== Internal Utils =====
    static void Lock() { EnterCriticalSection(&gCs); }
    static void Unlock() { LeaveCriticalSection(&gCs); }

    // Simple ID increment function
    static MCIDEVICEID AllocId() {
        MCI_OPEN_PARMSW mciOpenParms;

        // Create Dummy DeviceId
        ZeroMemory(&mciOpenParms, sizeof(mciOpenParms));
        mciOpenParms.lpstrAlias = L"cdaudio";
        mciOpenParms.lpstrDeviceType = (LPCWSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;

        if (!mciSendCommandW(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ALIAS, (DWORD_PTR)&mciOpenParms)) {
            gDeviceID = mciOpenParms.wDeviceID;
        }

        return gDeviceID;
    }

    static void ClearContext(DeviceContext* c) {
        if (!c) return;
        ZeroMemory(c, sizeof(*c));
        c->deviceId = 0;
        c->requestId = 0;
        c->inUse = FALSE;
        c->isOpen = FALSE;
        c->isCDA = FALSE;
        c->openFlags = 0;
        c->elementId = 0;
        c->element[0] = L'\0';
        c->alias[0] = L'\0';
        c->timeFormat = DT_DEFAULT_TIMEFORMAT; // Default MSF
        c->notifyHwnd = NULL;
        c->hCreatorThreadId = NULL;
        c->hOpeningThreadId = NULL;
    }

    static void FillDeviceInfo() {
        if (!gDeviceInfo.fillData) {
            gDeviceInfo.wMid = 2;  // MM_CREATIVE
            gDeviceInfo.wPid = 401;// MM_CREATIVE_AUX_CD
            gDeviceInfo.vDriverVersion = 0x10000; // v1.0000
            gDeviceInfo.wTechnology = AUXCAPS_CDAUDIO;
            gDeviceInfo.dwSupport = AUXCAPS_VOLUME | AUXCAPS_LRVOLUME;
            lstrcpynA(gDeviceInfo.szPnameA, "WinMM Virtual CD", MAXPNAMELEN);
            lstrcpynA(gDeviceInfo.upcA, "777777777777", 13);
            lstrcpynW(gDeviceInfo.szPnameW, L"WinMM Virtual CD", MAXPNAMELEN);
            lstrcpynW(gDeviceInfo.upcW, L"777777777777", 13);
            lstrcpynW(gDeviceInfo.idW, L"cafecafecafecafe", 17);
            gDeviceInfo.fillData = TRUE;
        }
    }
}

namespace DeviceInfo {
    BOOL Initialize(void) {
        if (gInited) return TRUE;
        InitializeCriticalSection(&gCs);
        Lock();
        PreferenceLoader::Initialize();
        FillDeviceInfo();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) ClearContext(&gTable[i]);
        Unlock();
        gInited = TRUE;
        return TRUE;
    }

    void Shutdown(void) {
        if (!gInited) return;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) ClearContext(&gTable[i]);
        Unlock();
        DeleteCriticalSection(&gCs);
        gInited = FALSE;
    }

    BOOL CreateDevice(LPCWSTR aliasOpt, MCIDEVICEID* outDeviceId) {
        if (!gInited || !outDeviceId) return FALSE;

        Lock();

        // Simulate only a single CDA
        DeviceContext* c;
        if (c = GetFirstDevice()) {
            if (outDeviceId) {
                *outDeviceId = c->deviceId;
            }
            Unlock(); // Unlock before returning
            return TRUE;
        }

        // Find an empty slot
        int idx = -1;
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (!gTable[i].inUse) { idx = i; break; }
        }
        if (idx < 0) { Unlock(); return FALSE; }

        c = &gTable[idx];
        ClearContext(c);

        c->inUse = TRUE;
        c->isCDA = TRUE;
        c->isOpen = TRUE;  // Created on successful OPEN
        c->deviceId = AllocId();
        c->timeFormat = DT_DEFAULT_TIMEFORMAT;

        if (aliasOpt && aliasOpt[0]) {
            lstrcpynW(c->alias, aliasOpt, ARRAYSIZE(c->alias));
        }
        else {
            // Default alias: "cdaudio"
            lstrcpynW(c->alias, L"cdaudio", ARRAYSIZE(c->alias));
        }

        // Thread info (for reference)
        c->hCreatorThreadId = GetCurrentThreadId();
        c->hOpeningThreadId = c->hCreatorThreadId;

        *outDeviceId = c->deviceId;
        Unlock();
        return TRUE;
    }

    BOOL Destroy(MCIDEVICEID deviceId) {
        if (!gInited) return FALSE;
        Lock();
        mciSendCommandW(deviceId, MCI_CLOSE, NULL, NULL);
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                ClearContext(&gTable[i]);
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    DeviceContext* FindByDeviceID(MCIDEVICEID deviceId) {
        if (!gInited) return NULL;
        DeviceContext* ret = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                ret = &gTable[i];
                break;
            }
        }
        Unlock();
        return ret;
    }

    DeviceContext* FindByAlias(LPCWSTR alias) {
        if (!gInited || !alias || !alias[0]) return NULL;
        DeviceContext* ret = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (!gTable[i].inUse) continue;
            if (lstrcmpiW(gTable[i].alias, alias) == 0) {
                ret = &gTable[i];
                break;
            }
        }
        Unlock();
        return ret;
    }

    DeviceContext* FindByElement(LPCWSTR element) {
        if (!gInited || !element || !element[0]) return NULL;
        DeviceContext* ret = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (!gTable[i].inUse) continue;
            if (lstrcmpiW(gTable[i].element, element) == 0) {
                ret = &gTable[i];
                break;
            }
        }
        Unlock();
        return ret;
    }

    DeviceContext* FindByElementID(DWORD elementId) {
        if (!gInited) return NULL;
        DeviceContext* ret = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].elementId == elementId) {
                ret = &gTable[i];
                break;
            }
        }
        Unlock();
        return ret;
    }

    BOOL SetAlias(MCIDEVICEID deviceId, LPCWSTR alias) {
        if (!gInited || !alias) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                lstrcpynW(gTable[i].alias, alias, ARRAYSIZE(gTable[i].alias));
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    BOOL SetElement(MCIDEVICEID deviceId, LPCWSTR element) {
        if (!gInited || !element) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                lstrcpynW(gTable[i].element, element, ARRAYSIZE(gTable[i].element));
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    BOOL SetNotifyHWND(MCIDEVICEID deviceId, HWND hwnd) {
        if (!gInited) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                gTable[i].notifyHwnd = hwnd;
                gTable[i].hOpeningThreadId = GetCurrentThreadId();
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    HWND GetNotifyHWND(MCIDEVICEID deviceId) {
        if (!gInited) return NULL;
        HWND h = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                h = gTable[i].notifyHwnd;
                break;
            }
        }
        Unlock();
        return h;
    }

    BOOL SetDeviceTimeFormat(MCIDEVICEID deviceId, UINT timeFormat) {
        if (!gInited) return FALSE;
        if (timeFormat != MCI_FORMAT_MSF &&
            timeFormat != MCI_FORMAT_TMSF &&
            timeFormat != MCI_FORMAT_MILLISECONDS)
            return FALSE;

        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                gTable[i].timeFormat = timeFormat;
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    UINT GetDeviceTimeFormat(MCIDEVICEID deviceId) {
        if (!gInited) return DT_DEFAULT_TIMEFORMAT;
        UINT tf = DT_DEFAULT_TIMEFORMAT;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                tf = gTable[i].timeFormat;
                break;
            }
        }
        Unlock();
        return tf;
    }

    BOOL SetOpenState(MCIDEVICEID deviceId, BOOL isOpen) {
        if (!gInited) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                gTable[i].isOpen = isOpen ? TRUE : FALSE;
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    BOOL GetOpenState(MCIDEVICEID deviceId, BOOL* outOpen) {
        if (!gInited || !outOpen) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                *outOpen = gTable[i].isOpen;
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    BOOL SetOpenFlags(MCIDEVICEID deviceId, DWORD flags) {
        if (!gInited) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                gTable[i].openFlags = flags;
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    DWORD GetOpenFlags(MCIDEVICEID deviceId) {
        if (!gInited) return 0;
        DWORD f = 0;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                f = gTable[i].openFlags;
                break;
            }
        }
        Unlock();
        return f;
    }

    BOOL SetElementId(MCIDEVICEID deviceId, DWORD elementId) {
        if (!gInited) return FALSE;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                gTable[i].elementId = elementId;
                Unlock();
                return TRUE;
            }
        }
        Unlock();
        return FALSE;
    }

    DWORD GetElementId(MCIDEVICEID deviceId) {
        if (!gInited) return 0;
        DWORD e = 0;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].deviceId == deviceId) {
                e = gTable[i].elementId;
                break;
            }
        }
        Unlock();
        return e;
    }

    DeviceContext* GetFirstDevice(void) {
        if (!gInited) return NULL;
        DeviceContext* ret = NULL;
        Lock();
        for (int i = 0; i < DT_MAX_DEVICES; ++i) {
            if (gTable[i].inUse && gTable[i].isCDA) { ret = &gTable[i]; break; }
        }
        Unlock();
        return ret;
    }

    IDevice* GetDeviceInfo(void) {
        if (!gInited) return NULL;
        Lock();
        FillDeviceInfo();
        Unlock();
        return &gDeviceInfo;
    }
}
