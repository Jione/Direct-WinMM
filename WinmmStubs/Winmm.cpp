#include "DeviceInfo.h"
#include "DeviceControl.h"
#include "NotifyManager.h"
#include "AudioEngineAdapter.h"
#include <wchar.h>

#define COPY_GENERIC_FIELDS_A2W(pA, pW)  do { if ((pA) && (pW)) { (pW)->dwCallback = (pA)->dwCallback; }} while(0)

// const WCHAR* const [] : Array pointers and the strings they point to are both constants
const WCHAR* const mciCommand[] = {
    L"open", L"play", L"stop", L"pause", L"resume", L"seek", L"close", L"set", L"status", L"info", L"sysinfo"
};
enum MCI_COMMANDS {
    CMD_OPEN = 0,
    CMD_PLAY,
    CMD_STOP,
    CMD_PAUSE,
    CMD_RESUME,
    CMD_SEEK,
    CMD_CLOSE,
    CMD_SET,
    CMD_STATUS,
    CMD_INFO,
    CMD_SYSINFO,
    CMD_UNKNOWN,
};
struct ParsedString {
    // Default device/alias
    WCHAR device[64]{ 0, };   // "cdaudio" or alias
    WCHAR alias[64]{ 0, };    // open alias x

    // Common parameters
    DWORD fdwCommand = 0;
    BOOL  notify = FALSE;
    BOOL  wait = FALSE;

    // MCI_PLAY
    int   fromTrack = 0;    DWORD fromMs = 0;
    int   toTrack = 0;    DWORD toMs = 0;

    // MCI_SET, MCI_STATUS
    DWORD item = 0;
    int   track = 0;
    DWORD audio = 0;
    DWORD timeFormat = 0;
};

// Helper to ASCII <-> Unicode conversion
static LPWSTR DupAtoW(LPCSTR s) {
    if (!s) return NULL;
    int cch = MultiByteToWideChar(CP_ACP, 0, s, -1, NULL, 0);
    if (cch <= 0) return NULL;
    LPWSTR w = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * cch);
    if (!w) return NULL;
    MultiByteToWideChar(CP_ACP, 0, s, -1, w, cch);
    return w;
}
static void CopyWtoA(LPCWSTR w, LPSTR a, UINT aCap) {
    if (!a || aCap == 0) return;
    if (!w) { a[0] = '\0'; return; }
    WideCharToMultiByte(CP_ACP, 0, w, -1, a, aCap, NULL, NULL);
    a[aCap - 1] = '\0';
}

// Helper to String Token Parsing
static BOOL ParseTimeToken(const WCHAR* tok, DWORD* outMs) {
    if (!tok || !outMs) return FALSE;
    const WCHAR* p1 = wcschr(tok, L':');
    if (!p1) { *outMs = (DWORD)wcstol(tok, NULL, 10); return TRUE; } // milliseconds number
    // mm:ss[:ff]
    int mm = (int)wcstol(tok, NULL, 10);
    int ss = (int)wcstol(p1 + 1, NULL, 10);
    const WCHAR* p2 = wcschr(p1 + 1, L':');
    int ff = 0;
    if (p2) ff = (int)wcstol(p2 + 1, NULL, 10);
    DWORD frames = (DWORD)(mm * 60 * 75 + ss * 75 + ff);
    *outMs = FramesToMilliseconds(frames);
    return TRUE;
}
static BOOL IsWordEq(const WCHAR* a, const WCHAR* b) { return (lstrcmpiW(a, b) == 0); }
static int  ToInt(const WCHAR* s) { if (!s) return 0; return (int)wcstol(s, NULL, 10); }

// Helper to pack parsed time/track into a DWORD for MCI structures
static DWORD PackStringTime(UINT tf, DWORD ms, int track) {
    if (tf == MCI_FORMAT_MILLISECONDS) return ms;

    DWORD frames = MillisecondsToFrames(ms);
    BYTE mm = (BYTE)(frames / (75 * 60));
    frames %= (75 * 60);
    BYTE ss = (BYTE)(frames / 75);
    BYTE ff = (BYTE)(frames % 75);

    if (tf == MCI_FORMAT_MSF) {
        return MakeMSF(mm, ss, ff);
    }
    else { // TMSF
        BYTE tr = (BYTE)((track > 0 && track <= 99) ? track : 1);
        return MakeTMSF(tr, mm, ss, ff);
    }
}

// Helper to format MCI_STATUS return values into a string
static void FormatTimeString(UINT tf, DWORD packedTime, LPWSTR out, UINT cch) {
    if (tf == MCI_FORMAT_MILLISECONDS) {
        wsprintfW(out, L"%lu", packedTime);
    }
    else if (tf == MCI_FORMAT_MSF) {
        wsprintfW(out, L"%02u:%02u:%02u",
            (UINT)MCI_MSF_MINUTE(packedTime),
            (UINT)MCI_MSF_SECOND(packedTime),
            (UINT)MCI_MSF_FRAME(packedTime));
    }
    else { // TMSF
        wsprintfW(out, L"%02u:%02u:%02u:%02u",
            (UINT)MCI_TMSF_TRACK(packedTime),
            (UINT)MCI_TMSF_MINUTE(packedTime),
            (UINT)MCI_TMSF_SECOND(packedTime),
            (UINT)MCI_TMSF_FRAME(packedTime));
    }
}


// mci command processing hub
// TRUE: Processed by mciCommandHub, FALSE: Relay to original
BOOL WINAPI mciCommandHub(MCIDEVICEID deviceId, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam, MCIERROR* ret) {
    if (!ret) {
        MCIERROR retn = MMSYSERR_NOERROR;
        ret = &retn;
    }
    else {
        *ret = MMSYSERR_NOERROR;
    }

    // OPEN is a normal case for deviceId==0 (identified by type/alias)
    if (uMsg == MCI_OPEN) {
        MCI_OPEN_PARMSW* p = (MCI_OPEN_PARMSW*)dwParam;
        if (!p) {
            *ret = MCIERR_MISSING_PARAMETER;
            return TRUE;
        }

        // Check if cdaudio type
        BOOL wantCda = FALSE;
        LPCWSTR typeName = NULL;

        if (fdwCommand & MCI_OPEN_TYPE_ID) {
            wantCda = (DWORD)p->lpstrDeviceType == MCI_DEVTYPE_CD_AUDIO;
        }
        else if (fdwCommand & MCI_OPEN_TYPE) {
            typeName = p->lpstrDeviceType;
            if (typeName ? (lstrcmpiW(typeName, L"cdaudio") == 0) : NULL) {
                wantCda = TRUE;
            }
        }

        // Extract alias
        LPCWSTR aliasOpt = NULL;
        if (fdwCommand & MCI_OPEN_ALIAS) aliasOpt = p->lpstrAlias;

        // Register device
        if (wantCda) {
            DeviceContext* ctx = DeviceInfo::GetFirstDevice();
            if (!ctx) {
                MCIDEVICEID newId = 0;
                if (!DeviceInfo::Initialize()) {
                    *ret = MCIERR_INTERNAL;
                    return TRUE;
                }

                // If alias exists, check first
                if (aliasOpt && aliasOpt[0]) {
                    ctx = DeviceInfo::FindByAlias(aliasOpt);
                }

                // If not found, create new
                if (ctx && ctx->isOpen) { newId = ctx->deviceId; }
                else if (!DeviceInfo::CreateDevice(aliasOpt, &newId)) {
                    *ret = MCIERR_INTERNAL;
                    return TRUE;
                }

                ctx = DeviceInfo::FindByDeviceID(newId);
                ctx->isCDA = TRUE;
                p->wDeviceID = newId;

                // Cache callback/flags
                DeviceInfo::SetNotifyHWND(ctx->deviceId, (HWND)p->dwCallback);
                DeviceInfo::SetOpenFlags(ctx->deviceId, fdwCommand);
                if (aliasOpt && aliasOpt[0]) { DeviceInfo::SetAlias(newId, aliasOpt); }

                // Delegate to Controller
                ctx->requestId = deviceId;
                *ret = Device::Open(ctx, fdwCommand, dwParam);

                return TRUE;
            }
            else {
                p->wDeviceID = ctx->deviceId;
                ctx->requestId = deviceId;
                DeviceInfo::SetNotifyHWND(ctx->deviceId, (HWND)p->dwCallback);
                if (fdwCommand & MCI_NOTIFY) {
                    NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
                }
                return TRUE;
            }
        }

        // If not cdaudio, forward to original
        return FALSE;
    }
    // Other than OPEN: Determine CDA status by deviceId
    else {
        BOOL isUnique = FALSE;
        DeviceContext* ctx = DeviceInfo::FindByDeviceID(deviceId);
        if (deviceId == 0 || deviceId == MCI_ALL_DEVICE_ID) {
            ctx = DeviceInfo::GetFirstDevice();
            if (!ctx) {
                MCIDEVICEID newId = 0;
                if (!(DeviceInfo::Initialize() && DeviceInfo::CreateDevice(NULL, &newId))) {
                    return FALSE;
                }

                ctx = DeviceInfo::FindByDeviceID(newId);
                ctx->isCDA = TRUE;
                ctx->timeFormat = MCI_FORMAT_TMSF;  // Time format of the unique device is assumed to be tmsf.
                isUnique = TRUE;
            }
        }
        else if (!ctx || !ctx->isCDA) {
            return FALSE;
        }

        // Update callback hwnd
        ctx->requestId = deviceId;
        if (dwParam) {
            MCI_GENERIC_PARMS* p = (MCI_GENERIC_PARMS*)dwParam;
            DeviceInfo::SetNotifyHWND(ctx->deviceId, (HWND)p->dwCallback);
        }

        // Common dispatch
        switch (uMsg) {
        case MCI_PLAY:   *ret = Device::Play(ctx, fdwCommand, dwParam); break;
        case MCI_STOP:   *ret = Device::Stop(ctx, fdwCommand, dwParam); break;
        case MCI_PAUSE:  *ret = Device::Pause(ctx, fdwCommand, dwParam); break;
        case MCI_RESUME: *ret = Device::Resume(ctx, fdwCommand, dwParam); break;
        case MCI_SEEK:   *ret = Device::Seek(ctx, fdwCommand, dwParam); break;
        case MCI_SET:    *ret = Device::Set(ctx, fdwCommand, dwParam); break;
        case MCI_STATUS: *ret = Device::Status(ctx, fdwCommand, dwParam); break;
        case MCI_INFO:   *ret = Device::Info(ctx, fdwCommand, dwParam); break;
        case MCI_SYSINFO:*ret = Device::SysInfo(ctx, fdwCommand, dwParam); break;
        case MCI_CLOSE: {
            *ret = Device::Close(ctx, fdwCommand, dwParam);
            DeviceInfo::Destroy(ctx->deviceId);
            break;
        }
        default: // Relay unknown or unimplemented commands
            return FALSE;
        }

        // If it was a unique command (DeviceId 0 or -1), execute original as well
        return !isUnique;
    }

    return FALSE;
}

// String parsing and return
// TRUE: Processed by mciStringHub, FALSE: Relay to original
BOOL WINAPI mciStringHub(LPCWSTR lpstrCommand, LPWSTR lpstrReturn, UINT uReturnLength, HWND hCallback, MCIERROR* ret) {
    if (!ret) {
        MCIERROR retn = MMSYSERR_NOERROR;
        ret = &retn;
    }
    else {
        *ret = MMSYSERR_NOERROR;
    }
    if (!lpstrCommand || !lpstrCommand[0]) {
        *ret = MCIERR_MISSING_PARAMETER;
        return TRUE;
    }

    // Simple tokenizer (space-delimited)
    WCHAR buf[512]; lstrcpynW(buf, lpstrCommand, 512);
    WCHAR* checkText = NULL;
    MCI_COMMANDS cmd = CMD_UNKNOWN;
    WCHAR* token = wcstok(buf, L" \t\r\n", &checkText);

    // If parsing fails, relay to original
    if (!token) return FALSE;

    // Parse first command
    for (size_t i = 0; i < CMD_UNKNOWN; ++i) {
        if (lstrcmpiW(mciCommand[i], token) == 0) {
            cmd = (MCI_COMMANDS)i;
            break;
        }
    }

    // If unsupported command, relay to original
    if (cmd == CMD_UNKNOWN) return FALSE;

    ParsedString pStr;
    WCHAR* w = NULL;
    while ((token = wcstok(NULL, L" \t\r\n", &checkText)) != NULL) {
        // Common flags, MCI_OPEN, MCI_PLAY, MCI_STOP, MCI_PAUSE, MCI_RESUME, MCI_SEEK, MCI_CLOSE
        if (IsWordEq(token, L"alias")) {
            w = wcstok(NULL, L" \t\r\n", &checkText);
            if (w) {
                lstrcpynW(pStr.alias, w, 64);
                if (cmd == CMD_OPEN) {
                    pStr.fdwCommand |= MCI_OPEN_ALIAS;
                }
                else if (!DeviceInfo::FindByAlias(pStr.alias))
                    return FALSE;
            }
        }
        else if (IsWordEq(token, L"type")) {
            w = wcstok(NULL, L" \t\r\n", &checkText);
            if (w) {
                lstrcpynW(pStr.device, w, 64);
                if (!IsWordEq(token, L"cdaudio"))
                    return FALSE;
            }
        }
        else if (IsWordEq(token, L"notify"))
            pStr.fdwCommand |= MCI_NOTIFY;
        else if (IsWordEq(token, L"wait"))
            pStr.fdwCommand |= MCI_WAIT;
        else if (IsWordEq(token, L"from")) {
            w = wcstok(NULL, L" \t\r\n", &checkText);
            if (w) {
                pStr.fdwCommand |= MCI_FROM;
                if (IsWordEq(w, L"track")) {
                    // "from track N"
                    w = wcstok(NULL, L" \t\r\n", &checkText);
                    if (w) {
                        pStr.fromMs = 0;
                        pStr.fromTrack = ToInt(w);
                    }
                }
                else {
                    // "from [time]"
                    ParseTimeToken(w, &pStr.fromMs);
                    pStr.fromTrack = ToInt(w); // For TMSF
                }
            }
        }
        else if (IsWordEq(token, L"to")) {
            w = wcstok(NULL, L" \t\r\n", &checkText);
            if (w) {
                if (IsWordEq(w, L"start"))
                    pStr.fdwCommand |= MCI_SEEK_TO_START;
                else if (IsWordEq(w, L"end"))
                    pStr.fdwCommand |= MCI_SEEK_TO_END;
                else {
                    pStr.fdwCommand |= MCI_TO;
                    if (IsWordEq(w, L"track")) {
                        // "to track N"
                        w = wcstok(NULL, L" \t\r\n", &checkText);
                        if (w) {
                            pStr.toMs = 0;
                            pStr.toTrack = ToInt(w);
                        }
                    }
                    else {
                        // "to [time]"
                        ParseTimeToken(w, &pStr.toMs);
                        pStr.toTrack = ToInt(w); // For TMSF
                    }
                }
            }
        }
        else if (IsWordEq(token, L"track")) {
            // This is for MCI_STATUS track N length/position
            pStr.fdwCommand |= MCI_TRACK;
            w = wcstok(NULL, L" \t\r\n", &checkText);
            if (w) pStr.track = ToInt(w);
        }
        else if (cmd < CMD_SET) {
            if (!pStr.device[0]) { lstrcpynW(pStr.device, token, 64); } // First token: usually device/alias
            continue;
        }

        // MCI_STATUS
        else if (cmd == CMD_STATUS) {
            if (IsWordEq(token, L"time") || IsWordEq(token, L"time_format")) {
                if (!IsWordEq(token, L"time_format")) {
                    w = wcstok(NULL, L" \t\r\n", &checkText); // format keyword
                    if (!w || !IsWordEq(w, L"format"))
                        continue;
                }
                pStr.item = MCI_STATUS_TIME_FORMAT;
            }
            else if (IsWordEq(token, L"length"))
                pStr.item = MCI_STATUS_LENGTH;
            else if (IsWordEq(token, L"number") || IsWordEq(token, L"number_of_tracks"))
                pStr.item = MCI_STATUS_NUMBER_OF_TRACKS;
            else if (IsWordEq(token, L"position"))
                pStr.item = MCI_STATUS_POSITION;
            else if (IsWordEq(token, L"current") || IsWordEq(token, L"current_track"))
                pStr.item = MCI_STATUS_CURRENT_TRACK;
            else if (IsWordEq(token, L"mode"))
                pStr.item = MCI_STATUS_MODE;
            else if (IsWordEq(token, L"present") || IsWordEq(token, L"media_present"))
                pStr.item = MCI_STATUS_MEDIA_PRESENT;
            else if (!pStr.device[0]) { lstrcpynW(pStr.device, token, 64); }
        }

        // MCI_SET
        else if (cmd == CMD_SET) {
            if (IsWordEq(token, L"time") || IsWordEq(token, L"time_format")) {
                if (!IsWordEq(token, L"time_format")) {
                    w = wcstok(NULL, L" \t\r\n", &checkText); // format keyword
                    if (!w || !IsWordEq(w, L"format"))
                        continue;
                }
                pStr.fdwCommand |= MCI_SET_TIME_FORMAT;

                w = wcstok(NULL, L" \t\r\n", &checkText);
                if (w && IsWordEq(w, L"tmsf"))
                    pStr.timeFormat = MCI_FORMAT_TMSF;
                else if (w && IsWordEq(w, L"msf"))
                    pStr.timeFormat = MCI_FORMAT_MSF;
                else if (w && (IsWordEq(w, L"ms") || IsWordEq(w, L"milliseconds")))
                    pStr.timeFormat = MCI_FORMAT_MILLISECONDS;
            }
            else if (IsWordEq(token, L"audio")) {
                w = wcstok(NULL, L" \t\r\n", &checkText); // all, left, right keyword
                if (!w) continue;
                else if (IsWordEq(w, L"all"))
                    pStr.audio = MCI_SET_AUDIO_ALL;
                else if (IsWordEq(w, L"left"))
                    pStr.audio = MCI_SET_AUDIO_LEFT;
                else if (IsWordEq(w, L"right"))
                    pStr.audio = MCI_SET_AUDIO_RIGHT;
                else continue;

                w = wcstok(NULL, L" \t\r\n", &checkText); // on, off keyword
                if (!w) continue;
                else if (IsWordEq(w, L"on"))
                    pStr.fdwCommand |= MCI_SET_ON;
                else if (IsWordEq(w, L"off"))
                    pStr.fdwCommand |= MCI_SET_OFF;
            }
            else if (IsWordEq(token, L"door")) {
                w = wcstok(NULL, L" \t\r\n", &checkText); // open, closed keyword
                if (!w) continue;
                else if (IsWordEq(w, L"open"))
                    pStr.fdwCommand |= MCI_SET_DOOR_OPEN;
                else if (IsWordEq(w, L"closed"))
                    pStr.fdwCommand |= MCI_SET_DOOR_CLOSED;
            }
            else if (!pStr.device[0]) { lstrcpynW(pStr.device, token, 64); }
        }

        // MCI_INFO
        else if (cmd == CMD_INFO) {
            if (IsWordEq(token, L"product")) {
                pStr.fdwCommand |= MCI_INFO_PRODUCT;
            }
            else if (IsWordEq(token, L"info")) continue;
            else if (IsWordEq(token, L"upc"))
                pStr.fdwCommand |= MCI_INFO_MEDIA_UPC;
            else if (IsWordEq(token, L"identity"))
                pStr.fdwCommand |= MCI_INFO_MEDIA_IDENTITY;
            else if (!pStr.device[0]) { lstrcpynW(pStr.device, token, 64); }
        }

        // MCI_SYSINFO
        else if (cmd == CMD_SYSINFO) {
            if (IsWordEq(token, L"installname"))
                pStr.fdwCommand |= MCI_SYSINFO_INSTALLNAME;
            else if (IsWordEq(token, L"quantity"))
                pStr.fdwCommand |= MCI_SYSINFO_QUANTITY;
            else if (IsWordEq(token, L"open"))
                pStr.fdwCommand |= MCI_SYSINFO_OPEN;
            else if (IsWordEq(token, L"name"))
                pStr.fdwCommand |= MCI_SYSINFO_NAME;
            else if (!pStr.device[0]) { lstrcpynW(pStr.device, token, 64); }
        }
    }

    // If device name is not cdaudio and alias is empty, relay to original
    // FIX: Modify search criteria for CD Audio or alias.
    if (!IsWordEq(pStr.device, L"cdaudio") && !pStr.alias[0]) {
        if (!DeviceInfo::FindByAlias(pStr.device)) return FALSE;
        else lstrcpynW(pStr.alias, pStr.device, 64);
    }

    // If MCI_OPEN, create new device or call existing
    DeviceContext* ctx;
    if (cmd == CMD_OPEN) {
        ctx = DeviceInfo::GetFirstDevice();
        if (!ctx) {
            MCIDEVICEID newId = 0;
            if (!DeviceInfo::Initialize()) {
                *ret = MCIERR_INTERNAL;
                return TRUE;
            }

            // If alias exists, check first
            if (pStr.alias[0]) {
                ctx = DeviceInfo::FindByAlias(pStr.alias);
            }

            // If not found, create new
            if (ctx && ctx->isOpen) { newId = ctx->deviceId; }
            else if (!DeviceInfo::CreateDevice(pStr.alias, &newId)) {
                *ret = MCIERR_INTERNAL;
                return TRUE;
            }

            DeviceInfo::SetAlias(newId, pStr.alias);
            ctx = DeviceInfo::FindByDeviceID(newId);
            ctx->isCDA = TRUE;

            // Cache callback/flags
            DeviceInfo::SetNotifyHWND(ctx->deviceId, hCallback);
            DeviceInfo::SetOpenFlags(ctx->deviceId, pStr.fdwCommand);
            if (pStr.alias[0]) { DeviceInfo::SetAlias(newId, pStr.alias); }

            // Delegate to Controller
            ctx->requestId = 0;
            *ret = Device::Open(ctx, pStr.fdwCommand, NULL);
        }
        else {
            ctx->requestId = 0;
            DeviceInfo::SetNotifyHWND(ctx->deviceId, hCallback);
            if (pStr.fdwCommand & MCI_NOTIFY) {
                NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
            }
        }
        // [!!] ADDED: Return device ID in lpstrReturn for string OPEN
        if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
            wsprintfW(lpstrReturn, L"%u", ctx->deviceId);
        }
        return TRUE;
    }

    // Other than OPEN (alias check already done above)
    else {
        // If alias exists, check first
        if (pStr.alias[0]) { ctx = DeviceInfo::FindByAlias(pStr.alias); }
        else { ctx = DeviceInfo::GetFirstDevice(); }

        // If lookup fails, create new device
        if (!ctx) {
            MCIDEVICEID newId = 0;
            if (!(DeviceInfo::Initialize() && DeviceInfo::CreateDevice(NULL, &newId))) {
                return FALSE;
            }

            ctx = DeviceInfo::FindByDeviceID(newId);
            ctx->isCDA = TRUE;
            ctx->timeFormat = MCI_FORMAT_TMSF; // Time format of the unique device is assumed to be tmsf.
        }

        // Update callback hwnd
        ctx->requestId = 0;
        DeviceInfo::SetNotifyHWND(ctx->deviceId, hCallback);

        // Common dispatch
        switch (cmd) {
        case CMD_PLAY: {
            MCI_PLAY_PARMS dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            if (pStr.fdwCommand & MCI_FROM) {
                dwParam.dwFrom = PackStringTime(ctx->timeFormat, pStr.fromMs, pStr.fromTrack);
            }
            if (pStr.fdwCommand & MCI_TO) {
                dwParam.dwTo = PackStringTime(ctx->timeFormat, pStr.toMs, pStr.toTrack);
            }

            *ret = Device::Play(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);
            break;
        }
        case CMD_STOP: { *ret = Device::Stop(ctx, pStr.fdwCommand, NULL); break; }
        case CMD_PAUSE: { *ret = Device::Pause(ctx, pStr.fdwCommand, NULL); break; }
        case CMD_RESUME: { *ret = Device::Resume(ctx, pStr.fdwCommand, NULL); break; }
        case CMD_SEEK: {
            MCI_SEEK_PARMS dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            if (pStr.fdwCommand & MCI_TO) {
                dwParam.dwTo = PackStringTime(ctx->timeFormat, pStr.toMs, pStr.toTrack);
            }
            *ret = Device::Seek(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);
            break;
        }
        case CMD_SET: {
            MCI_SET_PARMS dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            dwParam.dwAudio = pStr.audio;
            dwParam.dwTimeFormat = pStr.timeFormat;
            *ret = Device::Set(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);
            break;
        }
        case CMD_STATUS: {
            MCI_STATUS_PARMS dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            dwParam.dwItem = pStr.item;
            dwParam.dwTrack = pStr.track;

            *ret = Device::Status(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);

            if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
                switch (pStr.item) {
                case MCI_STATUS_LENGTH:
                case MCI_STATUS_POSITION:
                    FormatTimeString(ctx->timeFormat, dwParam.dwReturn, lpstrReturn, uReturnLength);
                    break;
                case MCI_STATUS_MODE:
                    switch (dwParam.dwReturn) {
                    case MCI_MODE_PLAY:   lstrcpynW(lpstrReturn, L"playing", uReturnLength); break;
                    case MCI_MODE_PAUSE:  lstrcpynW(lpstrReturn, L"paused", uReturnLength); break;
                    case MCI_MODE_STOP:   lstrcpynW(lpstrReturn, L"stopped", uReturnLength); break;
                    default:              lstrcpynW(lpstrReturn, L"unknown", uReturnLength); break;
                    }
                    break;
                case MCI_STATUS_MEDIA_PRESENT:
                    lstrcpynW(lpstrReturn, (dwParam.dwReturn ? L"TRUE" : L"FALSE"), uReturnLength);
                    break;
                case MCI_STATUS_TIME_FORMAT:
                    switch (dwParam.dwReturn) {
                    case MCI_FORMAT_MILLISECONDS: lstrcpynW(lpstrReturn, L"milliseconds", uReturnLength); break;
                    case MCI_FORMAT_MSF:          lstrcpynW(lpstrReturn, L"msf", uReturnLength); break;
                    case MCI_FORMAT_TMSF:         lstrcpynW(lpstrReturn, L"tmsf", uReturnLength); break;
                    default:                      lstrcpynW(lpstrReturn, L"unknown", uReturnLength); break;
                    }
                    break;
                case MCI_STATUS_NUMBER_OF_TRACKS:
                case MCI_STATUS_CURRENT_TRACK:
                default:
                    wsprintfW(lpstrReturn, L"%lu", dwParam.dwReturn);
                    break;
                }
            }
            break;
        }
        case CMD_INFO: {
            MCI_INFO_PARMSW dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            WCHAR str[128]{ 0, };
            dwParam.lpstrReturn = str;
            dwParam.dwRetSize = ARRAYSIZE(str);

            *ret = Device::Info(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);

            if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
                lstrcpynW(lpstrReturn, str, uReturnLength);
            }
            break;
        }
        case CMD_SYSINFO: {
            MCI_SYSINFO_PARMSW dwParam{ 0, };
            dwParam.dwCallback = (DWORD_PTR)hCallback;
            WCHAR str[128]{ 0, };
            dwParam.lpstrReturn = str;
            dwParam.dwRetSize = ARRAYSIZE(str);

            // Set params from parsed flags
            dwParam.wDeviceType = MCI_DEVTYPE_CD_AUDIO; // Assume cdaudio for all string sysinfo
            if (pStr.fdwCommand & (MCI_SYSINFO_NAME | MCI_SYSINFO_OPEN)) {
                // The parser is flawed, 'name cdaudio 1' isn't parsed right.
                // We'll just assume #1.
                dwParam.dwNumber = 1;
            }

            *ret = Device::SysInfo(ctx, pStr.fdwCommand, (DWORD_PTR)&dwParam);

            if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
                lstrcpynW(lpstrReturn, str, uReturnLength);
            }
            break;
        }
        case CMD_CLOSE: {
            *ret = Device::Close(ctx, pStr.fdwCommand, NULL);
            DeviceInfo::Destroy(ctx->deviceId);
            break;
        }
        }
    }
    return TRUE;
}

// mciSendCommandW connection function
MCIERROR WINAPI mciSendCommandWStubs(MCIDEVICEID deviceId, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam) {
    MCIERROR ret;
#ifdef _DEBUG
    wchar_t errText[256]{ 0, };
#endif
    dprintf(L"mciSendCommandW deviceId=0x%04X uMsg=0x%04X fdwCommand=0x%08X, dwParam=0x%08X", deviceId, uMsg, fdwCommand, dwParam);
    if (!mciCommandHub(deviceId, uMsg, fdwCommand, dwParam, &ret)) {
        ret = mciSendCommandW(deviceId, uMsg, fdwCommand, dwParam);
    }
    dprintf(L"Return=%s", mciGetErrorStringW(ret, errText, 256) ? errText : L"Unknown error");
    return ret;
}

// mciSendCommandA connection function
MCIERROR WINAPI mciSendCommandAStubs(MCIDEVICEID deviceId, UINT uMsg, DWORD fdwCommand, DWORD_PTR dwParam) {
    // Most structs not containing strings have identical A/W layout -> can pass directly.
    // However, commands with string fields require explicit conversion:
    //  - MCI_OPEN: (lpstrDeviceType / lpstrElementName / lpstrAlias)
    //  - MCI_SYSINFO: (lpstrReturn, dwRetSize)
    //  - MCI_INFO: (lpstrReturn, dwRetSize)
    // Others are passed to mciInterCommandW (either not cdaudio commands or fields are identical).

    MCIERROR ret;
#ifdef _DEBUG
    wchar_t errText[256]{ 0, };
#endif
    dprintf(L"mciSendCommandW deviceId=0x%04X uMsg=0x%04X fdwCommand=0x%08X, dwParam=0x%08X", deviceId, uMsg, fdwCommand, dwParam);

    switch (uMsg) {
    case MCI_OPEN: {
        const MCI_OPEN_PARMSA* pA = (const MCI_OPEN_PARMSA*)dwParam;
        if (!pA) {
            ret = MCIERR_MISSING_PARAMETER;
            break;
        }

        // Convert to W struct
        MCI_OPEN_PARMSW pW; ZeroMemory(&pW, sizeof(pW));
        COPY_GENERIC_FIELDS_A2W(pA, &pW);

        // String fields may be valid depending on flags.
        LPWSTR wType = NULL, wElem = NULL, wAlias = NULL;
        if (fdwCommand & MCI_OPEN_TYPE_ID) {
            wType = (LPWSTR)(pA->lpstrDeviceType);
        }
        else if (fdwCommand & MCI_OPEN_TYPE) {
            // String type
            wType = DupAtoW(pA->lpstrDeviceType);
        }
        if (fdwCommand & MCI_OPEN_ELEMENT_ID) {
            wElem = (LPWSTR)pA->lpstrElementName;
        }
        else if (fdwCommand & MCI_OPEN_ELEMENT) {
            wElem = DupAtoW(pA->lpstrElementName);
        }
        if (fdwCommand & MCI_OPEN_ALIAS) {
            wAlias = DupAtoW(pA->lpstrAlias);
        }
        pW.lpstrDeviceType = wType;
        pW.lpstrElementName = wElem;
        pW.lpstrAlias = wAlias;

        if (mciCommandHub(deviceId, uMsg, fdwCommand, (DWORD_PTR)&pW, &ret)) {
            MCI_OPEN_PARMSA* pOut = (MCI_OPEN_PARMSA*)dwParam;
            if (pOut) pOut->wDeviceID = pW.wDeviceID;
            break;
        }
        ret = mciSendCommandA(deviceId, uMsg, fdwCommand, dwParam);
        break;
    }
    case MCI_SYSINFO: {
        MCI_SYSINFO_PARMSA* pA = (MCI_SYSINFO_PARMSA*)dwParam;
        if (!pA) {
            ret = MCIERR_MISSING_PARAMETER;
            break;
        }

        MCI_SYSINFO_PARMSW pW; ZeroMemory(&pW, sizeof(pW));
        COPY_GENERIC_FIELDS_A2W(pA, &pW);
        pW.dwRetSize = pA->dwRetSize; // Assumed to be char count (capacity)

        // Wide temp buffer (if dwRetSize>=1)
        LPWSTR wbuf = NULL;
        if (pA->lpstrReturn && pA->dwRetSize > 0) {
            wbuf = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * pA->dwRetSize);
            if (!wbuf) {
                ret = MCIERR_OUT_OF_MEMORY;
                break;
            }
            wbuf[0] = L'\0';
        }
        pW.lpstrReturn = wbuf;
        pW.dwNumber = pA->dwNumber;
        pW.wDeviceType = pA->wDeviceType;


        if (mciCommandHub(deviceId, uMsg, fdwCommand, (DWORD_PTR)&pW, &ret)) {
            if (pA->lpstrReturn && pA->dwRetSize > 0 && wbuf) {
                CopyWtoA(wbuf, pA->lpstrReturn, (UINT)pA->dwRetSize);
            }
        }
        else ret = mciSendCommandA(deviceId, uMsg, fdwCommand, dwParam);

        if (wbuf) LocalFree(wbuf);
        break;
    }
    case MCI_INFO: {
        MCI_INFO_PARMSA* pA = (MCI_INFO_PARMSA*)dwParam;
        if (!pA) {
            ret = MCIERR_MISSING_PARAMETER;
            break;
        }

        MCI_INFO_PARMSW pW; ZeroMemory(&pW, sizeof(pW));
        COPY_GENERIC_FIELDS_A2W(pA, &pW);
        pW.dwRetSize = pA->dwRetSize;

        LPWSTR wbuf = NULL;
        if (pA->lpstrReturn && pA->dwRetSize > 0) {
            wbuf = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(WCHAR) * pA->dwRetSize);
            if (!wbuf) {
                ret = MCIERR_OUT_OF_MEMORY;
                break;
            }
            wbuf[0] = L'\0';
        }
        pW.lpstrReturn = wbuf;

        if (mciCommandHub(deviceId, uMsg, fdwCommand, (DWORD_PTR)&pW, &ret)) {
            if (pA->lpstrReturn && pA->dwRetSize > 0 && wbuf) {
                CopyWtoA(wbuf, pA->lpstrReturn, (UINT)pA->dwRetSize);
            }
        }
        else ret = mciSendCommandA(deviceId, uMsg, fdwCommand, dwParam);

        if (wbuf) LocalFree(wbuf);
        break;
    }
    // Others (PLAY/STOP/PAUSE/RESUME/SEEK/SET/STATUS etc.)
    // have no string pointers in their structs, so A/W layout is identical.
    case MCI_PLAY:
    case MCI_STOP:
    case MCI_PAUSE:
    case MCI_RESUME:
    case MCI_SEEK:
    case MCI_SET:
    case MCI_STATUS:
    case MCI_CLOSE:
        if (mciCommandHub(deviceId, uMsg, fdwCommand, dwParam, &ret)) { break; }
    default:
        ret = mciSendCommandA(deviceId, uMsg, fdwCommand, dwParam);
    }
    dprintf(L"Return=%s", mciGetErrorStringW(ret, errText, 256) ? errText : L"Unknown error");
    return ret;
}

// mciSendStringW connection function
MCIERROR WINAPI mciSendStringWStubs(LPCWSTR lpstrCommand, LPWSTR lpstrReturn, UINT uReturnLength, HWND hCallback) {
    MCIERROR ret;
#ifdef _DEBUG
    wchar_t errText[256]{ 0, };
#endif
    dprintf(L"mciSendStringW lpstrCommand=%s lpstrReturn=0x%08X uReturnLength=%lu, hCallback=0x%08X", lpstrCommand, lpstrReturn, uReturnLength, hCallback);
    if (!mciStringHub(lpstrCommand, lpstrReturn, uReturnLength, hCallback, &ret)) {
        ret = mciSendStringW(lpstrCommand, lpstrReturn, uReturnLength, hCallback);

    }
    dprintf(L"lpstrReturn=%s\nReturn=%s", lpstrReturn, mciGetErrorStringW(ret, errText, 256) ? errText : L"Unknown error");
    return ret;
}


MCIERROR WINAPI mciSendStringAStubs(LPCSTR lpstrCommand, LPSTR lpstrReturn, UINT uReturnLength, HWND hCallback) {
    MCIERROR ret;
#ifdef _DEBUG
    char errText[256]{ 0, };
#endif

    dprintf("mciSendStringA lpstrCommand=%s lpstrReturn=0x%08X uReturnLength=%lu, hCallback=0x%08X", lpstrCommand, lpstrReturn, uReturnLength, hCallback);

    WCHAR wCmd[1024]; wCmd[0] = 0;
    WCHAR wRet[1024]; wRet[0] = 0;
    if (lpstrCommand) MultiByteToWideChar(CP_ACP, 0, lpstrCommand, -1, wCmd, ARRAYSIZE(wCmd));

    if (!mciStringHub(wCmd, (lpstrReturn ? wRet : NULL), (lpstrReturn ? ARRAYSIZE(wRet) : 0), hCallback, &ret)) {
        ret = mciSendStringA(lpstrCommand, lpstrReturn, uReturnLength, hCallback);
    }
    else if (lpstrReturn && uReturnLength > 0) {
        if (wRet[0]) { WideCharToMultiByte(CP_ACP, 0, wRet, -1, lpstrReturn, uReturnLength, NULL, NULL); }
        else { lpstrReturn[0] = '\0'; }
    }
    dprintf("lpstrReturn=%s\nReturn=%s", lpstrReturn, mciGetErrorStringA(ret, errText, 256) ? errText : "Unknown error");
    return ret;
}

UINT WINAPI auxGetNumDevsStubs() {
    dprintf(L"auxGetNumDevsStubs Return=1 (Fake device)");
    return 1;
}

MMRESULT WINAPI auxGetDevCapsAStubs(UINT_PTR uDeviceID, LPAUXCAPSA pac, UINT cbac) {
    dprintf(L"auxGetDevCapsAStubs uDeviceID=0x%04X", uDeviceID);
    if (!DeviceInfo::Initialize()) {
        dprintf(L"Return=MCIERR_INTERNAL (DeviceInfo Init Failed)");
        return MCIERR_INTERNAL;
    }
    if (!pac || cbac < sizeof(AUXCAPSA)) {
        dprintf(L"Return=MMSYSERR_INVALPARAM");
        return MMSYSERR_INVALPARAM;
    }

    ZeroMemory(pac, sizeof(AUXCAPSA));
    auto devInfo = DeviceInfo::GetDeviceInfo();

    pac->wMid = devInfo->wMid;
    pac->wPid = devInfo->wPid;
    pac->vDriverVersion = devInfo->vDriverVersion;
    lstrcpynA(pac->szPname, devInfo->szPnameA, sizeof(pac->szPname));
    pac->wTechnology = devInfo->wTechnology;
    pac->dwSupport = devInfo->dwSupport;
    dprintf(L"Return=MMSYSERR_NOERROR");
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI auxGetDevCapsWStubs(UINT_PTR uDeviceID, LPAUXCAPSW pac, UINT cbac) {
    dprintf(L"auxGetDevCapsAStubs uDeviceID=0x%04X", uDeviceID);
    if (!DeviceInfo::Initialize()) {
        dprintf(L"Return=MCIERR_INTERNAL (DeviceInfo Init Failed)");
        return MCIERR_INTERNAL;
    }
    if (!pac || cbac < sizeof(AUXCAPSW)) {
        dprintf(L"Return=MMSYSERR_INVALPARAM");
        return MMSYSERR_INVALPARAM;
    }

    ZeroMemory(pac, sizeof(AUXCAPSW));
    auto devInfo = DeviceInfo::GetDeviceInfo();

    pac->wMid = devInfo->wMid;
    pac->wPid = devInfo->wPid;
    pac->vDriverVersion = devInfo->vDriverVersion;
    lstrcpynW(pac->szPname, devInfo->szPnameW, ARRAYSIZE(pac->szPname));
    pac->wTechnology = devInfo->wTechnology;
    pac->dwSupport = devInfo->dwSupport;
    dprintf(L"Return=MMSYSERR_NOERROR");
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI auxSetVolumeStubs(UINT uDeviceID, DWORD dwVolume) {
    dprintf(L"auxSetVolumeStubs uDeviceID=0x%04X, dwVolume=0x%08X", uDeviceID, dwVolume);

    // Extract Left/Right WORD values
    WORD leftWord = LOWORD(dwVolume);
    WORD rightWord = HIWORD(dwVolume);
    float volLeft = (float)leftWord / 65535.0f;
    float volRight = (float)rightWord / 65535.0f;

    // Call SetSubVolume with separate L/R values
    AudioEngine::SetSubVolume(volLeft, volRight);

    dprintf(L"Return=MMSYSERR_NOERROR");
    return MMSYSERR_NOERROR;
}

MMRESULT WINAPI auxGetVolumeStubs(UINT uDeviceID, LPDWORD pdwVolume) {
    dprintf(L"auxGetVolumeStubs uDeviceID=0x%04X, pdwVolume=0x%08X", uDeviceID, pdwVolume);
    if (!pdwVolume) {
        dprintf(L"Return=MMSYSERR_INVALPARAM");
        return MMSYSERR_INVALPARAM;
    }

    // Get separate L/R sub-volumes
    float volLeft = 0.0f, volRight = 0.0f;
    AudioEngine::GetSubVolume(&volLeft, &volRight);

    // Clamp just in case
    if (volLeft < 0.f) volLeft = 0.f; if (volLeft > 1.f) volLeft = 1.f;
    if (volRight < 0.f) volRight = 0.f; if (volRight > 1.f) volRight = 1.f;

    // Convert back to WORDs
    WORD leftWord = (WORD)(volLeft * 65535.0f + 0.5f);
    WORD rightWord = (WORD)(volRight * 65535.0f + 0.5f);
    if (leftWord > 0xFFFF) leftWord = 0xFFFF;
    if (rightWord > 0xFFFF) rightWord = 0xFFFF;

    *pdwVolume = MAKELONG(leftWord, rightWord);

    dprintf(L"Return=MMSYSERR_NOERROR, Volume=0x%08X", *pdwVolume);
    return MMSYSERR_NOERROR;
}
