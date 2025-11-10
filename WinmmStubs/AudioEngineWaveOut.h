#pragma once
#include "GlobalDefinitions.h"

#ifndef WAVEOUT_RING_BLOCKS
#define WAVEOUT_RING_BLOCKS 16    // 16 blocks queued
#endif
#ifndef WAVEOUT_BLOCK_MS
#define WAVEOUT_BLOCK_MS    75    // 75 ms per block
#endif

// ====== waveOut Audio Streaming Engine ======
// - Assumes 16-bit PCM streaming (S16, interleaved)
// - Playback data is supplied via a user callback
// - Uses a background thread to feed waveOut with small blocks

// PCM Fill Callback: frames = number of "sample frames" (1 frame = a group of 'channels' samples)
typedef DWORD(WINAPI* PcmFillProc)(short* outBuffer, DWORD frames, void* userData);

class WaveOutAudioEngine {
public:
    WaveOutAudioEngine();
    ~WaveOutAudioEngine();

    // Initialize/Shutdown
    BOOL Initialize(HWND initWindow); // initWindow kept for signature parity (unused)
    void Shutdown();

    // Playback Control (same public API as other engines)
    BOOL PlayStream(UINT sampleRate, UINT channels, PcmFillProc fillProc, void* userData, BOOL loop);
    BOOL PlayStaticBuffer(UINT sampleRate, UINT channels, short* pcmData, DWORD totalFrames, BOOL loop);
    void Stop();
    void Pause();
    void Resume();

    // Volume (0.0f ~ 1.0f)
    void SetVolume(float masterVol);
    void SetSubVolume(float volLeft, float volRight);
    void SetChannelMute(BOOL muteLeft, BOOL muteRight);

    // Status Inquiry
    BOOL IsInitialized() const;
    BOOL IsPlaying() const;
    BOOL IsPaused() const;
    DWORD GetPositionMs() const;
    UINT  CurrentSampleRate() const;
    UINT  CurrentChannels() const;

private:
    // Internal helpers
    void ResetState();
    BOOL StartThread();
    void StopThread();
    static DWORD WINAPI ThreadProc(LPVOID ctx);
    void ThreadLoop();

    // Staging Ring Buffer Helpers (same concept as other engines)
    void StagingFree();
    BOOL StagingInit(UINT sampleRate, UINT channels, UINT seconds);
    DWORD StagingTopUp(); // produce from fill()
    DWORD StagingConsume(short* dst, DWORD wantFrames);

    // Static buffer adapter
    static DWORD WINAPI FillFromStaticMemory(short* outBuffer, DWORD frames, void* userData);

    // Software volume/pan/mute on interleaved S16
    void ApplyGainPan(short* samples, DWORD frames);

    // waveOut callback
    static void CALLBACK WaveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);

    // Fields
    HWND                hwnd;
    HANDLE              thread;
    HANDLE              stopEvent;
    HANDLE              notifyEvent; // set by callback when a block completes
    volatile LONG       running;     // 1=Playing, 0=Not
    volatile LONG       paused;      // 1=Paused

    // waveOut device and buffers
    HWAVEOUT            hwo;

    enum { RING_BLOCKS = WAVEOUT_RING_BLOCKS };; // number of queued headers
    WAVEHDR             headers[RING_BLOCKS];
    BYTE* blocks[RING_BLOCKS];
    DWORD               blockBytes;  // bytes per block (aligned)
    volatile LONG       inUse[RING_BLOCKS]; // 0=free, 1=in-queue

    // Format/Buffer
    WAVEFORMATEX        wfx;
    UINT                samplerate;
    UINT                channels;

    // Position accounting
    ULONGLONG           totalBytesPlayed; // updated on WOM_DONE

    // User Callback
    PcmFillProc         fill;
    void* user;
    BOOL                loop;

    // Static source state
    short* staticPcmData;
    DWORD               staticTotalFrames;
    DWORD               staticCurrentFrame;

    // Volume/mute state (software)
    float               volume01;
    float               subVolL;
    float               subVolR;
    BOOL                muteL;
    BOOL                muteR;

    // Staging ring buffer (seconds of audio)
    BYTE* staging;
    DWORD               stagingBytes;
    DWORD               stagingFrames;
    DWORD               stagingWriteFrame;
    DWORD               stagingReadFrame;
    DWORD               stagingFillFrames;
    DWORD               targetPrebufferSec; // default 16s
};
