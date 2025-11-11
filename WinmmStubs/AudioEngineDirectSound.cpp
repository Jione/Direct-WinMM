#include "AudioEngineDirectSound.h"
#include <mmreg.h>
#include <math.h>

#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")

// External dependency: Force WaveOut Audio Engine
namespace PreferenceLoader {
    BOOL ForceWaveOutEngineForApp();
}

static inline LONG ClampLONG(LONG v, LONG lo, LONG hi) {
    if (v < lo) return lo; if (v > hi) return hi; return v;
}
static inline DWORD AlignDown(DWORD v, DWORD align) {
    return (align == 0) ? v : (v / align) * align;
}
static inline LONG VolumeFloatToDsDb(float vol01) {
    if (vol01 <= 0.00001f) return DSBVOLUME_MIN; // -10000 (prevents log10(0))
    if (vol01 >= 0.99999f) return 0;

    LONG dsdb = (LONG)(1000.0f * log10f(vol01 * vol01));

    return ClampLONG(dsdb, DSBVOLUME_MIN, 0);
}
static inline LONG CalculatePan(float left, float right) {
    if (left < 0.00001f && right < 0.00001f) return DSBPAN_CENTER; // Avoid division by zero
    if (left > 0.99999f && right > 0.99999f) return DSBPAN_CENTER; // Full volume both sides

    // Normalize max to 1.0
    float maxVol = (left <= right) ? right : left;
    float normL = (maxVol > 0.00001f) ? left / maxVol : 0.0f;
    float normR = (maxVol > 0.00001f) ? right / maxVol : 0.0f;

    // Simple ratio-based pan
    // If L=1, R=0 -> Pan = -10000
    // If L=0, R=1 -> Pan = 10000
    // If L=0.5, R=0.5 -> Pan = 0
    // If L=1, R=0.5 -> Pan = -3333 approx (more left)
    // If L=0.5, R=1 -> Pan = +3333 approx (more right)
    float totalNorm = normL + normR;
    if (totalNorm < 0.00001f) return DSBPAN_CENTER;

    float balance = (normR - normL) / totalNorm; // Range -1.0 (full left) to +1.0 (full right)

    LONG pan = (LONG)(balance * (float)DSBPAN_RIGHT); // DSBPAN_RIGHT is 10000

    return ClampLONG(pan, DSBPAN_LEFT, DSBPAN_RIGHT);
}

static inline void ForceWaveOutOnDirectSoundError(HRESULT hr) {
    // DSERR_PRIOLEVELNEEDED: need DSSCL_PRIORITY to set primary format
    // DSERR_ALLOCATED: often indicates exclusive-like conflicts
    if (hr == DSERR_PRIOLEVELNEEDED || hr == DSERR_ALLOCATED) {
        // write Engine=WaveOut to app override key
        PreferenceLoader::ForceWaveOutEngineForApp();
    }
}

DSoundAudioEngine::DSoundAudioEngine()
    : ds(NULL), primary(NULL), secondary(NULL),
    thread(NULL), stopEvent(NULL), running(0), paused(0),
    bufferBytes(0), halfBytes(0), blockBytes(0), writeCursor(0),
    fill(NULL), user(NULL), loop(FALSE),
    volume01(1.0f), subVolL(1.0f), subVolR(1.0f),
    muteL(FALSE), muteR(FALSE),
    approxMs(0), samplerate(0), channels(0), hwnd(NULL),
    lastPlayCursor(0), totalBytesPlayed(0),
    staging(NULL), stagingBytes(0), stagingFrames(0),
    stagingWriteFrame(0), stagingReadFrame(0), stagingFillFrames(0),
    targetPrebufferSec(16) // default 16s
{
    ZeroMemory(&wfx, sizeof(wfx));
}
DSoundAudioEngine::~DSoundAudioEngine() {
    Shutdown();
}

BOOL DSoundAudioEngine::Initialize(HWND initWindow) {
    if (ds) return TRUE;
    if (FAILED(DirectSoundCreate8(NULL, &ds, NULL))) return FALSE;

    hwnd = initWindow ? initWindow : GetDesktopWindow();
    HRESULT hr = ds->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
    if (FAILED(hr)) {
        ForceWaveOutOnDirectSoundError(hr); // trigger WaveOut override on DS conflict
        ds->Release(); ds = NULL; return FALSE;
    }
    if (!CreatePrimary()) {
        ds->Release(); ds = NULL; return FALSE;
    }
    stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!stopEvent) {
        primary->Release(); primary = NULL; ds->Release(); ds = NULL; return FALSE;
    }
    return TRUE;
}

void DSoundAudioEngine::Shutdown() {
    Stop();
    if (stopEvent) { CloseHandle(stopEvent); stopEvent = NULL; }
    if (ds && hwnd && IsWindow(hwnd)) {
        ds->SetCooperativeLevel(hwnd, DSSCL_NORMAL);
        if (primary) { primary->Stop(); primary->Release(); primary = NULL; }
        ds->Release();
        ds = NULL;
    }
    else {
        primary = NULL;
        ds = NULL;
    }
    ResetState();
}

void DSoundAudioEngine::ResetState() {
    volume01 = 1.0f; samplerate = 0; channels = 0;
    lastPlayCursor = 0; totalBytesPlayed = 0;
    StagingFree();
    ZeroMemory(&wfx, sizeof(wfx));
}

void DSoundAudioEngine::StagingFree() {
    if (staging) { free(staging); staging = NULL; }
    stagingBytes = 0; stagingFrames = 0;
    stagingWriteFrame = stagingReadFrame = stagingFillFrames = 0;
}

BOOL DSoundAudioEngine::StagingInit(UINT sampleRate, UINT channels, UINT seconds) {
    StagingFree();
    if (seconds < 3) seconds = 3;
    if (seconds > 30) seconds = 30;
    DWORD frames = sampleRate * seconds;
    DWORD bytes = frames * (channels * sizeof(short));
    bytes = AlignDown(bytes, wfx.nBlockAlign);
    if (bytes == 0) return FALSE;
    staging = (BYTE*)malloc(bytes);
    if (!staging) return FALSE;
    stagingBytes = bytes;
    stagingFrames = bytes / wfx.nBlockAlign;
    stagingWriteFrame = stagingReadFrame = stagingFillFrames = 0;
    return TRUE;
}

BOOL DSoundAudioEngine::CreatePrimary() {
    DSBUFFERDESC desc; ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
    HRESULT hr = ds->CreateSoundBuffer(&desc, &primary, NULL);
    if (FAILED(hr)) {
        ForceWaveOutOnDirectSoundError(hr);
        return FALSE;
    }

    // Default format: 48kHz, S16, Stereo
    WAVEFORMATEX wfx; ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 48000;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    hr = primary->SetFormat(&wfx);
    if (FAILED(hr)) {
        ForceWaveOutOnDirectSoundError(hr); // DSERR_PRIOLEVELNEEDED likely here
        return FALSE;
    }
    return TRUE;
}

BOOL DSoundAudioEngine::CreateSecondary(UINT sampleRate, UINT channels) {
    if (!ds) return FALSE;

    samplerate = sampleRate; this->channels = channels;
    ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    // Buffer size: 4 seconds worth (double buffer)
    DWORD bytes4sec = wfx.nAvgBytesPerSec * 4;
    // Safe block: 1 seconds aligned
    DWORD bytes1sec = wfx.nAvgBytesPerSec;
    bytes1sec = AlignDown(bytes1sec, wfx.nBlockAlign);

    bufferBytes = AlignDown(bytes4sec, wfx.nBlockAlign);
    halfBytes = bufferBytes / 2;
    blockBytes = bytes1sec ? bytes1sec : wfx.nBlockAlign;

    if (!StagingInit(sampleRate, channels, targetPrebufferSec)) return FALSE;

    DSBUFFERDESC desc; ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
    desc.dwBufferBytes = bufferBytes;
    desc.lpwfxFormat = &wfx;

    HRESULT hr = ds->CreateSoundBuffer(&desc, &secondary, NULL);
    if (FAILED(hr)) {
        ForceWaveOutOnDirectSoundError(hr); // treat exclusive-like conflicts
        return FALSE;
    }

    writeCursor = 0;

    // Apply initial volume/pan/mute state
    SetVolume(volume01);
    SetSubVolume(subVolL, subVolR); // This calculates initial Pan
    SetChannelMute(muteL, muteR);   // This applies final override

    return TRUE;
}

void DSoundAudioEngine::DestroySecondary() {
    if (secondary) { secondary->Release(); secondary = NULL; }
    bufferBytes = halfBytes = blockBytes = 0;
    writeCursor = 0;
    fill = NULL; user = NULL; loop = FALSE;
}

BOOL DSoundAudioEngine::StartThread() {
    if (thread) return TRUE;
    ResetEvent(stopEvent);
    running = 1; paused = 0;
    thread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
    if (!thread) { running = 0; return FALSE; }
    return TRUE;
}
void DSoundAudioEngine::StopThread() {
    if (!thread) return;
    SetEvent(stopEvent);
    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    thread = NULL;
    running = 0; paused = 0;
}

DWORD WINAPI DSoundAudioEngine::ThreadProc(LPVOID ctx) {
    DSoundAudioEngine* self = (DSoundAudioEngine*)ctx;
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    self->ThreadLoop();
    return 0;
}

void DSoundAudioEngine::ThreadLoop() {
    // Fill initial data and start loop
    FillInitial();
    if (secondary) secondary->Play(0, 0, DSBPLAY_LOOPING);
    int tryCount = 16;
    BOOL lastLost = FALSE;

    for (;;) {
        DWORD wr = WaitForSingleObject(stopEvent, 15);
        if (wr == WAIT_OBJECT_0) break;

        if (paused) {
            if (secondary) {
                secondary->GetCurrentPosition(&lastPlayCursor, NULL);
            }
            Sleep(10);
            continue;
        }

        RefillIfNeeded();

        // Accumulate approximate ms (fallback)
        if (secondary && wfx.nAvgBytesPerSec > 0) {
            DWORD playCursor = 0;
            HRESULT hr = secondary->GetCurrentPosition(&playCursor, NULL);
            if (SUCCEEDED(hr)) {
                if (lastLost) {
                    lastLost = FALSE;
                    tryCount = 16;
                }
                DWORD bytesPlayedThisTick = 0;
                if (playCursor < lastPlayCursor) {
                    bytesPlayedThisTick = (bufferBytes - lastPlayCursor) + playCursor;
                }
                else {
                    bytesPlayedThisTick = playCursor - lastPlayCursor;
                }

                totalBytesPlayed += (ULONGLONG)bytesPlayedThisTick;
                lastPlayCursor = playCursor;
            }
            else if (hr == DSERR_BUFFERLOST) {
                dprintf("Detect Buffer Lost, Try count=%d", tryCount);
                lastLost = TRUE;
                if (--tryCount < 0) {
                    PreferenceLoader::ForceWaveOutEngineForApp();
                }
            }
        }
    }

    if (secondary) secondary->Stop();
}

DWORD DSoundAudioEngine::StagingTopUp() {
    if (!fill || !staging || stagingFrames == 0) return 0;
    DWORD produced = 0;

    // try to fill up to ~90% of capacity to reduce callback overhead
    DWORD targetFill = (stagingFrames * 9) / 10;
    while (stagingFillFrames < targetFill) {
        // produce in chunks (>= 1/2 sec)
        DWORD chunk = (samplerate / 2);
        if (chunk < 256) chunk = 256; // guard
        // clamp to free space
        DWORD freeFrames = stagingFrames - stagingFillFrames;
        if (freeFrames == 0) break;
        if (chunk > freeFrames) chunk = freeFrames;

        // write pointer
        DWORD w = stagingWriteFrame;
        DWORD canCont = stagingFrames - w;
        DWORD first = (chunk <= canCont) ? chunk : canCont;
        DWORD second = chunk - first;

        short* dst1 = (short*)(staging + (w * wfx.nBlockAlign));
        DWORD got1 = 0;
        if (first) {
            got1 = fill(dst1, first, user);
            if (got1 < first) {
                // zero-fill the rest to avoid ring-repeat glitch
                ZeroMemory(dst1 + (got1 * wfx.nChannels),
                    (first - got1) * wfx.nBlockAlign);
            }
        }

        if (second) {
            short* dst2 = (short*)staging;
            DWORD got2 = fill(dst2, second, user);
            if (got2 < second) {
                ZeroMemory(dst2 + (got2 * wfx.nChannels),
                    (second - got2) * wfx.nBlockAlign);
            }
        }

        // Always advance by 'chunk' to keep timeline monotonic
        stagingWriteFrame = (stagingWriteFrame + chunk) % stagingFrames;
        stagingFillFrames += chunk;
        produced += chunk;

        // if callback is truly out of data (returns 0 twice), we still
        // advanced with zeros; future consume will play silence, not old data
        if (chunk < (samplerate / 8)) break; // tiny top-up avoid busy loop
    }
    return produced;
}

DWORD DSoundAudioEngine::StagingConsume(short* dst, DWORD wantFrames) {
    if (!staging || stagingFillFrames == 0) {
        // no data -> zero
        ZeroMemory(dst, wantFrames * wfx.nBlockAlign);
        return wantFrames;
    }
    DWORD take = (wantFrames <= stagingFillFrames) ? wantFrames : stagingFillFrames;

    DWORD r = stagingReadFrame;
    DWORD canCont = stagingFrames - r;
    DWORD first = (take <= canCont) ? take : canCont;
    DWORD second = take - first;

    memcpy(dst,
        staging + (r * wfx.nBlockAlign),
        first * wfx.nBlockAlign);

    if (second) {
        memcpy((BYTE*)dst + (first * wfx.nBlockAlign),
            staging,
            second * wfx.nBlockAlign);
    }

    stagingReadFrame = (stagingReadFrame + take) % stagingFrames;
    stagingFillFrames -= take;

    if (take < wantFrames) {
        ZeroMemory((BYTE*)dst + (take * wfx.nBlockAlign),
            (wantFrames - take) * wfx.nBlockAlign);
        return wantFrames;
    }
    return take;
}

void DSoundAudioEngine::FillInitial() {
    if (!secondary) return;

    // prefill staging first
    StagingTopUp();

    VOID* p1 = NULL; DWORD b1 = 0;
    if (FAILED(secondary->Lock(0, halfBytes, &p1, &b1, NULL, NULL, 0))) return;

    if (p1 && b1 > 0) {
        DWORD framesWant = b1 / wfx.nBlockAlign;
        // consume from staging (will zero-fill if shortage)
        StagingConsume((short*)p1, framesWant);
    }
    secondary->Unlock(p1, b1, NULL, NULL);
    writeCursor = b1;
}

void DSoundAudioEngine::RefillIfNeeded() {
    if (!secondary || !fill) return;

    DWORD play = 0, write = 0;
    if (FAILED(secondary->GetCurrentPosition(&play, &write))) return;

    // Our write cursor is writeCursor. Secure a safe distance from the play cursor (halfBytes)
    DWORD safeAhead = (play + halfBytes) % bufferBytes;

    // Fill up to 'safeAhead' so that writeCursor doesn't catch up to it
    DWORD toFill = 0;
    if (writeCursor == safeAhead) {
        // Sufficiently filled
        return;
    }
    else if (writeCursor < safeAhead) {
        toFill = safeAhead - writeCursor;
    }
    else {
        // Ring wrap-around
        toFill = (bufferBytes - writeCursor) + safeAhead;
    }
    // Prevent frequent large locks: fill in smaller 'blockBytes' units
    if (toFill > blockBytes) toFill = blockBytes;
    toFill = AlignDown(toFill, wfx.nBlockAlign);
    if (toFill == 0) return;

    StagingTopUp();

    VOID* p1 = NULL; DWORD b1 = 0; VOID* p2 = NULL; DWORD b2 = 0;
    if (FAILED(secondary->Lock(writeCursor, toFill, &p1, &b1, &p2, &b2, 0))) return;

    // p1
    if (b1) {
        DWORD frames = b1 / wfx.nBlockAlign;
        StagingConsume((short*)p1, frames); // always fills fully (zero-pads)
        writeCursor = (writeCursor + b1) % bufferBytes;
    }
    // p2
    if (p2 && b2) {
        DWORD frames = b2 / wfx.nBlockAlign;
        StagingConsume((short*)p2, frames);
        writeCursor = (writeCursor + b2) % bufferBytes;
    }
    secondary->Unlock(p1, b1, p2, b2);
}

BOOL DSoundAudioEngine::PlayStream(UINT sampleRate, UINT channels, PcmFillProc fillProc, void* userData, BOOL loop) {
    if (!ds) return FALSE;

    Stop(); // Clean up existing playback
    fill = fillProc;
    user = userData;
    this->loop = loop ? TRUE : FALSE; // (FIX) Store loop state

    if (!CreateSecondary(sampleRate, channels)) {
        fill = NULL; user = NULL; return FALSE;
    }
    
    totalBytesPlayed = 0;
    lastPlayCursor = 0;

    if (!StartThread()) {
        DestroySecondary();
        return FALSE;
    }
    return TRUE;
}

BOOL DSoundAudioEngine::PlayStaticBuffer(UINT sampleRate, UINT channels, short* pcmData, DWORD totalFrames, BOOL loop)
{
    if (!ds || !pcmData || totalFrames == 0) return FALSE;

    Stop(); // Clean up existing playback (stops thread if running)

    fill = NULL; // Mark as *non-streaming* (static)
    user = NULL;
    this->loop = loop ? TRUE : FALSE;

    // --- Create a static buffer exactly matching the data size ---
    samplerate = sampleRate; this->channels = channels;
    ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    bufferBytes = totalFrames * wfx.nBlockAlign;
    if (bufferBytes == 0) return FALSE;

    DSBUFFERDESC desc; ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc);
    // Flags: Use default flags, NOT DSBCAPS_STATIC which is obsolete/problematic
    desc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
    desc.dwBufferBytes = bufferBytes;
    desc.lpwfxFormat = &wfx;

    HRESULT hr = ds->CreateSoundBuffer(&desc, &secondary, NULL);
    if (FAILED(hr)) {
        DestroySecondary();
        return FALSE;
    }

    // --- Lock, Copy data, Unlock ---
    VOID* p1 = NULL; DWORD b1 = 0;
    VOID* p2 = NULL; DWORD b2 = 0;

    hr = secondary->Lock(0, bufferBytes, &p1, &b1, &p2, &b2, 0);
    if (SUCCEEDED(hr)) {
        if (p1 && b1 > 0) {
            memcpy(p1, pcmData, b1);
        }
        if (p2 && b2 > 0) {
            memcpy(p2, (BYTE*)pcmData + b1, b2);
        }

        secondary->Unlock(p1, b1, p2, b2);
    }
    else {
        DestroySecondary();
        return FALSE;
    }

    // Apply initial volume/pan/mute state
    SetVolume(volume01);
    SetSubVolume(subVolL, subVolR);
    SetChannelMute(muteL, muteR);

    // --- Play ---
    totalBytesPlayed = 0;
    lastPlayCursor = 0;
    running = 1;
    paused = 0;

    secondary->SetCurrentPosition(0);
    secondary->Play(0, 0, loop ? DSBPLAY_LOOPING : 0);

    return TRUE;
}

void DSoundAudioEngine::Stop() {
    if (fill != NULL) {
        StopThread(); // Only stop thread if we were streaming
    }

    if (secondary) {
        secondary->Stop(); // Always stop the buffer
    }

    DestroySecondary(); // This clears secondary, fill, user, etc.

    // Ensure thread-related flags are clear
    running = 0;
    paused = 0;
}

void DSoundAudioEngine::Pause() {
    if (!secondary) return;
    if (paused) return;
    paused = 1;
    secondary->Stop(); // Stop the cursor
}
void DSoundAudioEngine::Resume() {
    if (!secondary) return;
    if (!paused) return;
    paused = 0;
    secondary->Play(0, 0, DSBPLAY_LOOPING);
}

void DSoundAudioEngine::SetVolume(float masterVol) {
    if (masterVol < 0.0f) masterVol = 0.0f;
    if (masterVol > 1.0f) masterVol = 1.0f;
    volume01 = masterVol; // Store the new *master* volume

    // Calculate the effective volume considering sub-volume attenuation
    // Use the *maximum* of sub-volumes as the attenuation factor
    float subAttenuation = (subVolL <= subVolR) ? subVolR : subVolL;
    float effectiveVol = volume01 * subAttenuation;

    if (ds) {
        LONG dsvol = VolumeFloatToDsDb(effectiveVol);

        // Mute override
        if (muteL && muteR) {
            dsvol = DSBVOLUME_MIN;
        }

        if (secondary) secondary->SetVolume(dsvol);
    }
}

void DSoundAudioEngine::SetSubVolume(float volLeft, float volRight) {
    if (volLeft < 0.0f) volLeft = 0.0f; if (volLeft > 1.0f) volLeft = 1.0f;
    if (volRight < 0.0f) volRight = 0.0f; if (volRight > 1.0f) volRight = 1.0f;
    subVolL = volLeft;
    subVolR = volRight;

    // Calculate and apply Pan based on sub volumes
    if (secondary) {
        // Only apply sub-volume pan if not explicitly muted
        if (!muteL || !muteR) {
            LONG pan = CalculatePan(subVolL, subVolR);
            secondary->SetPan(pan);
        }
        // else: Mute logic in SetChannelMute will handle pan override
    }

    // Re-apply master volume which now considers sub-volume attenuation
    SetVolume(volume01);
}

void DSoundAudioEngine::SetChannelMute(BOOL muteLeft, BOOL muteRight) {
    muteL = muteLeft;
    muteR = muteRight;

    // Determine Pan override based on Mute
    if (secondary) {
        LONG pan = CalculatePan(subVolL, subVolR); // Start with sub-volume pan

        // Mute overrides pan
        if (muteL && !muteR) {
            pan = DSBPAN_RIGHT; // Mute Left -> Pan Right hard
        }
        else if (!muteL && muteR) {
            pan = DSBPAN_LEFT;  // Mute Right -> Pan Left hard
        }
        else if (muteL && muteR) {
            pan = DSBPAN_CENTER; // Both muted, pan doesn't matter but center is neutral
        }
        // else: neither muted, keep sub-volume pan calculated above

        secondary->SetPan(pan);
    }

    // Re-apply Volume (SetVolume checks muteL && muteR)
    SetVolume(volume01);
}

BOOL DSoundAudioEngine::IsInitialized() const { return ds != NULL; }
BOOL DSoundAudioEngine::IsPlaying() const {
    // If there's no secondary buffer or it's paused, it's definitely not playing.
    if (!secondary || paused == 1) return FALSE;

    // Mode 1: Streaming Mode (callback 'fill' exists)
    // In this mode, the 'running' flag managed by the background thread is reliable.
    if (fill != NULL) {
        return (running == 1);
    }

    // Mode 2: Static Buffer Mode ('fill' is NULL)
    // In this mode, the 'running' flag is unreliable as there's no thread
    // to reset it when playback finishes.
    DWORD dwStatus = 0;
    if (FAILED(secondary->GetStatus(&dwStatus))) {
        return FALSE; // On error, assume not playing
    }

    // Return TRUE only if the hardware buffer is actively playing.
    // (dwStatus & DSBSTATUS_PLAYING) will be 0 when a non-looping static buffer finishes.
    return (dwStatus & DSBSTATUS_PLAYING) != 0;
}
BOOL DSoundAudioEngine::IsPaused() const { return (paused == 1); }

DWORD DSoundAudioEngine::GetPositionMs() const {
    if (!secondary || wfx.nAvgBytesPerSec == 0) return 0;

    // Streaming mode: Use the thread-accumulated value
    if (fill != NULL) {
        //dprintf("total time=%d", (DWORD)((totalBytesPlayed * 1000ULL) / (ULONGLONG)wfx.nAvgBytesPerSec));
        return (DWORD)((totalBytesPlayed * 1000ULL) / (ULONGLONG)wfx.nAvgBytesPerSec);
    }
    // Static buffer mode: Get hardware cursor directly
    else {
        DWORD playCursor = 0;
        if (FAILED(secondary->GetCurrentPosition(&playCursor, NULL))) return 0;
        //dprintf("total time=%d", (DWORD)(((ULONGLONG)playCursor * 1000ULL) / (ULONGLONG)wfx.nAvgBytesPerSec));
        return (DWORD)(((ULONGLONG)playCursor * 1000ULL) / (ULONGLONG)wfx.nAvgBytesPerSec);
    }
}
UINT  DSoundAudioEngine::CurrentSampleRate() const { return samplerate; }
UINT  DSoundAudioEngine::CurrentChannels()   const { return channels; }
