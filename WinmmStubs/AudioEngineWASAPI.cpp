#include "AudioEngineWASAPI.h"
#include <functiondiscoverykeys_devpkey.h>
#include <initguid.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")

// Undocumented flag for auto-conversion
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
// Custom audio session GUID
DEFINE_GUID(MY_CD_AUDIO_SESSION_GUID, 0xf39b5668, 0x4e6d, 0x4b7e, 0x8a, 0x6e, 0x93, 0x1f, 0x7, 0x58, 0x84, 0x6);

// Convert 100ns to ms
static inline DWORD HnsToMs(REFERENCE_TIME hns) { return (DWORD)(hns / 10000); }

WasapiAudioEngine::WasapiAudioEngine()
    : comInited(FALSE), enumerator(NULL), device(NULL), client(NULL),
    render(NULL), volume(NULL), clock(NULL), channelVolume(NULL),
    thread(NULL), stopEvent(NULL), running(0), paused(0),
    bufferFrameCount(0), fill(NULL), user(NULL),
    volume01(1.0f), subVolL(1.0f), subVolR(1.0f),
    muteL(FALSE), muteR(FALSE)
{
    ZeroMemory(&wfx, sizeof(wfx));
}

WasapiAudioEngine::~WasapiAudioEngine() {
    Shutdown();
}

BOOL WasapiAudioEngine::Initialize(HWND initWindow) {
    if (comInited) return TRUE;

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return FALSE;
    comInited = TRUE;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&enumerator);
    if (FAILED(hr)) { Shutdown(); return FALSE; }

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
    if (FAILED(hr)) { Shutdown(); return FALSE; }

    stopEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!stopEvent) { Shutdown(); return FALSE; }

    return TRUE;
}

void WasapiAudioEngine::Shutdown() {
    Stop();
    if (stopEvent) { CloseHandle(stopEvent); stopEvent = NULL; }
    if (device) { device->Release(); device = NULL; }
    if (enumerator) { enumerator->Release(); enumerator = NULL; }
    if (comInited) { CoUninitialize(); comInited = FALSE; }
    ResetState();
}

void WasapiAudioEngine::ResetState() {
    volume01 = 1.0f;
    ZeroMemory(&wfx, sizeof(wfx));
}

// Stops stream playback and releases related resources (called from Stop())
void WasapiAudioEngine::CleanupStream() {
    if (client) {
        client->Stop();
        client->Release();
        client = NULL;
    }
    if (render) { render->Release(); render = NULL; }
    if (volume) { volume->Release(); volume = NULL; }
    if (clock) { clock->Release(); clock = NULL; }
    if (channelVolume) { channelVolume->Release(); channelVolume = NULL; }

    bufferFrameCount = 0;
    fill = NULL;
    user = NULL;
    ZeroMemory(&wfx, sizeof(wfx));
}

BOOL WasapiAudioEngine::StartThread() {
    if (thread) return TRUE;
    ResetEvent(stopEvent);
    running = 1; paused = 0;
    thread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
    if (!thread) { running = 0; return FALSE; }
    return TRUE;
}

void WasapiAudioEngine::StopThread() {
    if (!thread) return;
    SetEvent(stopEvent);
    WaitForSingleObject(thread, 5000);
    CloseHandle(thread);
    thread = NULL;
    running = 0; paused = 0;
}

DWORD WINAPI WasapiAudioEngine::ThreadProc(LPVOID ctx) {
    WasapiAudioEngine* self = (WasapiAudioEngine*)ctx;
    // WASAPI thread also needs COM initialization
    if (SUCCEEDED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) {
        self->ThreadLoop();
        CoUninitialize();
    }
    return 0;
}

void WasapiAudioEngine::ThreadLoop() {
    // Wait time corresponding to half the buffer (ms)
    REFERENCE_TIME hnsBufferDuration = 0;
    client->GetStreamLatency(&hnsBufferDuration); // Approximate buffer latency
    DWORD sleepMs = HnsToMs(hnsBufferDuration) / 2;

    if (sleepMs < 10) sleepMs = 10; // Min 10ms
    if (sleepMs > 100) sleepMs = 100; // Max 100ms

    // Start playback
    if (FAILED(client->Start())) {
        running = 0;
        return;
    }

    for (;;) {
        DWORD wr = WaitForSingleObject(stopEvent, sleepMs);
        if (wr == WAIT_OBJECT_0) break; // Stop signal

        if (paused) {
            Sleep(10); // Short wait while paused
            continue;
        }

        UINT32 padding = 0;
        if (FAILED(client->GetCurrentPadding(&padding))) continue;

        UINT32 framesAvailable = bufferFrameCount - padding;
        if (framesAvailable == 0) continue; // No space to fill

        BYTE* pData = NULL;
        if (FAILED(render->GetBuffer(framesAvailable, &pData))) continue;

        // Fill data with callback
        DWORD framesFilled = 0;
        if (fill) {
            framesFilled = fill((short*)pData, framesAvailable, user);
        }

        // If the callback filled less than requested (end of file, not looping),
        // fill the rest with silence.
        if (framesFilled < framesAvailable) {
            DWORD silentFrames = framesAvailable - framesFilled;
            DWORD silentBytes = silentFrames * wfx.nBlockAlign;
            BYTE* pSilentStart = pData + (framesFilled * wfx.nBlockAlign);
            ZeroMemory(pSilentStart, silentBytes);
        }

        render->ReleaseBuffer(framesAvailable, 0);
    }
}


BOOL WasapiAudioEngine::PlayStream(UINT sampleRate, UINT channels, PcmFillProc fillProc, void* userData, BOOL loop) {
    if (!device || !fillProc) return FALSE;

    Stop(); // Clean up existing playback

    fill = fillProc;
    user = userData;
    // Looping is handled by the adapter/callback, so the engine doesn't store it

    HRESULT hr;
    // Activate IAudioClient
    hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&client);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    // Set format
    ZeroMemory(&wfx, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = (WORD)channels;
    wfx.nSamplesPerSec = sampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    // Check if the mixer supports this format (resampling needed if not)
    // For simple implementation, fail if not supported.
    WAVEFORMATEX* pClosestMatch = NULL;
    hr = client->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &wfx, &pClosestMatch);

    // pClosestMatch can be allocated when S_FALSE, free it regardless of HRESULT
    if (pClosestMatch) {
        CoTaskMemFree(pClosestMatch);
        pClosestMatch = NULL;
    }

    // Also consider S_FALSE (auto-conversion needed) as success and proceed
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    // Initialize audio client (use default buffer time)
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM, // Use auto-converter
        1000 * 10000, // Buffer duration decided by device (1sec)
        0,
        &wfx,
        &MY_CD_AUDIO_SESSION_GUID); // Pass custom session GUID
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    // Get buffer size
    hr = client->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    // Get render/volume/clock services
    hr = client->GetService(__uuidof(IAudioRenderClient), (void**)&render);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    hr = client->GetService(__uuidof(ISimpleAudioVolume), (void**)&volume);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    hr = client->GetService(__uuidof(IChannelAudioVolume), (void**)&channelVolume);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    hr = client->GetService(__uuidof(IAudioClock), (void**)&clock);
    if (FAILED(hr)) { CleanupStream(); return FALSE; }

    // Apply current volume/mute state
    ApplyVolumeSettings();

    // Start streaming thread
    if (!StartThread()) {
        CleanupStream();
        return FALSE;
    }

    return TRUE;
}

void WasapiAudioEngine::Stop() {
    StopThread(); // Stop thread first
    CleanupStream(); // Cleanup stream resources
}

void WasapiAudioEngine::Pause() {
    if (!client) return;
    if (paused) return;
    paused = 1;
    client->Stop(); // Stop WASAPI stream (preserves playback position)
}

void WasapiAudioEngine::Resume() {
    if (!client) return;
    if (!paused) return;
    paused = 0;
    client->Start(); // Restart WASAPI stream
}

void WasapiAudioEngine::ApplyVolumeSettings() {
    // Apply Master Volume
    if (volume) {
        volume->SetMasterVolume(volume01, NULL);
        // Master Mute if both channels are muted
        volume->SetMute((muteL && muteR), NULL);
    }

    // Apply Per-Channel Volume (SubVolume * MuteState)
    if (channelVolume) {
        UINT count = 0;
        channelVolume->GetChannelCount(&count);
        if (count == 0) return;

        // Calculate effective channel levels (relative to master)
        float levelL = subVolL;
        float levelR = subVolR;
        if (muteL) levelL = 0.0f;
        if (muteR) levelR = 0.0f;

        // Clamp just in case
        if (levelL < 0.0f) levelL = 0.0f; if (levelL > 1.0f) levelL = 1.0f;
        if (levelR < 0.0f) levelR = 0.0f; if (levelR > 1.0f) levelR = 1.0f;

        if (count == 1) { // Mono
            channelVolume->SetChannelVolume(0, (levelL <= levelR) ? levelR : levelL, NULL); // Use the louder sub-vol for mono
        }
        else { // Stereo or more
            channelVolume->SetChannelVolume(0, levelL, NULL); // Left
            // Apply Right level to all other channels (1, 2, ...)
            for (UINT i = 1; i < count; ++i) {
                channelVolume->SetChannelVolume(i, levelR, NULL);
            }
        }
    }
}

void WasapiAudioEngine::SetVolume(float vol) {
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;
    volume01 = vol; // Store the new volume

    ApplyVolumeSettings(); // Apply combined settings
}

void WasapiAudioEngine::SetSubVolume(float volLeft, float volRight) {
    // Clamp and store the sub-volume values internally
    if (volLeft < 0.0f) volLeft = 0.0f; if (volLeft > 1.0f) volLeft = 1.0f;
    if (volRight < 0.0f) volRight = 0.0f; if (volRight > 1.0f) volRight = 1.0f;
    subVolL = volLeft;
    subVolR = volRight;

    ApplyVolumeSettings(); // Apply combined settings
}

void WasapiAudioEngine::SetChannelMute(BOOL muteLeft, BOOL muteRight) {
    muteL = muteLeft;
    muteR = muteRight;

    ApplyVolumeSettings(); // Apply combined settings
}

BOOL WasapiAudioEngine::IsInitialized() const { return (device != NULL); }
BOOL WasapiAudioEngine::IsPlaying() const { return (client != NULL) && (running == 1) && (paused == 0); }
BOOL WasapiAudioEngine::IsPaused() const { return (paused == 1); }

DWORD WasapiAudioEngine::GetPositionMs() const {
    if (!clock || wfx.nSamplesPerSec == 0 || bufferFrameCount == 0) return 0;

    UINT64 posFrames = 0, freq = 0;
    // GetPosition returns *total frames played* since stream start
    if ((FAILED(clock->GetPosition(&posFrames, NULL))) || (FAILED(clock->GetFrequency(&freq)))) return 0;

    // Calculate position total time played
    DWORD ms = (DWORD)((posFrames * 1000ULL) / freq);
    //dprintf("total time=%d", ms);
    return ms;
}

UINT  WasapiAudioEngine::CurrentSampleRate() const { return wfx.nSamplesPerSec; }
UINT  WasapiAudioEngine::CurrentChannels()   const { return wfx.nChannels; }
