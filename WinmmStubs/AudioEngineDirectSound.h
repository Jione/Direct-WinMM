#pragma once
#include "GlobalDefinitions.h"
#include <dsound.h>

// ====== DirectSound Audio Streaming Engine ======
// - Assumes 16-bit PCM streaming (S16, interleaved)
// - Playback data is supplied via a user callback
// - Operates using a double-buffered ring buffer (loop + background thread)

// PCM Fill Callback: frames = number of "sample frames" (1 frame = a group of 'channels' samples)
typedef DWORD(WINAPI* PcmFillProc)(short* outBuffer, DWORD frames, void* userData);

class DSoundAudioEngine {
public:
    DSoundAudioEngine();
    ~DSoundAudioEngine();

    // DirectSound Initialize/Shutdown
    BOOL Initialize(HWND initWindow); // initWindow: A message window is recommended instead of NULL to allow sound playback even when the app lacks focus
    void Shutdown();

    // Playback Control
    // Specify sample rate/channels (16-bit fixed)
    // The callback is invoked periodically from the engine thread to fill the PCM buffer (silence is added if 0 is returned)
    BOOL PlayStream(UINT sampleRate, UINT channels, PcmFillProc fillProc, void* userData, BOOL loop);
    void Stop();            // Stops immediately (cleans up buffer/thread)
    void Pause();           // Pauses playback (replacement for DSBPLAY_PAUSE: stops + preserves cursor)
    void Resume();          // Resumes from pause

    // Volume (0.0f ~ 1.0f)
    void SetVolume(float masterVol); // Sets the overall volume level
    void SetSubVolume(float volLeft, float volRight); // Sets L/R balance via Pan
    void SetChannelMute(BOOL muteLeft, BOOL muteRight); // Overrides volume/pan

    // Status Inquiry
    BOOL IsInitialized() const;
    BOOL IsPlaying() const;
    BOOL IsPaused() const;
    DWORD GetPositionMs() const;    // Approximate playback position (ms)
    UINT  CurrentSampleRate() const;
    UINT  CurrentChannels() const;

private:
    void ResetState();
    BOOL CreatePrimary();
    BOOL CreateSecondary(UINT sampleRate, UINT channels);
    void DestroySecondary();
    BOOL StartThread();
    void StopThread();
    static DWORD WINAPI ThreadProc(LPVOID ctx);
    void ThreadLoop();

    // Ring Buffer Helpers
    void FillInitial();
    void RefillIfNeeded();

    // Fields
    IDirectSound8       *ds; // DirectSound8
    IDirectSoundBuffer  *primary;
    IDirectSoundBuffer  *secondary;

    HANDLE              thread; // Streaming thread
    HANDLE              stopEvent; // Stop signal event
    volatile LONG       running; // 1=Playing, 0=Not
    volatile LONG       paused; // 1=Paused

    // Format/Buffer
    WAVEFORMATEX        wfx;
    DWORD               bufferBytes; // Total size of the secondary buffer
    DWORD               halfBytes; // Half size (for double buffering)
    DWORD               blockBytes; // Write unit (safe block)
    DWORD               writeCursor; // The offset where write next
    DWORD               lastPlayCursor;
    ULONGLONG           totalBytesPlayed;

    // User Callback
    PcmFillProc         fill;
    void                *user;
    BOOL                loop;

    // Status/Volume
    float               volume01;   // Master Volume
    float               subVolL;
    float               subVolR;
    BOOL                muteL;
    BOOL                muteR;
    DWORD               approxMs; // Approximate position (ms)
    UINT                samplerate;
    UINT                channels;

    HWND                hwnd; // Target for SetCooperativeLevel
};
