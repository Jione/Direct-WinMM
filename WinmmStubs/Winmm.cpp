#include "DeviceInfo.h"
#include "DeviceControl.h"
#include "NotifyManager.h"
#include "AudioEngineAdapter.h"
#include <wchar.h>

#define COPY_GENERIC_FIELDS_A2W(pA, pW)  do { if ((pA) && (pW)) { (pW)->dwCallback = (pA)->dwCallback; }} while(0)

// const WCHAR* const [] : Array pointers and the strings they point to are both constants
const WCHAR* const mciCommand[] = {
    L"open", L"play", L"stop", L"pause", L"resume", L"seek", L"close", L"set", L"status", L"info", L"sysinfo", L"capability"
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
    CMD_GETDEVCAPS,
    CMD_UNKNOWN,
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

// Helper for case-insensitive string comparison.
static BOOL IsWordEq(const WCHAR* a, const WCHAR* b) { return (lstrcmpiW(a, b) == 0); }

// Quote-aware tokenizer (e.g., open "my file.wav" alias foo).
static WCHAR* GetNextToken(WCHAR** ppString) {
    if (!ppString || !*ppString) return NULL;

    WCHAR* p = *ppString;
    BOOL inQuote = FALSE;

    // Skip leading whitespace
    while (*p && iswspace(*p)) p++;
    if (*p == L'\0') {
        *ppString = p;
        return NULL;
    }

    WCHAR* tokenStart = p;

    // Find end of token
    while (*p) {
        if (*p == L'"') { inQuote = !inQuote; }
        else if (iswspace(*p) && !inQuote) { break; }
        p++;
    }

    if (*p) {
        *p = L'\0'; // Terminate token
        *ppString = p + 1; // Save next start position
    }
    else { *ppString = p; } // End of string

    // Trim quotes
    if (*tokenStart == L'"' && *(p - 1) == L'"') {
        tokenStart++;
        *(p - 1) = L'\0';
    }

    return tokenStart;
}

// Parses an MCI integer or a "Colonized" time string (e.g., mm:ss:ff) into a DWORD.
// "1:10:30" (MSF) -> 0x1E0A01 (30 frames, 10 sec, 1 min)
static BOOL ParseMciInteger(const WCHAR* tok, DWORD* outValue) {
    if (!tok || !outValue) return FALSE;

    *outValue = 0;
    DWORD dwResult = 0;
    DWORD Shift = 0;
    BOOL fDigitFound = FALSE;
    const WCHAR* p = tok;

    // Check for simple integer/ms (no colons)
    if (wcschr(tok, L':') == NULL) {
        // Parse as a standard integer (handles negatives)
        WCHAR* endPtr;
        *outValue = (DWORD)wcstol(tok, &endPtr, 10);
        return (*endPtr == L'\0'); // Ensure the entire token was parsed
    }

    // "Colonized" format (mm:ss:ff or tt:mm:ss:ff) parsing
    while (*p) {
        if (*p >= L'0' && *p <= L'9') {
            fDigitFound = TRUE;
            dwResult = (dwResult * 10) + (*p - L'0');
            // Each component (mm, ss, ff) cannot exceed one byte (0-255)
            if (dwResult > 0xFF) return FALSE;
        }
        else if (*p == L':') {
            if (!fDigitFound) return FALSE; // Prevent formats like ":10"
            if (Shift > 24) return FALSE;   // Prevent overflow (more than 4 bytes, e.g., tt:mm:ss:ff)

            *outValue |= (dwResult << Shift);
            Shift += 8;
            dwResult = 0;
            fDigitFound = FALSE;
        }
        else { break; } // Stop if a non-digit, non-colon char is found
        p++;
    }

    // Store the last parsed component
    if (fDigitFound) {
        if (Shift > 24) return FALSE;
        *outValue |= (dwResult << Shift);
    }
    else if (Shift == 0) { return FALSE; } // Fail if string ends with a colon, e.g., "10:"

    return (*p == L'\0');
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
        case MCI_PLAY:      *ret = Device::Play(ctx, fdwCommand, dwParam); break;
        case MCI_STOP:      *ret = Device::Stop(ctx, fdwCommand, dwParam); break;
        case MCI_PAUSE:     *ret = Device::Pause(ctx, fdwCommand, dwParam); break;
        case MCI_RESUME:    *ret = Device::Resume(ctx, fdwCommand, dwParam); break;
        case MCI_SEEK:      *ret = Device::Seek(ctx, fdwCommand, dwParam); break;
        case MCI_SET:       *ret = Device::Set(ctx, fdwCommand, dwParam); break;
        case MCI_STATUS:    *ret = Device::Status(ctx, fdwCommand, dwParam); break;
        case MCI_INFO:      *ret = Device::Info(ctx, fdwCommand, dwParam); break;
        case MCI_SYSINFO:   *ret = Device::SysInfo(ctx, fdwCommand, dwParam); break;
        case MCI_GETDEVCAPS:*ret = Device::DevCaps(ctx, fdwCommand, dwParam); break;
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

    WCHAR buf[512]; lstrcpynW(buf, lpstrCommand, 512);
    WCHAR* checkText = buf; // Points to the current parsing position
    MCI_COMMANDS cmd = CMD_UNKNOWN;
    WCHAR* token = GetNextToken(&checkText); // Use quote-aware tokenizer

    if (!token) return FALSE;

    // Parse first command
    for (size_t i = 0; i < CMD_UNKNOWN; ++i) {
        if (lstrcmpiW(mciCommand[i], token) == 0) {
            cmd = (MCI_COMMANDS)i;
            break;
        }
    }
    if (lstrcmpiW(mciCommand[CMD_UNKNOWN], token) == 0) {
        cmd = CMD_GETDEVCAPS;
    }

    if (cmd == CMD_UNKNOWN) return FALSE;

    // Parse device name / alias
    token = GetNextToken(&checkText); // "cdaudio", "myAlias", "new"
    if (!token && cmd != CMD_SYSINFO) { // SYSINFO may omit device name
        *ret = MCIERR_MISSING_DEVICE_NAME;
        return TRUE;
    }

    WCHAR deviceName[64] = { 0, };
    WCHAR deviceAlias[64] = { 0, };
    BOOL isCdaudio = FALSE;
    BOOL isNew = FALSE;
    BOOL isAll = FALSE;

    if (token) {
        if (IsWordEq(token, L"cdaudio")) {
            lstrcpynW(deviceName, token, 64);
            isCdaudio = TRUE;
        }
        else if (IsWordEq(token, L"new")) {
            isNew = TRUE;
            isCdaudio = TRUE; // 'new' implies cdaudio for this stub
        }
        else if (IsWordEq(token, L"all")) {
            isAll = TRUE; // 'all' implies cdaudio for this stub
            isCdaudio = TRUE;
        }
        else if (DeviceInfo::FindByAlias(token)) {
            lstrcpynW(deviceAlias, token, 64);
            lstrcpynW(deviceName, L"cdaudio", 64);
            isCdaudio = TRUE;
        }
        else if (cmd != CMD_OPEN && DeviceInfo::FindByElement(token)) {
            DeviceContext* foundCtx = DeviceInfo::FindByElement(token);
            if (foundCtx) {
                lstrcpynW(deviceAlias, foundCtx->alias, 64); // Use its alias
                lstrcpynW(deviceName, L"cdaudio", 64);
                isCdaudio = TRUE;
            }
        }
        else {
            if (cmd == CMD_OPEN) {
                // For 'open', this could be an element name or a new alias
                lstrcpynW(deviceName, token, 64);
            }
            else {
                // For other commands, this is an unknown device
                lstrcpynW(deviceName, token, 64);
                isCdaudio = FALSE;
            }
        }
    }


    // Find DeviceContext
    DeviceContext* ctx = NULL;
    if (cmd == CMD_OPEN) {
        // OPEN creates or finds the ctx later in the parsing loop
        if (isNew) {
            // 'open new ...'
        }
        else if (isAll) {
            return FALSE;
        }
        else if (isCdaudio) {
            if (deviceAlias[0]) {
                ctx = DeviceInfo::FindByAlias(deviceAlias);
            }
            else {
                ctx = DeviceInfo::GetFirstDevice();
            }
        }
    }
    else {
        // Other commands require an existing ctx
        if (isAll) {
            ctx = DeviceInfo::GetFirstDevice();
            if (!ctx) {
                MCIDEVICEID newId = 0;
                if (!(DeviceInfo::Initialize() && DeviceInfo::CreateDevice(NULL, &newId))) {
                    return FALSE;
                }

                ctx = DeviceInfo::FindByDeviceID(newId);
                ctx->isCDA = TRUE;
                ctx->timeFormat = MCI_FORMAT_TMSF;  // Time format of the unique device is assumed to be tmsf.
            }
        }
        else if (isCdaudio) {
            if (deviceAlias[0]) {
                ctx = DeviceInfo::FindByAlias(deviceAlias);
            }
            else {
                ctx = DeviceInfo::GetFirstDevice();
            }
        }

        if (!isCdaudio) return FALSE; // Don't handle non-cdaudio strings

        if (!ctx) {
            // Auto-open default device if 'play cdaudio' is used
            if (!deviceAlias[0] && IsWordEq(deviceName, L"cdaudio")) {
                MCIDEVICEID newId = 0;
                if (!(DeviceInfo::Initialize() && DeviceInfo::CreateDevice(NULL, &newId))) {
                    return FALSE;
                }
                ctx = DeviceInfo::FindByDeviceID(newId);
                ctx->isCDA = TRUE;
                ctx->timeFormat = MCI_FORMAT_MSF;
            }
            else {
                *ret = MCIERR_INVALID_DEVICE_NAME;
                return !isAll;;
            }
        }
    }

    DWORD fdwCommand = 0;

    // OPEN is special cased (ctx creation)
    if (cmd == CMD_OPEN) {
        MCI_OPEN_PARMSW dwParam{ 0, };
        dwParam.dwCallback = (DWORD_PTR)hCallback;

        BOOL typeFlagFound = FALSE;
        WCHAR alias[64] = { 0, };

        // Parse 'open' parameters
        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify"))
                fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait"))
                fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"alias")) {
                token = GetNextToken(&checkText);
                if (token) {
                    fdwCommand |= MCI_OPEN_ALIAS;
                    lstrcpynW(alias, token, 64);
                    dwParam.lpstrAlias = alias;
                }
            }
            else if (IsWordEq(token, L"type")) {
                token = GetNextToken(&checkText);
                if (token) {
                    typeFlagFound = TRUE;
                    if (IsWordEq(token, L"cdaudio")) {
                        isCdaudio = TRUE;
                        dwParam.lpstrDeviceType = (LPCWSTR)MCI_DEVTYPE_CD_AUDIO;
                        fdwCommand |= MCI_OPEN_TYPE_ID; // Use type ID
                    }
                    else {
                        return FALSE; // Not cdaudio
                    }
                }
            }
        }

        if (!isCdaudio && !typeFlagFound) return FALSE; // 'open foo.wav'
        if (isNew && !(fdwCommand & MCI_OPEN_ALIAS)) {
            *ret = MCIERR_NEW_REQUIRES_ALIAS;
            return TRUE;
        }

        // --- Re-using mciCommandHub OPEN logic ---
        MCIDEVICEID newId = 0;
        if (alias[0]) {
            ctx = DeviceInfo::FindByAlias(alias);
        }
        if (!ctx) {
            ctx = DeviceInfo::GetFirstDevice();
        }

        if (ctx && ctx->isOpen) {
            newId = ctx->deviceId;
        }
        else {
            if (!DeviceInfo::Initialize()) { *ret = MCIERR_INTERNAL; return TRUE; }
            if (!DeviceInfo::CreateDevice(alias[0] ? alias : NULL, &newId)) {
                *ret = MCIERR_INTERNAL; return TRUE;
            }
        }

        ctx = DeviceInfo::FindByDeviceID(newId);
        ctx->isCDA = TRUE;
        dwParam.wDeviceID = newId;

        if (isCdaudio && !IsWordEq(deviceName, L"cdaudio") && !isNew) {
            fdwCommand |= MCI_OPEN_ELEMENT;
            dwParam.lpstrElementName = deviceName;
        }

        DeviceInfo::SetNotifyHWND(ctx->deviceId, (HWND)dwParam.dwCallback);
        DeviceInfo::SetOpenFlags(ctx->deviceId, fdwCommand);
        if (alias[0]) { DeviceInfo::SetAlias(newId, alias); }

        ctx->requestId = 0; // mciSendString uses deviceId 0
        *ret = Device::Open(ctx, fdwCommand, (DWORD_PTR)&dwParam);

        if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
            wsprintfW(lpstrReturn, L"%u", ctx->deviceId);
        }
        return TRUE;
    }

    // --- All commands other than OPEN ---
    if (!ctx) {
        *ret = MCIERR_INVALID_DEVICE_NAME;
        return !isAll;
    }
    ctx->requestId = ctx->deviceId;
    DeviceInfo::SetNotifyHWND(ctx->deviceId, hCallback);

    switch (cmd) {
    case CMD_PLAY: {
        MCI_PLAY_PARMS dwParam{ 0, };
        BOOL isTrack = FALSE;
        dwParam.dwCallback = (DWORD_PTR)hCallback;

        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"from")) {
                fdwCommand |= MCI_FROM;
                token = GetNextToken(&checkText);
                if (IsWordEq(token, L"track")) {
                    isTrack = TRUE;
                    token = GetNextToken(&checkText);
                }
                else if (!(token[1] == ':' || token[2] == ':')) isTrack = TRUE;
                if (!token || !ParseMciInteger(token, &dwParam.dwFrom)) {
                    *ret = MCIERR_BAD_INTEGER; return !isAll;;
                }
            }
            else if (IsWordEq(token, L"to")) {
                fdwCommand |= MCI_TO;
                token = GetNextToken(&checkText);
                if (IsWordEq(token, L"track")) {
                    isTrack = TRUE;
                    token = GetNextToken(&checkText);
                }
                if (!(token[1] == ':' || token[2] == ':')) isTrack = TRUE;
                if (!token || !ParseMciInteger(token, &dwParam.dwTo)) {
                    *ret = MCIERR_BAD_INTEGER; return !isAll;;
                }
            }
        }
        if (isTrack) {
            UINT storeTf = ctx->timeFormat;
            ctx->timeFormat = MCI_FORMAT_TMSF;
            *ret = Device::Play(ctx, fdwCommand, (DWORD_PTR)&dwParam);
            ctx->timeFormat = storeTf;
        }
        else {
            *ret = Device::Play(ctx, fdwCommand, (DWORD_PTR)&dwParam);
        }
        break;
    }
    case CMD_STOP:
    case CMD_PAUSE:
    case CMD_RESUME: {
        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
        }
        if (cmd == CMD_STOP) *ret = Device::Stop(ctx, fdwCommand, NULL);
        else if (cmd == CMD_PAUSE) *ret = Device::Pause(ctx, fdwCommand, NULL);
        else if (cmd == CMD_RESUME) *ret = Device::Resume(ctx, fdwCommand, NULL);
        break;
    }
    case CMD_SEEK: {
        MCI_SEEK_PARMS dwParam{ 0, };
        dwParam.dwCallback = (DWORD_PTR)hCallback;

        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"track")) fdwCommand |= MCI_TRACK;
            else if (IsWordEq(token, L"to")) {
                token = GetNextToken(&checkText);
                if (token) {
                    if (IsWordEq(token, L"start")) fdwCommand |= MCI_SEEK_TO_START;
                    else if (IsWordEq(token, L"end")) fdwCommand |= MCI_SEEK_TO_END;
                    else if (IsWordEq(token, L"track")) fdwCommand |= MCI_TRACK;
                    else {
                        fdwCommand |= MCI_TO;
                        if (!ParseMciInteger(token, &dwParam.dwTo)) {
                            *ret = MCIERR_BAD_INTEGER; return !isAll;;
                        }
                    }
                }
            }
        }
        *ret = Device::Seek(ctx, fdwCommand, (DWORD_PTR)&dwParam);
        break;
    }
    case CMD_SET: {
        MCI_SET_PARMS dwParam{ 0, };
        dwParam.dwCallback = (DWORD_PTR)hCallback;
        BOOL isFail = FALSE;

        while (isFail || ((token = GetNextToken(&checkText)) != NULL)) {
            isFail = FALSE;
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"time") || IsWordEq(token, L"time_format")) {
                if (IsWordEq(token, L"time")) {
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"format")) { isFail = TRUE; continue; }
                }
                fdwCommand |= MCI_SET_TIME_FORMAT;
                token = GetNextToken(&checkText);
                if (token && IsWordEq(token, L"tmsf"))
                    dwParam.dwTimeFormat = MCI_FORMAT_TMSF;
                else if (token && IsWordEq(token, L"msf"))
                    dwParam.dwTimeFormat = MCI_FORMAT_MSF;
                else if (token && (IsWordEq(token, L"ms") || IsWordEq(token, L"milliseconds")))
                    dwParam.dwTimeFormat = MCI_FORMAT_MILLISECONDS;
                else { *ret = MCIERR_BAD_CONSTANT; return !isAll;; }
            }
            else if (IsWordEq(token, L"audio")) {
                token = GetNextToken(&checkText); // all/left/right
                if (token && IsWordEq(token, L"all")) dwParam.dwAudio = MCI_SET_AUDIO_ALL;
                else if (token && IsWordEq(token, L"left")) dwParam.dwAudio = MCI_SET_AUDIO_LEFT;
                else if (token && IsWordEq(token, L"right")) dwParam.dwAudio = MCI_SET_AUDIO_RIGHT;
                else { *ret = MCIERR_BAD_CONSTANT; return !isAll;; }

                token = GetNextToken(&checkText); // on/off
                if (token && IsWordEq(token, L"on")) fdwCommand |= MCI_SET_ON;
                else if (token && IsWordEq(token, L"off")) fdwCommand |= MCI_SET_OFF;
                else { *ret = MCIERR_MISSING_PARAMETER; return !isAll;; }
            }
            else if (IsWordEq(token, L"door")) {
                token = GetNextToken(&checkText); // open/closed
                if (token && IsWordEq(token, L"open")) fdwCommand |= MCI_SET_DOOR_OPEN;
                else if (token && IsWordEq(token, L"closed")) fdwCommand |= MCI_SET_DOOR_CLOSED;
                else { *ret = MCIERR_BAD_CONSTANT; return !isAll;; }
            }
        }
        *ret = Device::Set(ctx, fdwCommand, (DWORD_PTR)&dwParam);
        break;
    }
    case CMD_STATUS: {
        MCI_STATUS_PARMS dwParam{ 0, };
        dwParam.dwCallback = (DWORD_PTR)hCallback;
        BOOL isFail = FALSE;

        while (isFail || ((token = GetNextToken(&checkText)) != NULL)) {
            isFail = FALSE;
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"length")) {
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_LENGTH;
            }
            else if (IsWordEq(token, L"position")) {
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_POSITION;
            }
            else if (IsWordEq(token, L"number") || IsWordEq(token, L"number_of_tracks")) {
                if (IsWordEq(token, L"number")) {
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"of")) { isFail = TRUE; continue; }
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"tracks")) { isFail = TRUE; continue; }
                }
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;
            }
            else if (IsWordEq(token, L"current") || IsWordEq(token, L"current_track")) {
                if (IsWordEq(token, L"current")) {
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"track")) { isFail = TRUE; continue; }
                }
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_CURRENT_TRACK;
            }
            else if (IsWordEq(token, L"mode")) {
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_MODE;
            }
            else if (IsWordEq(token, L"media") || IsWordEq(token, L"media_present")) {
                if (IsWordEq(token, L"media")) {
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"present")) { isFail = TRUE; continue; }
                }
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_MEDIA_PRESENT;
            }
            else if (IsWordEq(token, L"time") || IsWordEq(token, L"time_format")) {
                if (IsWordEq(token, L"time")) {
                    token = GetNextToken(&checkText);
                    if (!token) { continue; }
                    else if (!IsWordEq(token, L"format")) { isFail = TRUE; continue; }
                }
                fdwCommand |= MCI_STATUS_ITEM; dwParam.dwItem = MCI_STATUS_TIME_FORMAT;
            }
            else if (IsWordEq(token, L"track")) {
                // "status ... track N ..."
                fdwCommand |= MCI_TRACK;
                token = GetNextToken(&checkText);
                DWORD trackNum;
                if (!token || !ParseMciInteger(token, &trackNum)) {
                    *ret = MCIERR_BAD_INTEGER; return !isAll;;
                }
                dwParam.dwTrack = trackNum;
            }
        }

        if (!(fdwCommand & MCI_STATUS_ITEM)) {
            *ret = MCIERR_MISSING_PARAMETER; return !isAll;;
        }

        *ret = Device::Status(ctx, fdwCommand, (DWORD_PTR)&dwParam);

        if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
            // Format return string
            switch (dwParam.dwItem) {
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
                lstrcpynW(lpstrReturn, (dwParam.dwReturn ? L"true" : L"false"), uReturnLength);
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

        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"product")) fdwCommand |= MCI_INFO_PRODUCT;
            else if (IsWordEq(token, L"upc")) fdwCommand |= MCI_INFO_MEDIA_UPC;
            else if (IsWordEq(token, L"identity")) fdwCommand |= MCI_INFO_MEDIA_IDENTITY;
        }
        if (!fdwCommand) { *ret = MCIERR_MISSING_PARAMETER; return !isAll;; }

        *ret = Device::Info(ctx, fdwCommand, (DWORD_PTR)&dwParam);

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
        dwParam.wDeviceType = MCI_DEVTYPE_CD_AUDIO; // Fixed to cdaudio

        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"installname")) fdwCommand |= MCI_SYSINFO_INSTALLNAME;
            else if (IsWordEq(token, L"quantity")) fdwCommand |= MCI_SYSINFO_QUANTITY;
            else if (IsWordEq(token, L"open")) fdwCommand |= MCI_SYSINFO_OPEN;
            else if (IsWordEq(token, L"name")) {
                fdwCommand |= MCI_SYSINFO_NAME;
                token = GetNextToken(&checkText); // "name cdaudio N"
                DWORD devNum;
                if (token && ParseMciInteger(token, &devNum)) {
                    dwParam.dwNumber = devNum;
                }
                else {
                    dwParam.dwNumber = 1; // Default
                }
            }
        }
        if (!fdwCommand) { *ret = MCIERR_MISSING_PARAMETER; return !isAll;; }

        *ret = Device::SysInfo(ctx, fdwCommand, (DWORD_PTR)&dwParam);

        if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
            lstrcpynW(lpstrReturn, str, uReturnLength);
        }
        break;
    }
    case CMD_GETDEVCAPS: {
        MCI_GETDEVCAPS_PARMS dwParam{ 0, };
        dwParam.dwCallback = (DWORD_PTR)hCallback;
        BOOL isFail = FALSE;

        while (isFail || ((token = GetNextToken(&checkText)) != NULL)) {
            isFail = FALSE;
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
            else if (IsWordEq(token, L"can")) {
                token = GetNextToken(&checkText);
                if (!token) { continue; }
                else if (IsWordEq(token, L"eject")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_CAN_EJECT;
                }
                else if (IsWordEq(token, L"play")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_CAN_PLAY;
                }
                else if (IsWordEq(token, L"record")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_CAN_RECORD;
                }
                else if (IsWordEq(token, L"save")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_CAN_SAVE;
                }
                else { isFail = TRUE; continue; }
            }
            else if (IsWordEq(token, L"compound")) {
                token = GetNextToken(&checkText);
                if (!token) { continue; }
                else if (!IsWordEq(token, L"device")) { isFail = TRUE; continue; }
                fdwCommand |= MCI_GETDEVCAPS_ITEM;
                dwParam.dwItem = MCI_GETDEVCAPS_COMPOUND_DEVICE;
            }
            else if (IsWordEq(token, L"device")) {
                token = GetNextToken(&checkText);
                if (!token) { continue; }
                else if (!IsWordEq(token, L"type")) { isFail = TRUE; continue; }
                fdwCommand |= MCI_GETDEVCAPS_ITEM;
                dwParam.dwItem = MCI_GETDEVCAPS_DEVICE_TYPE;
            }
            else if (IsWordEq(token, L"has")) {
                token = GetNextToken(&checkText);
                if (!token) { continue; }
                else if (IsWordEq(token, L"audio")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_HAS_AUDIO;
                }
                else if (IsWordEq(token, L"video")) {
                    fdwCommand |= MCI_GETDEVCAPS_ITEM;
                    dwParam.dwItem = MCI_GETDEVCAPS_HAS_VIDEO;
                }
                else { isFail = TRUE; continue; }
            }
            else if (IsWordEq(token, L"uses")) {
                token = GetNextToken(&checkText);
                if (!token) { continue; }
                else if (!IsWordEq(token, L"files")) { isFail = TRUE; continue; }
                fdwCommand |= MCI_GETDEVCAPS_ITEM;
                dwParam.dwItem = MCI_GETDEVCAPS_USES_FILES;
            }
        }

        *ret = Device::DevCaps(ctx, fdwCommand, (DWORD_PTR)&dwParam);

        if (*ret == MMSYSERR_NOERROR && lpstrReturn) {
            // Format return string
            switch (dwParam.dwItem) {
            case MCI_GETDEVCAPS_DEVICE_TYPE:
                lstrcpynW(lpstrReturn, L"cdaudio", uReturnLength); break;
            default:
                lstrcpynW(lpstrReturn, dwParam.dwReturn ? L"true" : L"false", uReturnLength); break;
                break;
            }
        }
        break;
    }
    case CMD_CLOSE: {
        while ((token = GetNextToken(&checkText)) != NULL) {
            if (IsWordEq(token, L"notify")) fdwCommand |= MCI_NOTIFY;
            else if (IsWordEq(token, L"wait")) fdwCommand |= MCI_WAIT;
        }
        *ret = Device::Close(ctx, fdwCommand, NULL);
        DeviceInfo::Destroy(ctx->deviceId);
        break;
    }
    default:
        *ret = MCIERR_UNRECOGNIZED_COMMAND;
        break;
    }

    return !isAll;;
}

// mciSendCommandW connection function
MCIERROR WINAPI mciSendCommandWStubs(MCIDEVICEID deviceId, UINT uMsg, DWORD_PTR fdwCommand, DWORD_PTR dwParam) {
    MCIERROR ret;
#ifdef _DEBUG
    wchar_t errText[256]{ 0, };
#endif
    dprintf(L"mciSendCommandW deviceId=0x%04X uMsg=0x%04X fdwCommand=0x%08X dwParam=0x%08X", deviceId, uMsg, fdwCommand, dwParam);
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
    dprintf(L"mciSendCommandA deviceId=0x%04X uMsg=0x%04X fdwCommand=0x%08X dwParam=0x%08X", deviceId, uMsg, fdwCommand, dwParam);

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
    dprintf(L"mciSendStringW lpstrCommand=%s lpstrReturn=0x%08X uReturnLength=%lu hCallback=0x%08X", lpstrCommand, lpstrReturn, uReturnLength, hCallback);
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

    dprintf("mciSendStringA lpstrCommand=%s lpstrReturn=0x%08X uReturnLength=%lu hCallback=0x%08X", lpstrCommand, lpstrReturn, uReturnLength, hCallback);

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
