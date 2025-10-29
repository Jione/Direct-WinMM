#include "AudioEngineAdapter.h"
#include "AudioEngineDirectSound.h"
#include "AudioEngineWASAPI.h"
#include <string>
#include <map>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

// Define this to pre-decode the entire play range into memory.
// If undefined, the engine will stream from the files.
// #define AE_USE_FULLBUFFER

// ===================== Internal Helpers / State =====================
namespace {

    // --- Global State & Engine Abstraction ---

    static BOOL gVistaOrLater = FALSE;
    typedef LONG(WINAPI* RtlGetVersion_t)(PRTL_OSVERSIONINFOW);
    static BOOL IsVistaOrLater() {
        static int cached = -1;
        if (cached != -1) return cached ? TRUE : FALSE;
        HMODULE hNt = GetModuleHandleW(L"ntdll.dll");
        if (hNt) {
            RtlGetVersion_t p = (RtlGetVersion_t)GetProcAddress(hNt, "RtlGetVersion");
            if (p) { RTL_OSVERSIONINFOW vi = { sizeof(vi) }; if (p(&vi) == 0) { cached = (vi.dwMajorVersion >= 6) ? 1 : 0; return cached ? TRUE : FALSE; } }
        }
        OSVERSIONINFOEXW os = { sizeof(os) }; GetVersionExW((OSVERSIONINFOW*)&os);
        cached = (os.dwMajorVersion >= 6) ? 1 : 0;
        return cached ? TRUE : FALSE;
    }

    // Engine Instances
    static DSoundAudioEngine gDS;
    static WasapiAudioEngine gWAS;

    // Engine Abstraction Wrappers
    inline BOOL Engine_Initialize(HWND hwnd) { return gVistaOrLater ? gWAS.Initialize(hwnd) : gDS.Initialize(hwnd); }
    inline void Engine_Shutdown() { gVistaOrLater ? gWAS.Shutdown() : gDS.Shutdown(); }
    inline BOOL Engine_Play(UINT sr, UINT ch, BOOL loop, DWORD(WINAPI* fill)(short*, DWORD, void*)) {
        return gVistaOrLater ? gWAS.PlayStream(sr, ch, fill, NULL, loop)
            : gDS.PlayStream(sr, ch, fill, NULL, loop);
    }
    inline void Engine_Stop() { gVistaOrLater ? gWAS.Stop() : gDS.Stop(); }
    inline void Engine_Pause() { gVistaOrLater ? gWAS.Pause() : gDS.Pause(); }
    inline void Engine_Resume() { gVistaOrLater ? gWAS.Resume() : gDS.Resume(); }
    inline void Engine_SetVol(float v) { gVistaOrLater ? gWAS.SetVolume(v) : gDS.SetVolume(v); }
    inline void Engine_SetChannelMute(BOOL l, BOOL r) { gVistaOrLater ? gWAS.SetChannelMute(l, r) : gDS.SetChannelMute(l, r); }
    inline void Engine_SetSubVol(float l, float r) { gVistaOrLater ? gWAS.SetSubVolume(l, r) : gDS.SetSubVolume(l, r); }
    inline BOOL Engine_IsPlaying() { return gVistaOrLater ? gWAS.IsPlaying() : gDS.IsPlaying(); }
    inline BOOL Engine_IsPaused() { return gVistaOrLater ? gWAS.IsPaused() : gDS.IsPaused(); }
    inline DWORD Engine_PosMs() { return gVistaOrLater ? gWAS.GetPositionMs() : gDS.GetPositionMs(); }
    inline UINT  Engine_SR() { return gVistaOrLater ? gWAS.CurrentSampleRate() : gDS.CurrentSampleRate(); }
    inline UINT  Engine_CH() { return gVistaOrLater ? gWAS.CurrentChannels() : gDS.CurrentChannels(); }

    // Adapter State (Common)
    static BOOL gInited = FALSE;
    static int  gCurTrack = -1;
    static BOOL gLoop = TRUE;
    static HWND gNotify = NULL;

    // Volume Cache (Adapter Master/Sub)
    static float gMasterVol = 1.0f;
    static float gSubVolLeft = 1.0f;
    static float gSubVolRight = 1.0f;
    static BOOL  gMuteLeft = FALSE;
    static BOOL  gMuteRight = FALSE;

    // MCICDA compatibility state cache
    static BOOL  gEverPlayed = FALSE; // Has playback ever started
    static int   gStatusTrack = 1;    // Current track number for status query
    static BOOL  gBufferEmpty = FALSE; // Has non-looping playback finished
    static std::wstring gPathFormat = L"music\\Track%02d"; // Default path format base
    static BOOL  gPathFormatDetected = FALSE;              // Flag to run detection only once
    static DWORD gRangeStartMs = 0;   // The absolute start Ms of the first segment
    static DWORD gRangeTotalMs = 0;   // The total duration of the current range
    static DWORD gLatency = 0;        // Engine buffer latency (ms)

    // --- General Utilities ---

    static inline DWORD MinDW(DWORD a, DWORD b) { return (a < b) ? a : b; }
    static inline BOOL  PathExistsW(const wchar_t* p) { return (p && PathFileExistsW(p)); }

    static DWORD MsToFrame(DWORD ms, const AD_Format& f) {
        if (ms == 0xFFFFFFFF) return 0xFFFFFFFF;
        return (DWORD)((unsigned __int64)ms * (unsigned __int64)f.sampleRate / 1000ULL);
    }
    static DWORD FramesToMs(DWORD frames, const AD_Format& f) {
        if (f.sampleRate == 0) return 0;
        return (DWORD)((unsigned __int64)frames * 1000ULL / (unsigned __int64)f.sampleRate);
    }
    static BOOL HasReachedEnd() {
        if (gLoop || !gEverPlayed || gRangeTotalMs == 0) return FALSE;

        DWORD elapsedMs = Engine_PosMs();
        return (elapsedMs >= gRangeTotalMs);
    }

    // --- Track/Disc Logic (Helpers for Public API) ---
    static void InitializeTrackPath() {
        if (gPathFormatDetected) return; // Already detected or set externally

        wchar_t tempPath[MAX_PATH];
        bool currentFormatWorks = false;
        const wchar_t* exts[] = { L".wav", L".ogg", L".mp3" };

        // Check if the current gPathFormat works
        for (int t = 1; t <= 99; ++t) {
            wchar_t base[MAX_PATH];
            wsprintfW(base, gPathFormat.c_str(), t); // Check current format
            for (int i = 0; i < _countof(exts); ++i) {
                wchar_t fullPath[MAX_PATH];
                lstrcpynW(fullPath, base, MAX_PATH);
                lstrcatW(fullPath, exts[i]);
                if (PathFileExistsW(fullPath)) {
                    currentFormatWorks = true;
                    break;
                }
            }
            if (currentFormatWorks) break;
        }

        if (!currentFormatWorks) {
            std::map<std::wstring, int> prefixCounts;
            WIN32_FIND_DATAW findData;
            HANDLE hFind;

            hFind = FindFirstFileW(L"*", &findData); // Find all items in current dir to find subdirs
            if (hFind != INVALID_HANDLE_VALUE) {
                do {
                    // Check if it's a directory and not "." or ".."
                    if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        wcscmp(findData.cFileName, L".") != 0 &&
                        wcscmp(findData.cFileName, L"..") != 0)
                    {
                        std::wstring subDir = findData.cFileName;
                        std::wstring searchPath = subDir + L"\\*";

                        WIN32_FIND_DATAW subFindData;
                        HANDLE hSubFind = FindFirstFileW(searchPath.c_str(), &subFindData);

                        if (hSubFind != INVALID_HANDLE_VALUE) {
                            do {
                                if (!(subFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                    std::wstring filename = subFindData.cFileName;
                                    size_t len = filename.length();
                                    if (len >= 6) {
                                        std::wstring ext = filename.substr(len - 4);
                                        if (ext == L".wav" || ext == L".ogg" || ext == L".mp3") {
                                            if (iswdigit(filename[len - 6]) && iswdigit(filename[len - 5])) {
                                                std::wstring prefixPart = filename.substr(0, len - 6);
                                                std::wstring fullPrefix = subDir + L"\\" + prefixPart;
                                                prefixCounts[fullPrefix]++;
                                            }
                                        }
                                    }
                                }
                            } while (FindNextFileW(hSubFind, &subFindData));
                            FindClose(hSubFind);
                        }
                    }
                } while (FindNextFileW(hFind, &findData));
                FindClose(hFind);
            }

            // Find the most common prefix found in subdirectories
            if (!prefixCounts.empty()) {
                std::wstring bestPrefix = L"";
                int maxCount = 0;
                for (const auto& pair : prefixCounts) {
                    if (pair.second > maxCount) {
                        maxCount = pair.second;
                        bestPrefix = pair.first;
                    }
                }
                if (maxCount > 0) {
                    gPathFormat = bestPrefix + L"%02d";
                }
            }
        }

        gPathFormatDetected = TRUE;
    }

    // Builds a full path for a track file based on the determined format.
    static BOOL BuildTrackPath(int track, wchar_t* out, size_t cch) {
        if (!out || cch == 0) return FALSE;
        out[0] = L'\0';
        InitializeTrackPath();
        const wchar_t* exts[] = { L".wav", L".ogg", L".mp3" };
        wchar_t formatBase[MAX_PATH] = { 0 };
        wsprintfW(formatBase, gPathFormat.c_str(), track);

        // Try appending extensions
        for (int i = 0; i < _countof(exts); ++i) {
            wchar_t fullPath[MAX_PATH];
            lstrcpynW(fullPath, formatBase, MAX_PATH);
            lstrcatW(fullPath, exts[i]);

            if (PathFileExistsW(fullPath)) {
                lstrcpynW(out, fullPath, cch);
                return TRUE;
            }
        }

        return FALSE; // No matching file found
    }

    // Scans 1..99 using BuildTrackPath, considers the 'highest number' found
    static BOOL CountDiscNumTracks(int* outCount) {
        if (!outCount) return FALSE;
        int maxIdx = 0;
        wchar_t tmp[MAX_PATH];
        for (int t = 1; t <= 99; ++t) {
            if (BuildTrackPath(t, tmp, MAX_PATH)) {
                maxIdx = t; // Records the highest index, regardless of continuity
            }
        }
        *outCount = maxIdx;   // 0 if none found
        return TRUE;          // Returns TRUE even if 0
    }

    // Single track length (ms)
    static BOOL CountTrackLengthMs(int track, DWORD* outMs) {
        if (!outMs || track <= 0) return FALSE;

        int maxIdx = 0;
        if (!CountDiscNumTracks(&maxIdx)) return FALSE;
        if (track > maxIdx) return FALSE; // Exceeds max -> fail

        wchar_t path[MAX_PATH];
        if (!BuildTrackPath(track, path, MAX_PATH)) {
            *outMs = 0;  // Non-existent file is 0ms
            return TRUE;
        }

        AudioDecoder dec;
        if (!dec.OpenAuto(path)) { *outMs = 0; return TRUE; } // Open fail is 0ms
        AD_Format f; if (!dec.GetFormat(&f)) { dec.Close(); *outMs = 0; return TRUE; }
        DWORD totalFrames = dec.TotalFrames(); dec.Close();

        if (!f.sampleRate || !totalFrames) { *outMs = 0; return TRUE; }
        *outMs = FramesToMs(totalFrames, f);
        return TRUE;
    }

    // Range length (ms) - sums by ms even if sample rates differ
    static BOOL CountRangeLengthMs(int fromTrack, DWORD fromMs,
        int toTrack, DWORD toMs,
        DWORD* outMs)
    {
        if (!outMs) return FALSE;
        if (fromTrack <= 0 || toTrack <= 0 || toTrack < fromTrack) return FALSE;

        int maxIdx = 0;
        if (!CountDiscNumTracks(&maxIdx)) return FALSE;
        if (fromTrack > maxIdx || toTrack > maxIdx) {
            return FALSE; // Range endpoint exceeds max -> fail
        }

        unsigned __int64 totalMs = 0ULL;

        auto TrackLen = [](int tr, DWORD& msOut)->BOOL {
            return CountTrackLengthMs(tr, &msOut); // Non-existent returns 0ms
            };

        if (fromTrack == toTrack) {
            DWORD tlen = 0;
            if (!TrackLen(fromTrack, tlen)) return FALSE;

            DWORD fm = (fromMs == 0xFFFFFFFF) ? 0 : fromMs;
            DWORD tm = (toMs == 0xFFFFFFFF) ? tlen : toMs;
            if (fm > tlen) fm = tlen;
            if (tm > tlen) tm = tlen;
            if (tm > fm) totalMs = (unsigned __int64)(tm - fm);
        }
        else {
            // First track: fromMs -> end
            {
                DWORD tlen = 0; if (!TrackLen(fromTrack, tlen)) return FALSE;
                DWORD fm = (fromMs == 0xFFFFFFFF) ? 0 : fromMs;
                if (fm > tlen) fm = tlen;
                if (tlen > fm) totalMs += (unsigned __int64)(tlen - fm);
            }
            // Middle tracks: full length (0ms if non-existent)
            for (int t = fromTrack + 1; t < toTrack; ++t) {
                DWORD tlen = 0; if (!TrackLen(t, tlen)) return FALSE;
                totalMs += (unsigned __int64)tlen;
            }
            // Last track: 0 -> toMs
            {
                DWORD tlen = 0; if (!TrackLen(toTrack, tlen)) return FALSE;
                DWORD tm = (toMs == 0xFFFFFFFF) ? tlen : toMs;
                if (tm > tlen) tm = tlen;
                totalMs += (unsigned __int64)tm;
            }
        }

        if (totalMs > 0xFFFFFFFFui64) totalMs = 0xFFFFFFFFui64;
        *outMs = (DWORD)totalMs;
        return TRUE;
    }

    // Combined volume settings to the engine
    static void ApplyVolumeSettings() {
        if (!gInited) return;

        Engine_SetVol(gMasterVol);
        Engine_SetSubVol(gSubVolLeft, gSubVolRight);
        Engine_SetChannelMute(gMuteLeft, gMuteRight);
    }
#ifdef AE_USE_FULLBUFFER
// ====== Playback Mode 1 (Full Buffer, Pre-decode) ======

    // FullBuffer Globals
    static AudioDecoder        gDec;      // Decoder (for format reference)
    static AD_Format           gFmt;
    static BOOL                gFmtInit = FALSE;

    static std::vector<short>  gPcm;      // Pre-buffer
    static DWORD               gFrames = 0; // Total frames in gPcm
    static DWORD               gPos = 0;    // Current frame position in gPcm

    // Segment map (maps concatenated buffer <-> track/section)
    struct FullSeg {
        int   track;
        DWORD trackStartFrame;     // Start frame within the track (absolute)
        DWORD trackEndFrame;       // End frame within the track (absolute, exclusive)
        DWORD concatStartFrame;    // Start frame in the concatenated buffer (absolute)
    };
    static FullSeg gFullSegs[256];
    static int     gFullSegCount = 0;

    static BOOL AppendTrackSlice(int track, DWORD fromMs, DWORD toMs, AD_Format& ioFmt, BOOL& fmtInit, DWORD& totalFramesOut)
    {
        wchar_t path[MAX_PATH];
        if (!BuildTrackPath(track, path, MAX_PATH)) return FALSE;

        AudioDecoder dec;
        if (!dec.OpenAuto(path)) return FALSE;

        AD_Format f; if (!dec.GetFormat(&f)) { dec.Close(); return FALSE; }
        if (f.sampleRate == 0 || (f.channels != 1 && f.channels != 2)) { dec.Close(); return FALSE; }
        if (!fmtInit) { ioFmt = f; fmtInit = TRUE; }
        if (f.sampleRate != ioFmt.sampleRate || f.channels != ioFmt.channels) { dec.Close(); return FALSE; }

        const DWORD ch = f.channels;
        const DWORD totalFrames = dec.TotalFrames(); if (!totalFrames) { dec.Close(); return FALSE; }

        DWORD fromF = MsToFrame(fromMs, f);
        DWORD toF = MsToFrame(toMs, f);
        if (toMs == 0xFFFFFFFF) {
            if (toF < totalFrames) toF = totalFrames;
        }
        if (fromF == 0xFFFFFFFF) fromF = 0;
        if (toF == 0xFFFFFFFF) toF = totalFrames;
        if (fromF > totalFrames) fromF = totalFrames;
        if (toF > totalFrames) toF = totalFrames;
        if (toF <= fromF) { dec.Close(); return FALSE; }

        // Record segment map
        if (gFullSegCount < (int)(sizeof(gFullSegs) / sizeof(gFullSegs[0]))) {
            gFullSegs[gFullSegCount].track = track;
            gFullSegs[gFullSegCount].trackStartFrame = fromF;
            gFullSegs[gFullSegCount].trackEndFrame = toF;
            gFullSegs[gFullSegCount].concatStartFrame = totalFramesOut;
            gFullSegCount++;
        }
        else {
            dec.Close(); return FALSE;
        }

        // Decode and append to the concatenated buffer
        if (!dec.SeekFrames(fromF)) { dec.Close(); return FALSE; }
        const DWORD need = toF - fromF;

        size_t prevSize = gPcm.size();
        gPcm.resize(prevSize + (size_t)need * ch);

        DWORD done = 0;
        while (done < need) {
            DWORD chunk = MinDW(need - done, 32 * 1024);
            DWORD got = dec.ReadFrames(&gPcm[prevSize + (size_t)done * ch], chunk);
            if (!got) break;
            done += got;
        }
        gPcm.resize(prevSize + (size_t)done * ch); // Truncate if read failed
        dec.Close();

        totalFramesOut += done;
        return (done > 0);
    }

    // Update (track, track_ms) from concatenated buffer position gPos
    static void UpdateStatusFromConcat() {
        if (!gFmtInit || gFullSegCount <= 0) { return; }
        // If playback ended (no loop), keep last track/0ms
        if (gAtEnd) return;

        DWORD pos = (gPos < gFrames) ? gPos : (gFrames ? gFrames - 1 : 0);
        for (int i = 0; i < gFullSegCount; ++i) {
            const FullSeg& s = gFullSegs[i];
            DWORD segLen = (s.trackEndFrame - s.trackStartFrame);
            if (pos < s.concatStartFrame + segLen) {
                // Found the segment
                DWORD inSeg = pos - s.concatStartFrame;
                DWORD trackFrame = s.trackStartFrame + inSeg;
                gStatusTrack = s.track;
                gStatusPosMs = FramesToMs(trackFrame, gFmt);
                return;
            }
        }
        // Past the end Report 0ms of the last track
        const FullSeg& last = gFullSegs[gFullSegCount - 1];
        gStatusTrack = last.track;
        gStatusPosMs = 0;
    }

    // Audio Engine Callback (Full Buffer Mode)
    static DWORD WINAPI FillFromMemory(short* out, DWORD frames, void*) {
        if (!gFmtInit || gPcm.empty()) { ZeroMemory(out, frames * 2 * sizeof(short)); return frames; }
        const DWORD ch = gFmt.channels;
        DWORD remain = (gFrames > gPos) ? (gFrames - gPos) : 0;
        DWORD n = MinDW(frames, remain);
        if (n) {
            memcpy(out, &gPcm[(size_t)gPos * ch], n * ch * sizeof(short));
            gPos += n;
        }
        // Update status during playback
        gAtEnd = FALSE;
        UpdateStatusFromConcat();

        if (n < frames) {
            // Reached the end of the buffer
            if (gLoop) {
                DWORD left = frames - n;
                DWORD take = MinDW(left, gFrames); // Take from start
                if (take) {
                    memcpy(out + n * ch, &gPcm[0], take * ch * sizeof(short));
                    gPos = take;
                    n += take;
                    UpdateStatusFromConcat();
                }
                if (n < frames) ZeroMemory(out + n * ch, (frames - n) * ch * sizeof(short));
            }
            else {
                // MCICDA compatibility: At end, report last track / 0ms
                gAtEnd = TRUE;
                if (gFullSegCount > 0) {
                    const FullSeg& last = gFullSegs[gFullSegCount - 1];
                    gStatusTrack = last.track;
                }
                gStatusPosMs = 0;
                ZeroMemory(out + n * ch, (frames - n) * ch * sizeof(short));
            }
        }
        return frames;
    }

    // Main setup function for Full Buffer mode
    static BOOL BuildFullBuffer(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs) {
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = gPos = 0;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;
        gFullSegCount = 0; gAtEnd = FALSE;

        if (toTrack < fromTrack) return FALSE;

        DWORD totalFramesAccum = 0;
        BOOL  firstSegAdded = FALSE;

        for (int t = fromTrack; t <= toTrack; ++t) {
            // First, check track format/existence
            wchar_t path[MAX_PATH];
            if (!BuildTrackPath(t, path, MAX_PATH)) {
                continue; // Not found -> skip
            }
            AudioDecoder probe;
            if (!probe.OpenAuto(path)) {
                continue; // Open failed -> skip
            }
            AD_Format f; if (!probe.GetFormat(&f)) { probe.Close(); continue; }
            DWORD totFrames = probe.TotalFrames();
            probe.Close();
            if (!totFrames) {
                continue; // Empty file -> skip
            }

            if (!gFmtInit) {
                gFmt = f; gFmtInit = TRUE;
            }
            else {
                if (f.sampleRate != gFmt.sampleRate || f.channels != gFmt.channels) {
                    // Format mismatch -> fail
                    return FALSE;
                }
            }

            // Determine slice range
            DWORD useFromMs = 0;
            DWORD useToMs = 0xFFFFFFFF;

            if (!firstSegAdded) {
                // First *existing* track
                if (t == fromTrack) useFromMs = fromMs; // If it's fromTrack, use fromMs
                else                useFromMs = 0;      // Otherwise (e.g. fromTrack was missing), start at 0
            }
            else {
                useFromMs = 0; // Middle tracks are 0
            }

            if (t == toTrack) {
                useToMs = toMs; // If it's toTrack, use toMs
            }
            else {
                useToMs = 0xFFFFFFFF; // Otherwise, use full length
            }

            // Actually append (AppendTrackSlice handles slicing)
            if (AppendTrackSlice(t, useFromMs, useToMs, gFmt, gFmtInit, totalFramesAccum)) {
                firstSegAdded = TRUE;
            }
            // Failures (no file, 0-range) are just skipped
        }

        if (!gFmtInit || gFullSegCount <= 0 || totalFramesAccum == 0) return FALSE;

        gFrames = totalFramesAccum;
        gPos = 0;

        // Status at start (start of the first segment)
        gEverPlayed = TRUE;
        gAtEnd = FALSE;
        gStatusTrack = gFullSegs[0].track;
        gStatusPosMs = FramesToMs(gFullSegs[0].trackStartFrame, gFmt);
        return TRUE;
    }
#endif
#ifndef AE_USE_FULLBUFFER
// ====== Playback Mode 2 (Streaming) ======
    // Streaming Globals
    static AD_Format     gFmt;
    static BOOL          gFmtInit = FALSE;

    struct Segment { int track; DWORD startFrame; DWORD endFrame; };
    static Segment  gSegs[128];
    static int      gSegCount = 0;
    static int      gSegIndex = 0;
    static DWORD    gSegCursor = 0; // Frames read from current segment

    static AudioDecoder gDecCur;       // Current active decoder
    static int          gDecCurTrack = -1; // Track number gDecCur has open

    // Main setup function for Streaming mode
    static BOOL BuildSegments(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs) {
        gSegCount = 0; gSegIndex = 0; gSegCursor = 0;
        gDecCur.Close(); gDecCurTrack = -1;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;

        if (toTrack < fromTrack) return FALSE;

        // Iterate through range, adding only *existing* tracks to segments
        BOOL firstSegAdded = FALSE;

        for (int t = fromTrack; t <= toTrack; ++t) {
            wchar_t path[MAX_PATH];
            if (!BuildTrackPath(t, path, MAX_PATH)) {
                continue; // File not found -> skip
            }

            AudioDecoder dec;
            if (!dec.OpenAuto(path)) {
                continue; // Open failed -> skip
            }

            AD_Format f; if (!dec.GetFormat(&f)) { dec.Close(); continue; }
            DWORD totFrames = dec.TotalFrames();
            dec.Close();
            if (!totFrames) {
                continue; // Empty file (0 frames) -> skip
            }

            if (!gFmtInit) {
                gFmt = f; gFmtInit = TRUE;
            }
            else {
                if (f.sampleRate != gFmt.sampleRate || f.channels != gFmt.channels) {
                    // Format mismatch -> fail
                    return FALSE;
                }
            }

            DWORD startF = 0;
            DWORD endF = totFrames;

            if (!firstSegAdded) {
                // First *existing* track found
                if (t == fromTrack) {
                    // If it's fromTrack, apply fromMs
                    DWORD fm = MsToFrame(fromMs, gFmt);
                    if (fm == 0xFFFFFFFF) fm = 0;
                    if (fm > totFrames) fm = totFrames;
                    startF = fm;
                }
                else {
                    // fromTrack was missing, this is the first valid track
                    startF = 0;
                }
            }
            else {
                // Middle tracks are always 0
                startF = 0;
            }

            if (t == toTrack) {
                // If it's the boundary toTrack, apply toMs
                DWORD tm = MsToFrame(toMs, gFmt);
                if (tm > totFrames) tm = totFrames;
                endF = tm;
            }
            else {
                // Not the boundary, use full length
                endF = totFrames;
            }

            if (endF > startF) {
                // Valid range
                if (gSegCount >= (int)(sizeof(gSegs) / sizeof(gSegs[0]))) return FALSE;
                gSegs[gSegCount++] = { t, startF, endF };
                firstSegAdded = TRUE;
            }
            // else (e.g., fromMs > length), skip this track
        }

        if (!gFmtInit || gSegCount <= 0) return FALSE;

        // Status before play (start of first segment)
        gStatusTrack = gSegs[0].track;
        return TRUE;
    }

    // Opens or seeks the gDecCur decoder to the start of a segment
    static BOOL OpenDecoderAtSegment(const Segment& s) {
        if (gDecCurTrack == s.track) {
            // Already open, just seek
            if (!gDecCur.SeekFrames(s.startFrame)) return FALSE;
            gSegCursor = 0;
            gStatusTrack = s.track;
            return TRUE;
        }
        // Open new track
        gDecCur.Close();
        wchar_t path[MAX_PATH]; if (!BuildTrackPath(s.track, path, MAX_PATH)) return FALSE;
        if (!gDecCur.OpenAuto(path)) return FALSE;
        AD_Format f; if (!gDecCur.GetFormat(&f)) { gDecCur.Close(); return FALSE; }
        if (f.sampleRate == 0 || (f.channels != 1 && f.channels != 2)) { gDecCur.Close(); return FALSE; }
        if (f.sampleRate != gFmt.sampleRate || f.channels != gFmt.channels) { gDecCur.Close(); return FALSE; }
        if (!gDecCur.SeekFrames(s.startFrame)) { gDecCur.Close(); return FALSE; }

        gDecCurTrack = s.track; gSegCursor = 0;
        gStatusTrack = s.track;
        return TRUE;
    }

    // Advances to the next segment
    static BOOL AdvanceSegment() {
        gSegIndex++; gSegCursor = 0;
        if (gSegIndex >= gSegCount) {
            return FALSE; // No more segments
        }
        return OpenDecoderAtSegment(gSegs[gSegIndex]);
    }

    // Audio Engine Callback (Streaming Mode)
    static DWORD WINAPI FillFromStream(short* out, DWORD frames, void*) {
        if (!gFmtInit || gSegCount <= 0) { ZeroMemory(out, frames * 2 * sizeof(short)); return frames; }
        const DWORD ch = gFmt.channels ? gFmt.channels : 2;
        DWORD filled = 0;

        while (filled < frames) {
            if (gSegIndex < 0 || gSegIndex >= gSegCount) {
                // All segments consumed
                if (gLoop) {
                    gSegIndex = 0;
                    if (!OpenDecoderAtSegment(gSegs[gSegIndex])) break; // Loop failed
                }
                else {
                    // MCICDA compatibility: At end, report last track / 0ms
                    const Segment& last = gSegs[gSegCount - 1];
                    gStatusTrack = last.track;
                    gBufferEmpty = TRUE;
                    ZeroMemory(out + filled * ch, (frames - filled) * ch * sizeof(short));
                    return frames;
                }
            }

            const Segment& s = gSegs[gSegIndex];
            DWORD segRemain = (s.endFrame - s.startFrame) - gSegCursor;

            if (!segRemain) {
                // Current segment finished
                if (!AdvanceSegment()) {
                    ZeroMemory(out + filled * ch, (frames - filled) * ch * sizeof(short));
                    return frames;
                }
                continue; // Loop to read from new segment
            }

            DWORD want = MinDW(frames - filled, segRemain);
            DWORD got = gDecCur.ReadFrames(out + filled * ch, want);

            if (!got) {
                // Read failed (EOF), advance segment
                if (!AdvanceSegment()) {
                    ZeroMemory(out + filled * ch, (frames - filled) * ch * sizeof(short));
                    return frames;
                }
                continue;
            }

            filled += got;
            gSegCursor += got;

            // Update status during playback
            DWORD trackFrame = s.startFrame + gSegCursor;
            gStatusTrack = s.track;
        }

        // This should only be hit if the loop was broken
        if (filled < frames) {
            ZeroMemory(out + filled * ch, (frames - filled) * ch * sizeof(short));
        }
        return frames;
    }
#endif // !AE_USE_FULLBUFFER
} // namespace (anonymous)

// ===================== Public API Implementation =====================
namespace AudioEngine {

    BOOL InitializeIfNeeded(HWND initWindow) {
        if (gInited) return TRUE;
        gVistaOrLater = IsVistaOrLater();
        if (!Engine_Initialize(initWindow ? initWindow : GetDesktopWindow())) return FALSE;
        gInited = TRUE;

        // MCICDA compatibility: Initial (pre-play) state is Track=1, Pos=0
        gEverPlayed = FALSE;
        gStatusTrack = 1;

        // Apply cached volume to the engine
        ApplyVolumeSettings();
        return TRUE;
    }

    void Shutdown() {
        if (!gInited) return;
        Engine_Stop();
        Engine_Shutdown();

#ifdef AE_USE_FULLBUFFER
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = gPos = 0;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;
        gFullSegCount = 0;
#else
        gDecCur.Close(); gDecCurTrack = -1;
        gSegCount = 0; gSegIndex = 0; gSegCursor = 0;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;
#endif

        // Reset to MCICDA initial state
        gEverPlayed = FALSE;
        gStatusTrack = 1;

        gCurTrack = -1;
        gInited = FALSE;
    }

    void SetTrackPathFormat(const wchar_t* format) {
        if (format && format[0]) {
            gPathFormat = format;
            gPathFormatDetected = TRUE; // Mark as explicitly set, skip auto-detection
        }
    }

    BOOL PlayTrack(int trackNumber, BOOL loop, HWND notifyHwnd) {
        // PlayRange from start (0) to end (0xFFFFFFFF)
        return PlayRange(trackNumber, 0, trackNumber, 0xFFFFFFFF, loop, notifyHwnd);
    }

    BOOL PlayRange(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs, BOOL loop, HWND notifyHwnd) {
        if (!gInited) { if (!InitializeIfNeeded(notifyHwnd)) return FALSE; }

        // FIXME: if toTrack < fromTrack, it's backward play, but skip for now
        if (fromTrack <= 0 || toTrack <= 0 || toTrack < fromTrack) return FALSE;

        // Handle (fromTrack, 0ms) to (fromTrack, 0ms) -> play whole track
        if ((fromTrack == toTrack) && (fromMs == toMs) && (toMs == 0)) toMs = 0xFFFFFFFF;

        // Check against max track (maintaining previous patch logic)
        {
            int maxIdx = 0;
            if (!CountDiscNumTracks(&maxIdx)) return FALSE;
            if (fromTrack > maxIdx || toTrack > maxIdx) return FALSE;
        }
        Engine_Stop();
#ifndef AE_USE_FULLBUFFER
        gDecCur.Close();
#endif
        gLoop = loop ? TRUE : FALSE;
        gCurTrack = fromTrack;
        gNotify = notifyHwnd;

#ifdef AE_USE_FULLBUFFER
        if (!BuildFullBuffer(fromTrack, fromMs, toTrack, toMs)) return FALSE;
        // Store actual start time and total duration
        gRangeStartMs = (gFullSegCount > 0) ? FramesToMs(gFullSegs[0].trackStartFrame, gFmt) : 0;
        gRangeTotalMs = FramesToMs(gFrames, gFmt); // Total frames in buffer
        if (!Engine_Play(gFmt.sampleRate, gFmt.channels, gLoop, FillFromMemory)) return FALSE;
#else
        if (!BuildSegments(fromTrack, fromMs, toTrack, toMs)) return FALSE;
        // Store actual start time and total duration
        gRangeStartMs = (gSegCount > 0) ? FramesToMs(gSegs[0].startFrame, gFmt) : 0;
        if (!CountRangeLengthMs(fromTrack, fromMs, toTrack, toMs, &gRangeTotalMs)) {
            gRangeTotalMs = 0;
        }
        if (!OpenDecoderAtSegment(gSegs[0])) return FALSE;
        if (!Engine_Play(gFmt.sampleRate, gFmt.channels, gLoop, FillFromStream)) return FALSE;
#endif
        // gRangeTotalMs = (gRangeTotalMs < 256) ? (gRangeTotalMs / 2) : (gRangeTotalMs - 128);
        gEverPlayed = TRUE;

        // Re-apply cached volume/mute state after starting playback
        ApplyVolumeSettings();
        return TRUE;
    }

    void StopAll() {
        // Stops playback. Keeps gStatusTrack/gStatusPosMs cached (for MCI query after stop).
        Engine_Stop();
#ifdef AE_USE_FULLBUFFER
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = gPos = 0;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;
        gFullSegCount = 0;
#else
        gDecCur.Close(); gDecCurTrack = -1;
        gSegCount = 0; gSegIndex = 0; gSegCursor = 0;
        ZeroMemory(&gFmt, sizeof(gFmt)); gFmtInit = FALSE;
#endif
    }

    void Pause() { Engine_Pause(); }
    void Resume() { Engine_Resume(); }

    void SetMasterVolume(float volume01) {
        if (volume01 < 0.0f) volume01 = 0.0f;
        if (volume01 > 1.0f) volume01 = 1.0f;
        gMasterVol = volume01;
        ApplyVolumeSettings();
    }
    float GetMasterVolume() {
        return gMasterVol;
    }
    void SetSubVolume(float volLeft, float volRight) {
        if (volLeft < 0.0f) volLeft = 0.0f; if (volLeft > 1.0f) volLeft = 1.0f;
        if (volRight < 0.0f) volRight = 0.0f; if (volRight > 1.0f) volRight = 1.0f;
        gSubVolLeft = volLeft;
        gSubVolRight = volRight;
        ApplyVolumeSettings();
    }
    void GetSubVolume(float* outVolLeft, float* outVolRight) {
        if (outVolLeft) *outVolLeft = gSubVolLeft;
        if (outVolRight) *outVolRight = gSubVolRight;
    }
    void SetChannelMute(BOOL muteLeft, BOOL muteRight) {
        gMuteLeft = muteLeft;
        gMuteRight = muteRight;
        ApplyVolumeSettings();
    }

    BOOL  IsPlaying() {
        return !HasReachedEnd() && Engine_IsPlaying();
    }
    BOOL  IsPaused() { return Engine_IsPaused(); }

    int   CurrentTrack() {
        // MCICDA rule: 1 before ever played
        if (!gEverPlayed) return 1;
        return gStatusTrack;
    }
    BOOL  SeekTrack(int track) {
        return (1 <= track <= 99) ? gStatusTrack = track : FALSE;
    }

    DWORD CurrentPositionMs() {
        if (!gEverPlayed) return 0;

        // Use HasReachedEnd, which is now accurate
        if (HasReachedEnd()) {
            // MCI rule: 0 at end
            return 0;
        }

        // Calculate absolute position:
        // Start of the first segment + total elapsed time from engine
        DWORD elapsedMs = Engine_PosMs();
        DWORD absoluteMs = gRangeStartMs + elapsedMs;

        // Note: This simple addition may not account for gaps (missing tracks)
        // in a multi-track range. A more complex fix would iterate segments.
        // However, this correctly reports the position relative to the *start*
        // of playback, fixing the latency bug.

        // For simple playback, we can just return elapsedMs relative to the start
        // of the *first track played*.
        return absoluteMs;
    }

    UINT  CurrentSampleRate() { return Engine_SR(); }
    UINT  CurrentChannels() { return Engine_CH(); }

    BOOL GetDiscNumTracks(int* outCount) {
        return CountDiscNumTracks(outCount);
    }

    BOOL GetTrackLengthMs(int track, DWORD* outMs) {
        return CountTrackLengthMs(track, outMs);
    }

    BOOL GetDiscLengthMs(DWORD* outMs) {
        if (!outMs) return FALSE;
        int n = 0; if (!CountDiscNumTracks(&n)) return FALSE;
        unsigned __int64 sum = 0ULL;
        for (int t = 1; t <= n; ++t) {
            DWORD ms = 0;
            if (!CountTrackLengthMs(t, &ms)) return FALSE;
            sum += (unsigned __int64)ms;
        }
        if (sum > 0xFFFFFFFFui64) sum = 0xFFFFFFFFui64;
        *outMs = (DWORD)sum;
        return TRUE;
    }

    BOOL GetRangeLengthMs(int fromTrack, DWORD fromMs,
        int toTrack, DWORD toMs,
        DWORD* outMs)
    {
        return CountRangeLengthMs(fromTrack, fromMs, toTrack, toMs, outMs);
    }

} // namespace (AudioEngine)
