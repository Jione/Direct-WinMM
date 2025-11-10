#include "AudioEngineWaveOut.h"
#include <mmreg.h>
#include <string.h>
#include <math.h>
#include <cstdlib>

static inline DWORD AlignDownU32(DWORD v, DWORD align) { return (align == 0) ? v : (v / align) * align; }
static inline LONG  ClampS32(LONG v, LONG lo, LONG hi) { if (v < lo) return lo; if (v > hi) return hi; return v; }

WaveOutAudioEngine::WaveOutAudioEngine()
    : hwnd(NULL), thread(NULL), stopEvent(NULL), notifyEvent(NULL), running(0), paused(0),
    hwo(NULL), blockBytes(0), samplerate(0), channels(0), totalBytesPlayed(0),
    fill(NULL), user(NULL), loop(FALSE), staticPcmData(NULL), staticTotalFrames(0), staticCurrentFrame(0),
    volume01(1.0f), subVolL(1.0f), subVolR(1.0f), muteL(FALSE), muteR(FALSE),
    staging(NULL), stagingBytes(0), stagingFrames(0), stagingWriteFrame(0), stagingReadFrame(0), stagingFillFrames(0),
    targetPrebufferSec(16)
{
    ZeroMemory(&wfx, sizeof(wfx));
    for (int i = 0; i < RING_BLOCKS; ++i) { blocks[i] = NULL; ZeroMemory(&headers[i], sizeof(WAVEHDR)); inUse[i] = 0; }
}

WaveOutAudioEngine::~WaveOutAudioEngine() {
    Shutdown();
}

BOOL WaveOutAudioEngine::Initialize(HWND initWindow) {
    hwnd = initWindow ? initWindow : GetDesktopWindow();
    if (!stopEvent) stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!notifyEvent) notifyEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    return (stopEvent != NULL) && (notifyEvent != NULL);
}

void WaveOutAudioEngine::Shutdown() {
    Stop();
    if (notifyEvent) { CloseHandle(notifyEvent); notifyEvent = NULL; }
    if (stopEvent) { CloseHandle(stopEvent);   stopEvent = NULL; }
    ResetState();
}

void WaveOutAudioEngine::ResetState() {
    volume01 = 1.0f; subVolL = 1.0f; subVolR = 1.0f; muteL = FALSE; muteR = FALSE;
    samplerate = 0; channels = 0; totalBytesPlayed = 0;
    StagingFree();
    ZeroMemory(&wfx, sizeof(wfx));
}

void WaveOutAudioEngine::StagingFree() {
    if (staging) { free(staging); staging = NULL; }
    stagingBytes = stagingFrames = stagingWriteFrame = stagingReadFrame = stagingFillFrames = 0;
}

BOOL WaveOutAudioEngine::StagingInit(UINT sampleRate, UINT ch, UINT seconds) {
    StagingFree();
    if (seconds < 3) seconds = 3; if (seconds > 30) seconds = 30;
    DWORD frames = sampleRate * seconds;
    DWORD bytes = frames * (ch * sizeof(short));
    if (wfx.nBlockAlign) bytes = AlignDownU32(bytes, wfx.nBlockAlign);
    if (bytes == 0) return FALSE;
    staging = (BYTE*)malloc(bytes);
    if (!staging) return FALSE;
    stagingBytes = bytes;
    stagingFrames = bytes / wfx.nBlockAlign;
    stagingWriteFrame = stagingReadFrame = stagingFillFrames = 0;
    return TRUE;
}

DWORD WaveOutAudioEngine::StagingTopUp() {
    if (!fill || !staging || stagingFrames == 0) return 0;
    DWORD produced = 0;
    DWORD targetFill = (stagingFrames * 9) / 10;

    while (stagingFillFrames < targetFill) {
        DWORD chunk = (samplerate / 2); // ~0.5s
        if (chunk < 256) chunk = 256;
        DWORD freeFrames = stagingFrames - stagingFillFrames;
        if (freeFrames == 0) break;
        if (chunk > freeFrames) chunk = freeFrames;

        DWORD w = stagingWriteFrame;
        DWORD canCont = stagingFrames - w;
        DWORD first = (chunk <= canCont) ? chunk : canCont;
        DWORD second = chunk - first;

        short* dst1 = (short*)(staging + (w * wfx.nBlockAlign));
        DWORD got1 = 0;
        if (first) {
            got1 = fill(dst1, first, user);
            if (got1 < first) {
                ZeroMemory(dst1 + (got1 * wfx.nChannels), (first - got1) * wfx.nBlockAlign);
            }
        }
        if (second) {
            short* dst2 = (short*)staging;
            DWORD got2 = fill(dst2, second, user);
            if (got2 < second) {
                ZeroMemory(dst2 + (got2 * wfx.nChannels), (second - got2) * wfx.nBlockAlign);
            }
        }

        stagingWriteFrame = (stagingWriteFrame + chunk) % stagingFrames;
        stagingFillFrames += chunk;
        produced += chunk;
        if (chunk < (samplerate / 8)) break;
    }
    return produced;
}

DWORD WaveOutAudioEngine::StagingConsume(short* dst, DWORD wantFrames) {
    if (!staging || stagingFillFrames == 0) {
        ZeroMemory(dst, wantFrames * wfx.nBlockAlign);
        return wantFrames;
    }
    DWORD take = (wantFrames <= stagingFillFrames) ? wantFrames : stagingFillFrames;

    DWORD r = stagingReadFrame;
    DWORD canCont = stagingFrames - r;
    DWORD first = (take <= canCont) ? take : canCont;
    DWORD second = take - first;

    memcpy(dst, staging + (r * wfx.nBlockAlign), first * wfx.nBlockAlign);
    if (second) {
        memcpy((BYTE*)dst + (first * wfx.nBlockAlign), staging, second * wfx.nBlockAlign);
    }

    stagingReadFrame = (stagingReadFrame + take) % stagingFrames;
    stagingFillFrames -= take;

    if (take < wantFrames) {
        ZeroMemory((BYTE*)dst + (take * wfx.nBlockAlign), (wantFrames - take) * wfx.nBlockAlign);
        return wantFrames;
    }
    return take;
}

DWORD WINAPI WaveOutAudioEngine::FillFromStaticMemory(short* outBuffer, DWORD frames, void* userData) {
    WaveOutAudioEngine* self = (WaveOutAudioEngine*)userData;
    if (!self || !self->staticPcmData) return 0;

    DWORD framesFilled = 0;
    const UINT ch = self->wfx.nChannels;
    while (frames > 0) {
        DWORD remain = 0;
        if (self->staticTotalFrames > self->staticCurrentFrame)
            remain = self->staticTotalFrames - self->staticCurrentFrame;
        if (remain == 0) {
            if (self->loop) { self->staticCurrentFrame = 0; continue; }
            break;
        }
        DWORD take = (frames < remain) ? frames : remain;
        memcpy(outBuffer + (framesFilled * ch), self->staticPcmData + (self->staticCurrentFrame * ch), take * ch * sizeof(short));
        self->staticCurrentFrame += take;
        framesFilled += take;
        frames -= take;
    }
    return framesFilled;
}

void WaveOutAudioEngine::ApplyGainPan(short* samples, DWORD frames) {
    // Effective per-channel gain = master * subVol{L|R}, with mute override
    float gL = muteL ? 0.0f : (volume01 * subVolL);
    float gR = muteR ? 0.0f : (volume01 * subVolR);
    if (gL < 0.0f) gL = 0.0f; if (gL > 1.0f) gL = 1.0f;
    if (gR < 0.0f) gR = 0.0f; if (gR > 1.0f) gR = 1.0f;
    gL = pow(gL, 0.5f); gR = pow(gR, 0.5f);

    if (wfx.nChannels == 1) {
        // Mono: use the louder of L/R
        float g = (gL <= gR) ? gR : gL;
        LONG mul = (LONG)(g * 32768.0f);
        for (DWORD i = 0; i < frames; ++i) {
            LONG s = samples[i];
            s = (s * mul) / 32768;
            samples[i] = (short)ClampS32(s, -32768, 32767);
        }
        return;
    }

    // Stereo or more: apply L to ch0, R to others
    LONG mulL = (LONG)(gL * 32768.0f);
    LONG mulR = (LONG)(gR * 32768.0f);
    const UINT ch = wfx.nChannels;

    for (DWORD f = 0; f < frames; ++f) {
        LONG sL = samples[f * ch + 0];
        sL = (sL * mulL) / 32768;
        samples[f * ch + 0] = (short)ClampS32(sL, -32768, 32767);

        for (UINT c = 1; c < ch; ++c) {
            LONG sR = samples[f * ch + c];
            sR = (sR * mulR) / 32768;
            samples[f * ch + c] = (short)ClampS32(sR, -32768, 32767);
        }
    }
}

BOOL WaveOutAudioEngine::StartThread() {
    if (thread) return TRUE;
    ResetEvent(stopEvent);
    running = 1; paused = 0;
    thread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
    if (!thread) { running = 0; return FALSE; }
    return TRUE;
}

void WaveOutAudioEngine::StopThread() {
    if (!thread) return;
    SetEvent(stopEvent);
    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    thread = NULL;
}

void CALLBACK WaveOutAudioEngine::WaveOutCallback(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    WaveOutAudioEngine* self = (WaveOutAudioEngine*)dwInstance;
    if (!self) return;
    if (uMsg == MM_WOM_DONE) {
        WAVEHDR* hdr = (WAVEHDR*)dwParam1;
        // Mark block free and account position
        for (int i = 0; i < RING_BLOCKS; ++i) {
            if (&self->headers[i] == hdr) {
                self->inUse[i] = 0;
                self->totalBytesPlayed += hdr->dwBufferLength;
                break;
            }
        }
        // Wake the thread to enqueue more
        if (self->notifyEvent) SetEvent(self->notifyEvent);
    }
}

DWORD WINAPI WaveOutAudioEngine::ThreadProc(LPVOID ctx) {
    WaveOutAudioEngine* self = (WaveOutAudioEngine*)ctx;
    self->ThreadLoop();
    return 0;
}

void WaveOutAudioEngine::ThreadLoop() {
    // Pre-fill staging so first writes are ready
    StagingTopUp();

    for (;;) {
        HANDLE waits[2] = { stopEvent, notifyEvent };
        DWORD wr = WaitForMultipleObjects(2, waits, FALSE, 15);
        if (wr == WAIT_OBJECT_0) break; // stop
        if (paused) { Sleep(10); continue; }

        // Top-up staging from callback
        StagingTopUp();

        // Enqueue free headers
        for (int i = 0; i < RING_BLOCKS; ++i) {
            if (inUse[i]) continue;

            // Fill this block from staging
            DWORD frames = blockBytes / wfx.nBlockAlign;
            StagingConsume((short*)blocks[i], frames); // may zero-pad

            // Apply software volume/pan
            ApplyGainPan((short*)blocks[i], frames);

            // Submit
            headers[i].lpData = (LPSTR)blocks[i];
            headers[i].dwBufferLength = blockBytes;
            if (!(headers[i].dwFlags & WHDR_PREPARED)) {
                if (waveOutPrepareHeader(hwo, &headers[i], sizeof(WAVEHDR)) != MMSYSERR_NOERROR) continue;
            }
            if (waveOutWrite(hwo, &headers[i], sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
                inUse[i] = 1;
            }
        }
    }
}

BOOL WaveOutAudioEngine::PlayStream(UINT sampleRate, UINT ch, PcmFillProc fillProc, void* userData, BOOL doLoop) {
    if (!fillProc) return FALSE;
    if (!staticPcmData) { Stop(); } // Clean up existing playback if not static method

    samplerate = sampleRate; channels = ch; loop = doLoop ? TRUE : FALSE;

    ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)ch;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    // Open device in function-callback mode
    if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, (DWORD_PTR)WaveOutCallback, (DWORD_PTR)this, CALLBACK_FUNCTION) != MMSYSERR_NOERROR) {
        hwo = NULL; return FALSE;
    }

    // Set up ring of headers
    const DWORD bytesPerMs = wfx.nAvgBytesPerSec / 1000;
    blockBytes = AlignDownU32(bytesPerMs * WAVEOUT_BLOCK_MS, wfx.nBlockAlign);
    if (blockBytes < wfx.nBlockAlign) blockBytes = wfx.nBlockAlign;

    for (int i = 0; i < RING_BLOCKS; ++i) {
        if (!blocks[i]) blocks[i] = (BYTE*)malloc(blockBytes);
        ZeroMemory(blocks[i], blockBytes);
        ZeroMemory(&headers[i], sizeof(WAVEHDR));
        inUse[i] = 0;
    }

    // Prepare staging
    if (!StagingInit(sampleRate, ch, targetPrebufferSec)) {
        Stop(); return FALSE;
    }

    // Bind callback source
    fill = fillProc;
    user = userData;

    // Kick thread
    if (!StartThread()) { Stop(); return FALSE; }

    return TRUE;
}

BOOL WaveOutAudioEngine::PlayStaticBuffer(UINT sampleRate, UINT ch, short* pcmData, DWORD totalFrames, BOOL doLoop) {
    Stop();
    if (!pcmData || totalFrames == 0) return FALSE;

    staticPcmData = pcmData;
    staticTotalFrames = totalFrames;
    staticCurrentFrame = 0;
    loop = doLoop ? TRUE : FALSE;

    return PlayStream(sampleRate, ch, FillFromStaticMemory, this, loop);
}

void WaveOutAudioEngine::Stop() {
    StopThread();

    if (hwo) {
        waveOutReset(hwo);
        // Unprepare and free blocks
        for (int i = 0; i < RING_BLOCKS; ++i) {
            if (headers[i].dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(hwo, &headers[i], sizeof(WAVEHDR));
            if (blocks[i]) { free(blocks[i]); blocks[i] = NULL; }
            ZeroMemory(&headers[i], sizeof(WAVEHDR));
            inUse[i] = 0;
        }
        waveOutClose(hwo);
        hwo = NULL;
    }

    // Clear stream state
    fill = NULL; user = NULL; loop = FALSE;
    staticPcmData = NULL; staticTotalFrames = 0; staticCurrentFrame = 0;
    totalBytesPlayed = 0;
}

void WaveOutAudioEngine::Pause() {
    if (!hwo || paused) return;
    paused = 1;
    waveOutPause(hwo);
}

void WaveOutAudioEngine::Resume() {
    if (!hwo || !paused) return;
    paused = 0;
    waveOutRestart(hwo);
}

void WaveOutAudioEngine::SetVolume(float masterVol) {
    if (masterVol < 0.0f) masterVol = 0.0f; if (masterVol > 1.0f) masterVol = 1.0f;
    volume01 = masterVol; // software gain applied in ApplyGainPan()
}

void WaveOutAudioEngine::SetSubVolume(float volLeft, float volRight) {
    if (volLeft < 0.0f) volLeft = 0.0f; if (volLeft > 1.0f) volLeft = 1.0f;
    if (volRight < 0.0f) volRight = 0.0f; if (volRight > 1.0f) volRight = 1.0f;
    subVolL = volLeft; subVolR = volRight; // software pan applied in ApplyGainPan()
}

void WaveOutAudioEngine::SetChannelMute(BOOL l, BOOL r) {
    muteL = l; muteR = r; // applied in ApplyGainPan()
}

BOOL WaveOutAudioEngine::IsInitialized() const { return (stopEvent != NULL) && (notifyEvent != NULL); }
BOOL WaveOutAudioEngine::IsPlaying() const { return (hwo != NULL) && (running == 1) && (paused == 0); }
BOOL WaveOutAudioEngine::IsPaused()  const { return (paused == 1); }

DWORD WaveOutAudioEngine::GetPositionMs() const {
    if (wfx.nAvgBytesPerSec == 0) return 0;
    return (DWORD)((totalBytesPlayed * 1000ULL) / (ULONGLONG)wfx.nAvgBytesPerSec);
}

UINT  WaveOutAudioEngine::CurrentSampleRate() const { return samplerate; }
UINT  WaveOutAudioEngine::CurrentChannels()   const { return channels; }
