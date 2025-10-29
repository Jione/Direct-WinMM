#pragma once
#include "GlobalDefinitions.h"
#include "AudioDecoder.h"

// Selects audio engine based on OS version
// XP = DirectSound, Vista+ = WASAPI
// Define AE_USE_FULLBUFFER for full pre-decoding, otherwise streaming
namespace AudioEngine {

    // Initialization/Shutdown (lazy init)
    BOOL  InitializeIfNeeded(HWND initWindow);
    void  Shutdown();

    // Sets a custom format string for locating track files
    void  SetTrackPathFormat(const wchar_t* format);

    // Playback Control
    BOOL  PlayTrack(int trackNumber, BOOL loop, HWND notifyHwnd);
    BOOL  PlayRange(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs, BOOL loop, HWND notifyHwnd);

    void  StopAll();
    void  Pause();
    void  Resume();
    void  SetMasterVolume(float vol); // 0.0~1.0
    float GetMasterVolume();
    void  SetSubVolume(float volLeft, float volRight); // 0.0~1.0
    void  GetSubVolume(float* outVolLeft, float* outVolRight);
    void  SetChannelMute(BOOL muteLeft, BOOL muteRight);

    // Status
    BOOL  IsPlaying();
    BOOL  IsPaused();
    int   CurrentTrack();        // MCICDA compatibility rule
    BOOL  SeekTrack(int track);  // MCICDA compatibility rule
    DWORD CurrentPositionMs();   // MCICDA compatibility rule
    UINT  CurrentSampleRate();
    UINT  CurrentChannels();

    // Total number of tracks (MCI_STATUS_NUMBER_OF_TRACKS)
    BOOL  GetDiscNumTracks(int* outCount);

    // Total length of a specific track (ms)
    BOOL  GetTrackLengthMs(int track, DWORD* outMs);

    // Total disc length (ms): Sum of Track01..TrackN
    BOOL  GetDiscLengthMs(DWORD* outMs);

    // Range length (ms): fromTrack:fromMs ~ toTrack:toMs
    //  - toMs==0xFFFFFFFF -> end of that track
    BOOL  GetRangeLengthMs(int fromTrack, DWORD fromMs, int toTrack, DWORD toMs, DWORD* outMs);
}
