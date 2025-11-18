#include "AudioEngineAdapter.h"
#include "AudioEngineDirectSound.h"
#include "AudioEngineWASAPI.h"
#include "AudioEngineWaveOut.h"
#include <samplerate.h>
#include <string>
#include <vector>
#include <map>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

// ===================== Internal Helpers / State =====================
namespace {

    // --- Global State & Engine Abstraction ---
    static BOOL gVistaOrLater = FALSE;

    // Runtime Engine Override Switch
    // 0 = Auto (Default: DS on < Vista, WASAPI on Vista+)
    // 1 = Force DirectSound
    // 2 = Force WASAPI
    static int gEngineOverride = 0;

    // Runtime Buffering Mode Switch
    // 0 = Auto (Default: FullBuffer on DS, Streaming on WASAPI)
    // 1 = Force Streaming
    // 2 = Force Full Buffer
    static int gBufferOverride = 0;

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

    // Internal helper to determine if WASAPI should be used, respecting the override.
    inline BOOL UseWASAPI() {
        if (gEngineOverride == 2) return TRUE;  // Force WASAPI
        if (gEngineOverride != 0) return FALSE; // Force DirectSound or WaveOut
        return gVistaOrLater; // Auto mode
    }

    // Internal helper to determine if WASAPI should be used, respecting the override.
    inline BOOL UseWaveOut() {
        return (gEngineOverride == 3) ? TRUE : FALSE;
    }

    // Internal helper to determine if FullBuffer mode should be used, respecting the override and OS defaults.
    inline BOOL UseFullBuffer() {
        if (gBufferOverride == 1) return FALSE; // Force Streaming
        if (2 <= gBufferOverride) return TRUE;  // Force Full Buffer

        // Auto mode (0):
        // From now on, if it's in Auto mode by default, it works as streaming.
        return FALSE; // Auto mode
    }

    // Internal helper to determine if Resampling mode should be used, respecting FALSE.
    inline BOOL UseResample() {
        return (gBufferOverride == 3) ? TRUE : FALSE;
    }

    // Engine Instances
    static DSoundAudioEngine  gDS;
    static WasapiAudioEngine  gWAS;
    static WaveOutAudioEngine gWav;

    // Engine Abstraction Wrappers
    inline BOOL Engine_Initialize(HWND hwnd) {
        return UseWASAPI() ? gWAS.Initialize(hwnd) : (UseWaveOut() ? gWav.Initialize(hwnd) : gDS.Initialize(hwnd));
    }
    inline void Engine_Shutdown() { UseWASAPI() ? gWAS.Shutdown() : (UseWaveOut() ? gWav.Shutdown() : gDS.Shutdown()); }
    inline BOOL Engine_Play(UINT sr, UINT ch, BOOL loop, DWORD(WINAPI* fill)(short*, DWORD, void*)) {
        return UseWASAPI() ? gWAS.PlayStream(sr, ch, fill, NULL, loop)
            : (UseWaveOut() ? gWav.PlayStream(sr, ch, fill, NULL, loop)
                : gDS.PlayStream(sr, ch, fill, NULL, loop));
    }
    inline BOOL Engine_PlayStatic(UINT sr, UINT ch, short* pcm, DWORD frames, BOOL loop) {
        return UseWASAPI() ? gWAS.PlayStaticBuffer(sr, ch, pcm, frames, loop)
            : (UseWaveOut() ? gWav.PlayStaticBuffer(sr, ch, pcm, frames, loop)
                : gDS.PlayStaticBuffer(sr, ch, pcm, frames, loop));
    }
    inline void Engine_Stop() { UseWASAPI() ? gWAS.Stop() : (UseWaveOut() ? gWav.Stop() : gDS.Stop()); }
    inline void Engine_Pause() { UseWASAPI() ? gWAS.Pause() : (UseWaveOut() ? gWav.Pause() : gDS.Pause()); }
    inline void Engine_Resume() { UseWASAPI() ? gWAS.Resume() : (UseWaveOut() ? gWav.Resume() : gDS.Resume()); }
    inline void Engine_SetVol(float v) { UseWASAPI() ? gWAS.SetVolume(v) : (UseWaveOut() ? gWav.SetVolume(v) : gDS.SetVolume(v)); }
    inline void Engine_SetChannelMute(BOOL l, BOOL r) {
        UseWASAPI() ? gWAS.SetChannelMute(l, r) : (UseWaveOut() ? gWav.SetChannelMute(l, r) : gDS.SetChannelMute(l, r));
    }
    inline void Engine_SetSubVol(float l, float r) {
        UseWASAPI() ? gWAS.SetSubVolume(l, r) : (UseWaveOut() ? gWav.SetSubVolume(l, r) : gDS.SetSubVolume(l, r));
    }
    inline BOOL Engine_IsPlaying() { return UseWASAPI() ? gWAS.IsPlaying() : (UseWaveOut() ? gWav.IsPlaying() : gDS.IsPlaying()); }
    inline BOOL Engine_IsPaused() { return UseWASAPI() ? gWAS.IsPaused() : (UseWaveOut() ? gWav.IsPaused() : gDS.IsPaused()); }
    inline DWORD Engine_PosMs() {
        return UseWASAPI() ? gWAS.GetPositionMs() : (UseWaveOut() ? gWav.GetPositionMs() : gDS.GetPositionMs());
    }
    inline UINT  Engine_SR() {
        return UseWASAPI() ? gWAS.CurrentSampleRate() : (UseWaveOut() ? gWav.CurrentSampleRate() : gDS.CurrentSampleRate());
    }
    inline UINT  Engine_CH() {
        return UseWASAPI() ? gWAS.CurrentChannels() : (UseWaveOut() ? gWav.CurrentChannels() : gDS.CurrentChannels());
    }

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
    static int   gCountTracks = -1;   // Count tracks number for status query
    static int   gStatusTrack = 1;    // Current track number for status query
    static BOOL  gBufferEmpty = FALSE; // Has non-looping playback finished
    static std::wstring gPathFormat = L"music\\Track%02d"; // Default path format base
    static BOOL  gPathFormatDetected = FALSE;              // Flag to run detection only once
    static char  gTrackFileType[100] = { 0 };              // AD_FileType index per track (1..99)
    static DWORD gRangeStartMs = 0;   // The absolute start Ms of the first segment
    static DWORD gRangeTotalMs = 0;   // The total duration of the current range
    static int   gRangeToTrack = 1;   // Store the target TO position for end-of-play status
    static DWORD gRangeToMs = 0;      // Store the target TO position for end-of-play status
    static BOOL  gIsCued = FALSE;     // TRUE if MCI_SEEK was the last command
    static int   gCuedTrack = 1;      // State for MCI_SEEK
    static DWORD gCuedMs = 0;         // State for MCI_SEEK

    // --- General Utilities ---

    static inline DWORD MinDW(DWORD a, DWORD b) { return (a < b) ? a : b; }

    static DWORD MsToFrame(DWORD ms, const AD_Format& f) {
        if (ms == 0xFFFFFFFF) return 0xFFFFFFFF;
        if (f.sampleRate == 0) return 0;
        // Use 64-bit unsigned integers and add 500 for rounding (0.5ms)
        return (DWORD)(((unsigned __int64)ms * (unsigned __int64)f.sampleRate + 500ULL) / 1000ULL);
    }
    static DWORD FramesToMs(DWORD frames, const AD_Format& f) {
        if (f.sampleRate == 0) return 0;
        // Use 64-bit unsigned integers and add half the sample rate for rounding
        unsigned __int64 halfRate = (unsigned __int64)f.sampleRate / 2ULL;
        return (DWORD)(((unsigned __int64)frames * 1000ULL + halfRate) / (unsigned __int64)f.sampleRate);
    }

    // --- Track/Disc Logic (Helpers for Public API) ---
    static void InitializeTrackPath() {
        if (gPathFormatDetected) return; // Already detected

        // Reset track file type index and track count before detection
        ZeroMemory(gTrackFileType, sizeof(gTrackFileType));
        gCountTracks = 0;

        bool currentFormatWorks = false;
        const wchar_t* exts[] = { L".wav", L".ogg", L".mp3", L".flac" };

        // Check if the current gPathFormat works at least for one track
        for (int t = 1; t <= 99; ++t) {
            wchar_t base[MAX_PATH];
            wsprintfW(base, gPathFormat.c_str(), t);
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

                                    size_t dot = filename.find_last_of(L'.');
                                    if (dot == std::wstring::npos) {
                                        continue;
                                    }
                                    std::wstring ext = filename.substr(dot);

                                    if ((lstrcmpiW(ext.c_str(), L".wav") == 0) || (lstrcmpiW(ext.c_str(), L".ogg") == 0) ||
                                        (lstrcmpiW(ext.c_str(), L".mp3") == 0) || (lstrcmpiW(ext.c_str(), L".flac") == 0)) {
                                        if (dot >= 2 && iswdigit(filename[dot - 2]) && iswdigit(filename[dot - 1])) {
                                            std::wstring prefixPart = filename.substr(0, dot - 2);
                                            std::wstring fullPrefix = subDir + L"\\" + prefixPart;
                                            prefixCounts[fullPrefix]++;
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

        // Build track file type index and track count for the final gPathFormat
        int maxTrack = 0;
        for (int t = 1; t <= 99; ++t) {
            wchar_t base[MAX_PATH];
            wsprintfW(base, gPathFormat.c_str(), t);
            for (int i = 0; i < _countof(exts); ++i) {
                wchar_t fullPath[MAX_PATH];
                lstrcpynW(fullPath, base, MAX_PATH);
                lstrcatW(fullPath, exts[i]);

                if (PathFileExistsW(fullPath)) {
                    AD_FileType fileType = AD_File_None;
                    switch (i) {
                    case 0: fileType = AD_File_Wav;  break;
                    case 1: fileType = AD_File_Ogg;  break;
                    case 2: fileType = AD_File_Mp3;  break;
                    case 3: fileType = AD_File_Flac; break;
                    default: fileType = AD_File_None; break;
                    }
                    gTrackFileType[t] = (char)fileType;
                    maxTrack = t;
                    break;
                }
            }
        }
        gCountTracks = maxTrack; // 0 if none found

        gPathFormatDetected = TRUE;
    }

    // Builds a full path for a track file based on the determined format.
    static BOOL BuildTrackPath(int track, wchar_t* out, size_t cch) {
        if (!out || cch == 0) return FALSE;
        out[0] = L'\0';

        if (track <= 0 || track >= 100) return FALSE;

        InitializeTrackPath();

        AD_FileType fileType = AD_File_None;
        if (track >= 0 && track < (int)(_countof(gTrackFileType))) {
            fileType = (AD_FileType)gTrackFileType[track];
        }
        if (fileType == AD_File_None) {
            return FALSE; // No cached file type for this track
        }

        const wchar_t* ext = NULL;
        switch (fileType) {
        case AD_File_Wav:  ext = L".wav";  break;
        case AD_File_Ogg:  ext = L".ogg";  break;
        case AD_File_Mp3:  ext = L".mp3";  break;
        case AD_File_Flac: ext = L".flac"; break;
        default: return FALSE;
        }

        wchar_t formatBase[MAX_PATH] = { 0 };
        wsprintfW(formatBase, gPathFormat.c_str(), track);

        lstrcpynW(out, formatBase, (int)cch);
        int curLen = lstrlenW(out);
        int extLen = lstrlenW(ext);
        if ((size_t)(curLen + extLen + 1) > cch) {
            return FALSE;
        }
        lstrcatW(out, ext);
        return TRUE;
    }

    // Scans 1..99 using BuildTrackPath, considers the 'highest number' found
    static BOOL CountDiscNumTracks(int* outCount) {
        if (!outCount) return FALSE;

        // InitializeTrackPath will build gTrackFileType[] and gCountTracks
        if (!gPathFormatDetected) {
            InitializeTrackPath();
        }

        if (gCountTracks < 0) {
            *outCount = 0;
            return TRUE;
        }

        *outCount = gCountTracks;
        return TRUE; // Returns TRUE even if 0
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

    // ====== Playback Mode 1 (Full Buffer, Pre-decode) ======

    // FullBuffer Globals
    static AudioDecoder        gDec_Full; // Decoder (for format reference)
    static AD_Format           gFmt_Full;
    static BOOL                gFmtInit_Full = FALSE;

    static std::vector<short>  gPcm;      // Pre-buffer
    static DWORD               gFrames = 0; // Total frames in gPcm

    // Segment map (maps concatenated buffer <-> track/section)
    struct FullSeg {
        int   track;
        DWORD trackStartFrame;     // Start frame within the track (absolute)
        DWORD trackEndFrame;       // End frame within the track (absolute, exclusive)
        DWORD concatStartFrame;    // Start frame in the concatenated buffer (absolute)
    };
    static FullSeg gFullSegs[256];
    static int     gFullSegCount = 0;

    static BOOL AppendTrackSlice(int track, DWORD fromMs, DWORD toMs, AD_Format& ioFmt, BOOL& fmtInit, DWORD& totalFramesOut) {
        wchar_t path[MAX_PATH];
        if (!BuildTrackPath(track, path, MAX_PATH)) return FALSE;

        AudioDecoder dec;
        if (!dec.OpenAuto(path)) return FALSE;

        AD_Format f; if (!dec.GetFormat(&f)) { dec.Close(); return FALSE; }
        if (f.sampleRate == 0 || (f.channels != 1 && f.channels != 2)) { dec.Close(); return FALSE; }
        if (!fmtInit) { ioFmt = f; fmtInit = TRUE; }
        if ((!UseResample() && f.sampleRate != ioFmt.sampleRate) || f.channels != ioFmt.channels) { dec.Close(); return FALSE; }

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
        const DWORD needFrames = toF - fromF;
        if (needFrames == 0) { dec.Close(); return FALSE; }

        // --- Resampling Logic ---
        // Read all needed frames into a temporary buffer (native format)
        std::vector<short> tempPcm;
        // Allocates memory. May throw std::bad_alloc if OOM.
        tempPcm.resize(needFrames * ch);

        DWORD done = 0;
        while (done < needFrames) {
            DWORD chunk = MinDW(needFrames - done, 32 * 1024);
            DWORD got = dec.ReadFrames(&tempPcm[done * ch], chunk);
            if (!got) break; // EOF or error
            done += got;
        }
        tempPcm.resize(done * ch); // Truncate to what was actually read
        dec.Close();

        if (done == 0) return FALSE; // Nothing read

        DWORD sourceFrames = done;
        DWORD sourceRate = f.sampleRate;
        DWORD targetRate = ioFmt.sampleRate; // This is gFmt_Full.sampleRate (target)

        // Check if resampling is actually needed
        if (sourceRate == targetRate) {
            // No resampling needed, just append
            size_t prevSize = gPcm.size();
            // Allocates memory. May throw std::bad_alloc if OOM.
            gPcm.resize(prevSize + tempPcm.size());
            memcpy(&gPcm[prevSize], tempPcm.data(), tempPcm.size() * sizeof(short));

            totalFramesOut += sourceFrames;
        }
        else {
            // Resampling is required (e.g., 44100 -> 48000)
            double ratio = (double)targetRate / (double)sourceRate;
            long inputSamples = (long)tempPcm.size();
            // Estimate output size (add a small buffer just in case)
            long outputSamplesEstimate = (long)(inputSamples * ratio) + (ch * 16);

            std::vector<float> floatInput;
            std::vector<float> floatOutput;

            // Allocates memory. May throw std::bad_alloc if OOM.
            floatInput.resize(inputSamples);
            floatOutput.resize(outputSamplesEstimate);

            // Convert short input -> float input
            src_short_to_float_array(tempPcm.data(), floatInput.data(), inputSamples);

            // Prepare data for src_simple
            SRC_DATA srcData;
            srcData.data_in = floatInput.data();
            srcData.data_out = floatOutput.data();
            srcData.input_frames = (long)sourceFrames;
            srcData.output_frames = (long)(outputSamplesEstimate / ch);
            srcData.src_ratio = ratio;
            srcData.end_of_input = 0; // Not end of stream, just this block

            // Use SRC_SINC_FASTEST for speed.
            int error = src_simple(&srcData, SRC_SINC_FASTEST, ch);

            if (error != 0) {
                // Resampling failed
                return FALSE;
            }

            long framesGenerated = srcData.output_frames_gen;
            long samplesGenerated = framesGenerated * ch;
            if (samplesGenerated == 0) return FALSE; // Nothing generated

            // Convert float output -> short output and append
            size_t prevSize = gPcm.size();
            // Allocates memory. May throw std::bad_alloc if OOM.
            gPcm.resize(prevSize + samplesGenerated);
            src_float_to_short_array(floatOutput.data(), &gPcm[prevSize], samplesGenerated);

            totalFramesOut += framesGenerated;
        }

        return TRUE; // Success
    }

    // Update (track) from concatenated buffer position (in frames)
    static void UpdateStatusFromConcat(DWORD posFrames) {
        if (!gFmtInit_Full || gFullSegCount <= 0) { return; }

        DWORD pos = (posFrames < gFrames) ? posFrames : (gFrames ? gFrames - 1 : 0);
        for (int i = 0; i < gFullSegCount; ++i) {
            const FullSeg& s = gFullSegs[i];
            DWORD segLen = (s.trackEndFrame - s.trackStartFrame);
            if (pos < s.concatStartFrame + segLen) {
                // Found the segment
                gStatusTrack = s.track;
                return;
            }
        }
        // Past the end Report last track
        const FullSeg& last = gFullSegs[gFullSegCount - 1];
        gStatusTrack = last.track;
    }

    // Main setup function for Full Buffer mode
    static BOOL BuildFullBuffer(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs) {
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = 0;
        ZeroMemory(&gFmt_Full, sizeof(gFmt_Full)); gFmtInit_Full = FALSE;
        gFullSegCount = 0;

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
                // Cached track type may be stale. Rebuild index once and skip this track.
                gPathFormatDetected = FALSE;
                InitializeTrackPath();
                continue;
            }
            AD_Format f; if (!probe.GetFormat(&f)) { probe.Close(); continue; }
            DWORD totFrames = probe.TotalFrames();
            probe.Close();
            if (!totFrames) {
                continue; // Empty file -> skip
            }

            DWORD targetSampleRate = f.sampleRate;

            // Resample if UseResample is active AND the source rate is not 48k
            if (UseResample() && f.sampleRate != 48000) {
                targetSampleRate = 48000;
            }

            if (!gFmtInit_Full) {
                gFmt_Full = f;
                gFmt_Full.sampleRate = targetSampleRate; // Set the *target* sample rate
                gFmtInit_Full = TRUE;
            }
            else {
                // Check if *channels* match. 
                // Sample rate differences will be handled by AppendTrackSlice.
                if (f.channels != gFmt_Full.channels) {
                    // Format mismatch (channels) -> fail
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
            if (AppendTrackSlice(t, useFromMs, useToMs, gFmt_Full, gFmtInit_Full, totalFramesAccum)) {
                firstSegAdded = TRUE;
            }
            // Failures (no file, 0-range) are just skipped
        }

        if (!gFmtInit_Full || gFullSegCount <= 0 || totalFramesAccum == 0) return FALSE;

        gFrames = totalFramesAccum;

        // Status at start (start of the first segment)
        gEverPlayed = TRUE;
        gStatusTrack = gFullSegs[0].track;

        // Correct gRangeToMs if it was "end of track"
        if (gFullSegCount > 0) {
            const FullSeg& last = gFullSegs[gFullSegCount - 1];
            DWORD lastMs = FramesToMs(last.trackEndFrame, gFmt_Full);
            if (lastMs < gRangeToMs) {
                gRangeToMs = lastMs;
            }
        }
        return TRUE;
    }

    // ====== Playback Mode 2 (Streaming) ======
    // Streaming Globals
    static AD_Format     gFmt_Stream;
    static BOOL          gFmtInit_Stream = FALSE;

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
        ZeroMemory(&gFmt_Stream, sizeof(gFmt_Stream)); gFmtInit_Stream = FALSE;

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
                // Cached track type may be stale. Rebuild index once and skip this track.
                gPathFormatDetected = FALSE;
                InitializeTrackPath();
                continue;
            }

            AD_Format f; if (!dec.GetFormat(&f)) { dec.Close(); continue; }
            DWORD totFrames = dec.TotalFrames();
            dec.Close();
            if (!totFrames) {
                continue; // Empty file (0 frames) -> skip
            }

            if (!gFmtInit_Stream) {
                gFmt_Stream = f; gFmtInit_Stream = TRUE;
            }
            else {
                if (f.sampleRate != gFmt_Stream.sampleRate || f.channels != gFmt_Stream.channels) {
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
                    DWORD fm = MsToFrame(fromMs, gFmt_Stream);
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
                DWORD tm = MsToFrame(toMs, gFmt_Stream);
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

        if (!gFmtInit_Stream || gSegCount <= 0) return FALSE;

        // Status before play (start of first segment)
        gStatusTrack = gSegs[0].track;

        // Correct gRangeToMs if it was "end of track"
        if (gSegCount > 0) {
            const Segment& last = gSegs[gSegCount - 1];
            DWORD lastMs = FramesToMs(last.endFrame, gFmt_Stream);
            if (lastMs < gRangeToMs) {
                gRangeToMs = lastMs;
            }
        }
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
        if (f.sampleRate != gFmt_Stream.sampleRate || f.channels != gFmt_Stream.channels) { gDecCur.Close(); return FALSE; }
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
        if (!gFmtInit_Stream || gSegCount <= 0) { ZeroMemory(out, frames * 2 * sizeof(short)); return frames; }
        const DWORD ch = gFmt_Stream.channels ? gFmt_Stream.channels : 2;
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

} // namespace (anonymous)

// ===================== Public API Implementation =====================
namespace AudioEngine {

    BOOL InitializeIfNeeded(HWND initWindow) {
        if (gInited) return TRUE;
        gVistaOrLater = IsVistaOrLater();
        dprintf("Initialize audio with %s engine (Buffer: %s)",
            (UseWASAPI() ? "WASAPI" : UseWaveOut() ? "WaveOut" : "DirectSound"), (UseFullBuffer() ? "Full" : "Streaming"));
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

        // --- Clear Full Buffer State ---
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = 0;
        ZeroMemory(&gFmt_Full, sizeof(gFmt_Full)); gFmtInit_Full = FALSE;
        gFullSegCount = 0;

        // --- Clear Streaming State ---
        gDecCur.Close(); gDecCurTrack = -1;
        gSegCount = 0; gSegIndex = 0; gSegCursor = 0;
        ZeroMemory(&gFmt_Stream, sizeof(gFmt_Stream)); gFmtInit_Stream = FALSE;

        // Reset to MCICDA initial state
        gEverPlayed = FALSE;
        gStatusTrack = 1;
        gCurTrack = -1;
        gRangeToTrack = 1;
        gRangeToMs = 0;

        gInited = FALSE;
    }

    void SetTrackPathFormat(const wchar_t* format) {
        if (format && format[0]) {
            gPathFormat = format;
            // Reset detection so that track index and count are rebuilt lazily.
            gPathFormatDetected = FALSE;
            gCountTracks = -1;
            ZeroMemory(gTrackFileType, sizeof(gTrackFileType));
        }
    }

    // Set the buffering strategy (0=Auto, 1=Streaming, 2=Full, 3=Full Resampling)
    void SetBufferMode(int mode) {
        if (mode < 0 || mode > 3) mode = 0;
        if ((mode == gBufferOverride) || ((mode == 0) && (gBufferOverride == 1)) || ((mode == 0) && (gBufferOverride == 1)))
            return;

        // Check if the *effective* mode (what's currently running) will change
        BOOL prevMode = UseFullBuffer();
        BOOL newMode = (mode > 1) ? TRUE : FALSE;

        // If the effective mode changed AND the engine was already initialized, shut it down.
        if (gInited && (prevMode != newMode)) {
            Shutdown(); // This stops playback and sets gInited = FALSE
        }
        
        gBufferOverride = mode;
    }

    // Sets the audio engine override.
    void SetEngineOverride(int mode) {
        if (!gInited) gVistaOrLater = IsVistaOrLater();
        if (mode < 0 || mode > 3) mode = 0;
        if (mode == gEngineOverride) return;
        
        int prevEngine = gEngineOverride;
        int newEngine = mode;
        if (gEngineOverride == 0) prevEngine = gVistaOrLater ? 2 : 1;
        if (mode == 0) newEngine = gVistaOrLater ? 2 : 1;
        if (prevEngine == newEngine) return;
        
        if (gInited) Shutdown();

        gEngineOverride = mode;
    }

    BOOL PlayTrack(int trackNumber, BOOL loop, HWND notifyHwnd) {
        // PlayRange from start (0) to end (0xFFFFFFFF)
        return PlayRange(trackNumber, 0, trackNumber, 0xFFFFFFFF, loop, notifyHwnd);
    }

    BOOL PlayRange(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs, BOOL loop, HWND notifyHwnd) {
        gIsCued = FALSE;
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

        Engine_Stop(); // Stop any current playback

        // Clear previous streaming state (if any)
        gDecCur.Close(); gDecCurTrack = -1;

        // Clear previous fullbuffer state (if any)
        gPcm.clear(); gPcm.shrink_to_fit();

        gLoop = loop ? TRUE : FALSE;
        gCurTrack = fromTrack;
        gNotify = notifyHwnd;

        gRangeToTrack = toTrack;
        gRangeToMs = toMs;

        // Runtime switching based on the flag
        if (UseFullBuffer()) {
            // --- Full Buffer Mode ---
            if (!BuildFullBuffer(fromTrack, fromMs, toTrack, toMs)) return FALSE;
            // Store actual start time and total duration
            gRangeStartMs = (gFullSegCount > 0) ? FramesToMs(gFullSegs[0].trackStartFrame, gFmt_Full) : 0;
            gRangeTotalMs = FramesToMs(gFrames, gFmt_Full); // Total frames in buffer

            // Call static buffer playback
            if (!Engine_PlayStatic(gFmt_Full.sampleRate, gFmt_Full.channels, gPcm.data(), gFrames, gLoop)) return FALSE;
        }
        else {
            // --- Streaming Mode ---
            if (!BuildSegments(fromTrack, fromMs, toTrack, toMs)) return FALSE;
            // Store actual start time and total duration
            gRangeStartMs = (gSegCount > 0) ? FramesToMs(gSegs[0].startFrame, gFmt_Stream) : 0;
            if (!CountRangeLengthMs(fromTrack, fromMs, toTrack, toMs, &gRangeTotalMs)) {
                gRangeTotalMs = 0;
            }
            if (!OpenDecoderAtSegment(gSegs[0])) return FALSE;
            if (!Engine_Play(gFmt_Stream.sampleRate, gFmt_Stream.channels, gLoop, FillFromStream)) return FALSE;
        }

        // gRangeTotalMs = (gRangeTotalMs < 256) ? (gRangeTotalMs / 2) : (gRangeTotalMs - 128);
        gEverPlayed = TRUE;

        // Re-apply cached volume/mute state after starting playback
        ApplyVolumeSettings();
        return TRUE;
    }

    void StopAll() {
        int lastTrack = 1;
        DWORD lastMs = 0;

        if (gEverPlayed && !gIsCued) {
            GetCurrentTrackPosition(&lastTrack, &lastMs);
        }
        else if (gIsCued) {
            lastTrack = gCuedTrack;
            lastMs = gCuedMs;
        }
        else if (gEverPlayed) {
            lastTrack = gStatusTrack;
            lastMs = 0;
        }

        Engine_Stop();

        // --- Clear Full Buffer State ---
        gPcm.clear(); gPcm.shrink_to_fit(); gFrames = 0;
        ZeroMemory(&gFmt_Full, sizeof(gFmt_Full)); gFmtInit_Full = FALSE;
        gFullSegCount = 0;

        // --- Clear Streaming State ---
        gDecCur.Close(); gDecCurTrack = -1;
        gSegCount = 0; gSegIndex = 0; gSegCursor = 0;
        ZeroMemory(&gFmt_Stream, sizeof(gFmt_Stream)); gFmtInit_Stream = FALSE;

        // --- Clear Range State ---
        gRangeToTrack = 1; gRangeToMs = 0;
        gRangeTotalMs = 0; // Clear total range

        // Set the Cued state to the last known position
        gIsCued = TRUE;
        gCuedTrack = lastTrack;
        gCuedMs = lastMs;

        // Update global tracks to match the cued position
        gCurTrack = lastTrack;
        gStatusTrack = lastTrack;
        gRangeStartMs = lastMs;
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
    BOOL HasReachedEnd() {
        if (gLoop || !gEverPlayed || gRangeTotalMs == 0) return FALSE;

        DWORD elapsedMs = Engine_PosMs();
        return (elapsedMs >= gRangeTotalMs);
    }
    BOOL IsPlaying() {
        return !HasReachedEnd() && Engine_IsPlaying();
    }
    BOOL IsPaused() { return Engine_IsPaused(); }

    int  CurrentTrack() {
        // MCICDA rule: 1 before ever played
        if (!gEverPlayed) return 1;
        return gStatusTrack;
    }
    BOOL Seek(int toTrack, DWORD toMs) {
        StopAll(); // Stop playback and clear buffers

        gIsCued = TRUE;
        gCuedTrack = toTrack;
        gCuedMs = toMs;

        // Set state so STATUS reports correctly and PLAY starts here
        gCurTrack = toTrack;
        gStatusTrack = toTrack;
        gRangeStartMs = toMs; // Store relative Ms
        gEverPlayed = TRUE;   // Mark as "ready"

        // Ensure range TO matches cue position
        gRangeToTrack = toTrack;
        gRangeToMs = toMs;

        return TRUE;
    }

    DWORD CurrentPositionMs() {
        if (!gEverPlayed) return 0;

        // Get raw elapsed time from engine
        DWORD elapsedMs = Engine_PosMs();

        // Check for loop or range validity
        if (gLoop || !gEverPlayed || gRangeTotalMs == 0) {
            // Looping or range not set, just return raw elapsed time
            if (UseFullBuffer() && gFmtInit_Full) {
                UpdateStatusFromConcat(MsToFrame(elapsedMs, gFmt_Full));
            }
            return elapsedMs;
        }

        // --- Non-looping playback ---
        if (elapsedMs >= gRangeTotalMs) {
            // Playback has finished. Return the *total duration*
            // of the range that was played.
            if (UseFullBuffer() && gFmtInit_Full) {
                UpdateStatusFromConcat(gFrames); // Update status to last frame
            }
            else if (gFmtInit_Stream && gSegCount > 0) {
                // Ensure status track is set to the last segment
                gStatusTrack = gSegs[gSegCount - 1].track;
            }
            return gRangeTotalMs;
        }

        // Still playing
        if (UseFullBuffer() && gFmtInit_Full) {
            UpdateStatusFromConcat(MsToFrame(elapsedMs, gFmt_Full));
        }

        // Return raw elapsed time
        return elapsedMs;
    }

    UINT  CurrentSampleRate() { return Engine_SR(); }
    UINT  CurrentChannels() { return Engine_CH(); }

    BOOL GetDiscNumTracks(int* outCount) {
        return CountDiscNumTracks(outCount);
    }

    BOOL GetTrackLengthMs(int track, DWORD* outMs) {
        return CountTrackLengthMs(track, outMs);
    }

    BOOL GetCurrentTrackPosition(int* outTrack, DWORD* outRelativeMs) {
        if (!outTrack || !outRelativeMs) return FALSE;

        // Check if cued by MCI_SEEK and currently stopped
        if (gIsCued && !IsPlaying() && !IsPaused()) {
            *outTrack = gCuedTrack;
            *outRelativeMs = gCuedMs;
            return TRUE;
        }

        // Get the current absolute elapsed time since playback started (0...gRangeTotalMs).
        DWORD absoluteElapsedMs = CurrentPositionMs();

        // Handle "Stopped" (manually) or "Not Started"
        if (absoluteElapsedMs == 0 && !IsPlaying() && !IsPaused()) {
            *outTrack = (gCurTrack > 0) ? gCurTrack : 1;
            *outRelativeMs = 0;
            return TRUE;
        }

        // Handle "Finished" state
        if (absoluteElapsedMs >= gRangeTotalMs && !gLoop) {
            *outTrack = gRangeToTrack;
            *outRelativeMs = gRangeToMs;
            return TRUE;
        }

        // --- Playback is active or has finished ---
        if (UseFullBuffer()) {
            // --- Full Buffer Mode ---
            if (!gFmtInit_Full || gFullSegCount == 0) return FALSE;

            DWORD elapsedFrames = MsToFrame(absoluteElapsedMs, gFmt_Full);
            if (elapsedFrames > gFrames) elapsedFrames = gFrames; // Clamp to end

            for (int i = 0; i < gFullSegCount; ++i) {
                const FullSeg& s = gFullSegs[i];
                DWORD segLenFrames = (s.trackEndFrame - s.trackStartFrame);

                // Check if the position falls within this segment
                // (Use <= for the last segment to catch the exact end time)
                if (elapsedFrames <= s.concatStartFrame + segLenFrames) {
                    *outTrack = s.track;
                    DWORD framesIntoSegment = elapsedFrames - s.concatStartFrame;
                    // Convert back to relative Ms *within the track*
                    *outRelativeMs = FramesToMs(s.trackStartFrame + framesIntoSegment, gFmt_Full);
                    return TRUE;
                }
            }
            // If somehow past the end, report end of last segment
            const FullSeg& last = gFullSegs[gFullSegCount - 1];
            *outTrack = last.track;
            *outRelativeMs = FramesToMs(last.trackEndFrame, gFmt_Full);
            return TRUE;
        }
        else {
            // --- Streaming Mode ---
            if (!gFmtInit_Stream || gSegCount == 0) return FALSE;

            DWORD elapsedMsAccumulator = 0;

            for (int i = 0; i < gSegCount; ++i) {
                const Segment& s = gSegs[i];
                DWORD segDurationMs = FramesToMs(s.endFrame - s.startFrame, gFmt_Stream);

                // Check if the elapsed time falls within this segment
                // (Use <= for the last segment to catch the exact end time)
                if (absoluteElapsedMs <= elapsedMsAccumulator + segDurationMs) {
                    *outTrack = s.track;
                    DWORD msIntoSegment = absoluteElapsedMs - elapsedMsAccumulator;
                    // Convert to relative Ms *within the track*
                    *outRelativeMs = FramesToMs(s.startFrame, gFmt_Stream) + msIntoSegment;
                    return TRUE;
                }
                elapsedMsAccumulator += segDurationMs;
            }
            // If past the end, report end of last segment
            const Segment& last = gSegs[gSegCount - 1];
            *outTrack = last.track;
            *outRelativeMs = FramesToMs(last.endFrame, gFmt_Stream);
            return TRUE;
        }
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
