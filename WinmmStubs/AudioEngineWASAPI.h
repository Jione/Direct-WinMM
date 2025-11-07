#pragma once
#include "GlobalDefinitions.h"
#include <mmdeviceapi.h> // WASAPI
#include <audioclient.h> // WASAPI

// PCM Fill Callback: frames = number of "sample frames" (1 frame = a group of 'channels' samples)
typedef DWORD(WINAPI* PcmFillProc)(short* outBuffer, DWORD frames, void* userData);

// ====== WASAPI Audio Streaming Engine ======
// - Assumes 16-bit PCM streaming (S16, interleaved)
// - Playback data is supplied via a user callback
// - Supplies data to the WASAPI buffer using a background thread

class WasapiAudioEngine {
public:
    WasapiAudioEngine();
    ~WasapiAudioEngine();

    // WASAPI Initialize/Shutdown
    BOOL Initialize(HWND initWindow); // initWindow is kept for compatibility, but WASAPI doesn't use it
    void Shutdown();

    // Playback Control (Same interface as DSAudioEngine)
    BOOL PlayStream(UINT sampleRate, UINT channels, PcmFillProc fillProc, void* userData, BOOL loop);
    BOOL PlayStaticBuffer(UINT sampleRate, UINT channels, short* pcmData, DWORD totalFrames, BOOL loop);
    void Stop();
    void Pause();
    void Resume();

    // Volume (0.0f ~ 1.0f)
    void SetVolume(float masterVol); // Sets the master volume
    void SetSubVolume(float volLeft, float volRight); // Sets L/R channel volume relative to master
    void SetChannelMute(BOOL muteLeft, BOOL muteRight); // Mutes channels

    // Status Inquiry
    BOOL IsInitialized() const;
    BOOL IsPlaying() const;
    BOOL IsPaused() const;
    DWORD GetPositionMs() const; // Current position within the buffer (ms)
    UINT  CurrentSampleRate() const;
    UINT  CurrentChannels() const;

private:
    // Internal Helpers
    void ApplyVolumeSettings();
    void ResetState();
    BOOL StartThread();
    void StopThread();
    static DWORD WINAPI ThreadProc(LPVOID ctx);
    void ThreadLoop();
    void CleanupStream(); // Cleans up only stream-related resources
    static DWORD WINAPI FillFromStaticMemory(short* outBuffer, DWORD frames, void* userData);

    // Staging Ring Buffer Helpers
    void StagingFree();
    BOOL StagingInit(UINT sampleRate, UINT channels, UINT seconds);
    UINT32 StagingTopUp(); // produce from fill()
    UINT32 StagingConsume(short* dst, UINT32 wantFrames);

    // Fields
    BOOL                comInited;
    IMMDeviceEnumerator *enumerator;
    IMMDevice           *device;
    IAudioClient        *client;
    IAudioRenderClient  *render;
    ISimpleAudioVolume  *volume;        // Master volume control
    IChannelAudioVolume *channelVolume; // Per-channel control
    IAudioClock         *clock;

    HANDLE              thread;     // Streaming thread
    HANDLE              stopEvent;  // Stop signal event
    volatile LONG       running;    // 1=Playing, 0=Not
    volatile LONG       paused;     // 1=Paused

    // Format/Buffer
    WAVEFORMATEX        wfx;                // The format actually used by the mixer
    UINT32              bufferFrameCount;   // Total size of the WASAPI buffer (in frames)

    // User Callback
    PcmFillProc         fill;
    void                *user;

    // State for static buffer streaming
    short               *staticPcmData;
    DWORD               staticTotalFrames;
    volatile DWORD      staticCurrentFrame; // Must be volatile
    BOOL                staticLoop;

    // Status/Volume
    float               volume01;   // Master Volume Cache
    float               subVolL;    // Sub Volume Left Cache
    float               subVolR;    // Sub Volume Right Cache
    BOOL                muteL;
    BOOL                muteR;
    
    // Staging ring buffer for multi-buffering
    BYTE                *staging;           // contiguous ring storage
    UINT32              stagingBytes;       // total bytes
    UINT32              stagingFrames;      // total frames
    UINT32              stagingWriteFrame;  // producer cursor (frames)
    UINT32              stagingReadFrame;   // consumer cursor (frames)
    UINT32              stagingFillFrames;  // how many frames currently stored
    UINT32              targetPrebufferSec; // desired seconds to hold
};
