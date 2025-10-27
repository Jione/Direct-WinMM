#include "DeviceControl.h"
#include "NotifyManager.h"
#include "AudioEngineAdapter.h"

namespace {

    // Helper: Pack ms -> (MSF/TMSF/ms)
    static DWORD PackTime(UINT tf, DWORD ms, int trackForTmsf) {
        if (tf == MCI_FORMAT_MILLISECONDS) return ms;

        // ms -> frames(75fps)
        DWORD frames = MillisecondsToFrames(ms);
        BYTE mm = (BYTE)(frames / (75 * 60));
        frames %= (75 * 60);
        BYTE ss = (BYTE)(frames / 75);
        BYTE ff = (BYTE)(frames % 75);

        if (tf == MCI_FORMAT_MSF) {
            return MakeMSF(mm, ss, ff);         // mm:ss:ff
        }
        else { // TMSF
            BYTE tr = (BYTE)((trackForTmsf > 0 && trackForTmsf <= 99) ? trackForTmsf : 1);
            return MakeTMSF(tr, mm, ss, ff);    // t:mm:ss:ff
        }
    }

    static BOOL ResolvePlayRange(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam, int* outFromTr, DWORD* outFromMs, int* outToTr, DWORD* outToMs) {
        if (!outFromTr || !outFromMs || !outToTr || !outToMs) return FALSE;

        *outFromTr = 1;
        *outFromMs = 0;
        *outToTr = 1;
        *outToMs = 0xFFFFFFFF; // To the end

        // If no struct, keep defaults (String paths handled by Controller / or play with defaults)
        if (!dwParam) return TRUE;

        const MCI_PLAY_PARMS* p = (const MCI_PLAY_PARMS*)dwParam;
        UINT tf = DeviceInfo::GetDeviceTimeFormat(ctx->deviceId);

        // MCI_FROM
        if (fdw & MCI_FROM) {
            DWORD v = p->dwFrom;
            switch (tf) {
            case MCI_FORMAT_MILLISECONDS:
                *outFromTr = 1;
                *outFromMs = v;
                break;
            case MCI_FORMAT_MSF: {
                BYTE mm = MCI_MSF_MINUTE(v);
                BYTE ss = MCI_MSF_SECOND(v);
                BYTE ff = MCI_MSF_FRAME(v);
                DWORD frames = (DWORD)mm * 60 * 75 + (DWORD)ss * 75 + (DWORD)ff;
                *outFromTr = 1;
                *outFromMs = FramesToMilliseconds(frames);
                break;
            }
            case MCI_FORMAT_TMSF: {
                BYTE tr = MCI_TMSF_TRACK(v);
                BYTE mm = MCI_TMSF_MINUTE(v);
                BYTE ss = MCI_TMSF_SECOND(v);
                BYTE ff = MCI_TMSF_FRAME(v);
                DWORD frames = (DWORD)mm * 60 * 75 + (DWORD)ss * 75 + (DWORD)ff;
                *outFromTr = (tr > 0) ? tr : 1;
                *outFromMs = FramesToMilliseconds(frames);
                break;
            }
            default:
                return FALSE;
            }
        }

        // MCI_TO
        if (fdw & MCI_TO) {
            DWORD v = p->dwTo;
            switch (tf) {
            case MCI_FORMAT_MILLISECONDS:
                *outToTr = *outFromTr; // Assumed to be the same track
                *outToMs = v;
                break;
            case MCI_FORMAT_MSF: {
                BYTE mm = MCI_MSF_MINUTE(v);
                BYTE ss = MCI_MSF_SECOND(v);
                BYTE ff = MCI_MSF_FRAME(v);
                DWORD frames = (DWORD)mm * 60 * 75 + (DWORD)ss * 75 + (DWORD)ff;
                *outToTr = *outFromTr;
                *outToMs = FramesToMilliseconds(frames);
                break;
            }
            case MCI_FORMAT_TMSF: {
                BYTE tr = MCI_TMSF_TRACK(v);
                BYTE mm = MCI_TMSF_MINUTE(v);
                BYTE ss = MCI_TMSF_SECOND(v);
                BYTE ff = MCI_TMSF_FRAME(v);
                DWORD frames = (DWORD)mm * 60 * 75 + (DWORD)ss * 75 + (DWORD)ff;
                *outToTr = (tr > 0) ? tr : *outFromTr;
                *outToMs = FramesToMilliseconds(frames);
                break;
            }
            default:
                return FALSE;
            }
        }

        // If TO is missing, play by track only.
        else {
            switch (tf) {
            case MCI_FORMAT_MILLISECONDS:
            case MCI_FORMAT_TMSF:
                *outToTr = *outFromTr;
                *outToMs = 0xFFFFFFFF; // Use 0xFFFFFFFF for "to end"
                break;
            case MCI_FORMAT_MSF: {
                *outToTr = 99; // Play to end of disc
                *outToMs = 0xFFFFFFFF;
                break;
            }
            }
        }

        return TRUE;
    }

    // Internal state for Mute
    static BOOL gMuteLeft = FALSE;
    static BOOL gMuteRight = FALSE;

} // namespace

namespace Device {
    MMRESULT Open(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        // Register context
        ctx->isOpen = TRUE;
        ctx->isCDA = TRUE;
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Play(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        // Get callback handle
        HWND cb = NULL;

        // Unregister callback
        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);

        // Parse from/to/track parameters (common point for strings/structs)
        int fromTr = 1, toTr = 1; DWORD fromMs = 0, toMs = 0xFFFFFFFF;

        if (!ResolvePlayRange(ctx, fdw, dwParam, &fromTr, &fromMs, &toTr, &toMs)) {
            return MCIERR_BAD_INTEGER;
        }

        // Actual playback
        AudioEngine::StopAll();
        if (fdw & MCI_NOTIFY) {
            cb = DeviceInfo::GetNotifyHWND(ctx->deviceId);
        }

        if (!AudioEngine::PlayRange(fromTr, fromMs, toTr, toMs, FALSE, cb)) {
            NotifyManager::LazyNotify(ctx->deviceId, cb, MCI_NOTIFY_FAILURE);
            return MCIERR_INTERNAL;
        }

        // On playback success
        if (fdw & MCI_WAIT) {
            // WaitPlayback blocks and handles its own notify
            return NotifyManager::WaitPlayback(ctx->deviceId, cb, (fdw & MCI_NOTIFY));
        }
        else if (fdw & MCI_NOTIFY) {
            // Register async notification
            NotifyManager::RegisterPlaybackNotify(ctx->deviceId, cb);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Stop(DeviceContext* ctx, DWORD fdw, DWORD_PTR) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
        AudioEngine::StopAll();
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Pause(DeviceContext* ctx, DWORD fdw, DWORD_PTR) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
        AudioEngine::Pause();
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Resume(DeviceContext* ctx, DWORD fdw, DWORD_PTR) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
        AudioEngine::Resume();
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    // ADD: Added MCI_SEEK behavior emulation
    MMRESULT Seek(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        UINT tf = DeviceInfo::GetDeviceTimeFormat(ctx->deviceId);
        int tr;

        if (fdw & MCI_SEEK_TO_START) tr = 1;
        else if (fdw & MCI_SEEK_TO_END) {
            if (!AudioEngine::GetDiscNumTracks(&tr)) tr = 1;
        }
        else if (!dwParam) return MCIERR_MISSING_PARAMETER;
        else if (tf == MCI_FORMAT_TMSF) {
            MCI_SEEK_PARMS* p = (MCI_SEEK_PARMS*)dwParam;
            tr = MCI_TMSF_TRACK(p->dwTo);
            if (!(1 <= tr <= 99)) return MCIERR_BAD_INTEGER;
        }
        else tr = 0;

        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
        AudioEngine::StopAll();
        if (tr) AudioEngine::SeekTrack(tr);
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Set(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        if (!(fdw & ~(MCI_NOTIFY | MCI_WAIT))) return MCIERR_MISSING_PARAMETER;

        if (fdw & MCI_SET_TIME_FORMAT) {
            const MCI_SET_PARMS* p = (const MCI_SET_PARMS*)dwParam;
            if (!p) return MCIERR_MISSING_PARAMETER;

            UINT tf = (UINT)p->dwTimeFormat;
            switch (tf) {
            case MCI_FORMAT_MILLISECONDS:
            case MCI_FORMAT_MSF:
            case MCI_FORMAT_TMSF:
                if (DeviceInfo::SetDeviceTimeFormat(ctx->deviceId, tf)) break;
            default:
                return MCIERR_BAD_TIME_FORMAT;
            }
        }
        else if (fdw & MCI_SET_DOOR_OPEN) {
            NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
            AudioEngine::StopAll();
        }
        else if (fdw & MCI_SET_AUDIO) {
            const MCI_SET_PARMS* p = (const MCI_SET_PARMS*)dwParam;
            if (!p) return MCIERR_MISSING_PARAMETER;

            // Flags must contain ON or OFF
            if (!((fdw & MCI_SET_ON) || (fdw & MCI_SET_OFF))) {
                return MCIERR_MISSING_PARAMETER;
            }
            // Flags cannot contain both
            if ((fdw & MCI_SET_ON) && (fdw & MCI_SET_OFF)) {
                return MCIERR_FLAGS_NOT_COMPATIBLE;
            }

            BOOL setOn = (fdw & MCI_SET_ON) ? TRUE : FALSE;
            BOOL setMute = !setOn; // SET_OFF means mute

            switch (p->dwAudio) {
            case MCI_SET_AUDIO_ALL:
                gMuteLeft = setMute;
                gMuteRight = setMute;
                break;
            case MCI_SET_AUDIO_LEFT:
                gMuteLeft = setMute;
                break;
            case MCI_SET_AUDIO_RIGHT:
                gMuteRight = setMute;
                break;
            default:
                return MCIERR_BAD_CONSTANT; // Invalid audio channel
            }

            // Apply the new mute state to the audio engine
            AudioEngine::SetChannelMute(gMuteLeft, gMuteRight);
        }
        // MCI_SET_DOOR_CLOSED is just ignored
        else if (!(fdw & MCI_SET_DOOR_CLOSED)) {
            return MCIERR_UNSUPPORTED_FUNCTION;
        }
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Status(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        if (!dwParam) return MCIERR_MISSING_PARAMETER;
        MCI_STATUS_PARMS* p = (MCI_STATUS_PARMS*)dwParam;
        UINT tf = DeviceInfo::GetDeviceTimeFormat(ctx->deviceId);

        switch (p->dwItem) {
        case MCI_STATUS_NUMBER_OF_TRACKS: {
            int n = 0;
            if (!AudioEngine::GetDiscNumTracks(&n)) return MCIERR_INTERNAL;
            p->dwReturn = (DWORD)n;
            break;
        }
        case MCI_STATUS_LENGTH: {
            // If MCI_TRACK flag, get track length, otherwise total length
            if (fdw & MCI_TRACK) {
                int tr = (int)p->dwTrack;
                if (tr <= 0) return MCIERR_BAD_INTEGER;
                DWORD lenMs = 0;
                if (!AudioEngine::GetTrackLengthMs(tr, &lenMs)) return MCIERR_INTERNAL;
                p->dwReturn = PackTime(tf, lenMs, tr);
            }
            else {
                DWORD totalMs = 0;
                if (!AudioEngine::GetDiscLengthMs(&totalMs)) return MCIERR_INTERNAL;
                // Total length in TMSF usually sets t field to "track 1" or leaves it unused.
                // Here, pack t=1.
                p->dwReturn = PackTime(tf, totalMs, 1);
            }
            break;
        }
        case MCI_STATUS_POSITION: {
            // Default: Return current "track relative" position
            // If MCI_TRACK n, return the start position of that track.
            if (fdw & MCI_TRACK) {
                int tr = (int)p->dwTrack;
                if (tr <= 0) return MCIERR_BAD_INTEGER;
                // Track start point: 0ms (track relative).
                DWORD posMs = 0;
                p->dwReturn = PackTime(tf, posMs, tr);
            }
            else {
                int curTr = AudioEngine::CurrentTrack();
                DWORD curMs = AudioEngine::CurrentPositionMs();
                p->dwReturn = PackTime(tf, curMs, curTr);
            }
            break;
        }
        case MCI_STATUS_CURRENT_TRACK: {
            p->dwReturn = (DWORD)AudioEngine::CurrentTrack();
            break;
        }
        case MCI_STATUS_MODE: {
            // dwReturn holds an integer constant (not a string)
            BOOL isPlaying = !AudioEngine::HasReachedEnd() || AudioEngine::IsPlaying();
            BOOL isPaused = AudioEngine::IsPaused();
            if (isPlaying) {
                p->dwReturn = MCI_MODE_PLAY;
            }
            else if (isPaused) {
                p->dwReturn = MCI_MODE_PAUSE; // Corrected from STOP
            }
            else {
                p->dwReturn = MCI_MODE_STOP;
            }
            break;
        }
        case MCI_STATUS_MEDIA_PRESENT: {
            int n = 0;
            if (!AudioEngine::GetDiscNumTracks(&n)) return MCIERR_INTERNAL;
            p->dwReturn = (n > 0) ? TRUE : FALSE;
            break;
        }
        case MCI_STATUS_TIME_FORMAT: {
            p->dwReturn = tf; // MCI_FORMAT_*
            break;
        }
        default:
            return MCIERR_UNSUPPORTED_FUNCTION;
        }
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT Info(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        if (!dwParam) return MCIERR_MISSING_PARAMETER;

        MCI_INFO_PARMSW* p = (MCI_INFO_PARMSW*)dwParam;
        if (!p->lpstrReturn || p->dwRetSize == 0) return MCIERR_PARAM_OVERFLOW;

        auto devInfo = DeviceInfo::GetDeviceInfo();
        LPCWSTR out;
        if (fdw & MCI_INFO_PRODUCT) {
            out = devInfo->szPnameW;
        }
        else if (fdw & MCI_INFO_MEDIA_IDENTITY) {
            out = devInfo->idW;
        }
        else if (fdw & MCI_INFO_MEDIA_UPC) {
            out = devInfo->upcW;
        }
        else {
            return MCIERR_MISSING_PARAMETER;
        }

        // Copy string safely
        lstrcpynW(p->lpstrReturn, out, p->dwRetSize);

        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return MMSYSERR_NOERROR;
    }

    MMRESULT SysInfo(DeviceContext* ctx, DWORD fdw, DWORD_PTR dwParam) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        if (!dwParam) return MCIERR_MISSING_PARAMETER;

        MCI_SYSINFO_PARMSW* p = (MCI_SYSINFO_PARMSW*)dwParam;
        if (!p->lpstrReturn || p->dwRetSize == 0) return MCIERR_PARAM_OVERFLOW;

        // Default to empty string
        p->lpstrReturn[0] = L'\0';
        MMRESULT ret = MMSYSERR_NOERROR;

        if (fdw & MCI_SYSINFO_QUANTITY) {
            // Requesting the number of devices of a certain type.
            if (p->wDeviceType == MCI_DEVTYPE_CD_AUDIO) {
                // simulating one (1) CD audio device
                lstrcpynW(p->lpstrReturn, L"1", p->dwRetSize);
            }
            else {
                // Other types not supported
                lstrcpynW(p->lpstrReturn, L"0", p->dwRetSize);
            }
        }
        else if (fdw & MCI_SYSINFO_NAME) {
            // Requesting the name of device number N.
            if (p->wDeviceType == MCI_DEVTYPE_CD_AUDIO && p->dwNumber == 1) {
                // Return the name of our virtual CD device
                auto devInfo = DeviceInfo::GetDeviceInfo();
                lstrcpynW(p->lpstrReturn, devInfo->szPnameW, p->dwRetSize);
            }
            else {
                // Device number out of range (not 1) or wrong type
                ret = MCIERR_BAD_CONSTANT;
            }
        }
        else if (fdw & MCI_SYSINFO_OPEN) {
            // Requesting if device number N is open.
            if (p->wDeviceType == MCI_DEVTYPE_CD_AUDIO && p->dwNumber == 1) {
                // Check if our single device is in use
                DeviceContext* cda = DeviceInfo::GetFirstDevice();
                if (cda && cda->isOpen) {
                    lstrcpynW(p->lpstrReturn, L"TRUE", p->dwRetSize);
                }
                else {
                    lstrcpynW(p->lpstrReturn, L"FALSE", p->dwRetSize);
                }
            }
            else {
                // Device number out of range (not 1) or wrong type
                ret = MCIERR_BAD_CONSTANT;
            }
        }
        else {
            // Other MCI_SYSINFO flags (like MCI_SYSINFO_INSTALLNAME) not supported
            ret = MCIERR_UNSUPPORTED_FUNCTION;
        }

        if (ret == MMSYSERR_NOERROR && (fdw & MCI_NOTIFY)) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        return ret;
    }

    MMRESULT Close(DeviceContext* ctx, DWORD fdw, DWORD_PTR) {
        if (!ctx || !ctx->isOpen) return MCIERR_DEVICE_NOT_READY;
        NotifyManager::UnregisterPlaybackNotify(ctx->deviceId);
        AudioEngine::StopAll();
        if (fdw & MCI_NOTIFY) {
            NotifyManager::LazyNotify(ctx->deviceId, ctx->notifyHwnd, MCI_NOTIFY_SUCCESSFUL);
        }
        ctx->isOpen = FALSE;
        return MMSYSERR_NOERROR;
    }
}
