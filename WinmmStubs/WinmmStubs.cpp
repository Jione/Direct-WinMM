#include "GlobalDefinitions.h"
#include "PreferenceLoader.h"

// Environmental Initialization
BOOL WINAPI NsGetAddress(HMODULE hLibModule);
static HMODULE hLibModule = NULL;

// System DLL module load function
BOOL WINAPI LoadSystemLibraryW(WCHAR * wLibName) {
    WCHAR tzPath[MAX_PATH]{ 0, };
    HANDLE hFile;
    UINT szText;

    szText = GetSystemDirectoryW(tzPath, MAX_PATH);
    tzPath[szText++] = L'\\';
    tzPath[szText] = 0;
    lstrcatW(tzPath, wLibName);
    lstrcatW(tzPath, L".dll");
    hFile = CreateFileW(tzPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        hLibModule = LoadLibraryW(tzPath);
    }
    else {
        szText = GetCurrentDirectoryW(MAX_PATH, tzPath);
        tzPath[szText++] = L'\\';
        tzPath[szText] = 0;
        lstrcatW(tzPath, wLibName);
#ifndef _AMD64_
        lstrcatW(tzPath, L".x86.dll");
#else
        lstrcatW(tzPath, L".x64.dll");
#endif
        hFile = CreateFileW(tzPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);
            hLibModule = LoadLibraryW(tzPath);
        }
    }
    if (hLibModule) {
        NsGetAddress(hLibModule);
        dprintf(L"System DLL Path: %s\nLoaded %s.dll File (Address:0x%08X)\n", tzPath, wLibName, hLibModule);
        return true;
    }
    return false;
}

// Entry point at DLL Load & Release
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        if (!LoadSystemLibraryW(L"winmm"))
            MessageBoxA(NULL, "Failed to load system DLL file.", "Alert", MB_OK);
#ifdef _DEBUG
        else
            MessageBoxA(NULL, "Winmm.dll Loaded.", "Alert", MB_OK);
#endif
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        if (hLibModule) FreeLibrary(hLibModule);
        PreferenceLoader::Shutdown();
#ifdef _DEBUG
        MessageBoxA(NULL, "Winmm.dll Unloaded.", "Alert", MB_OK);
#endif
    }
    return TRUE;
}

// Get system library address
#pragma pack(push, 1)
extern "C" {
// PlaySound Functions
PVOID _imp_NT4PlaySound;
PVOID _imp_PlaySound;
PVOID _imp_sndPlaySoundA;
PVOID _imp_sndPlaySoundW;
PVOID _imp_PlaySoundA;
PVOID _imp_PlaySoundW;

// Driver and Drv help Functions
PVOID _imp_CloseDriver;
PVOID _imp_DefDriverProc;
PVOID _imp_DriverCallback;
PVOID _imp_DrvClose;
PVOID _imp_DrvGetModuleHandle;
PVOID _imp_DrvOpen;
PVOID _imp_DrvSendMessage;
PVOID _imp_GetDriverModuleHandle;
PVOID _imp_MigrateSoundEvents;
PVOID _imp_MigrateAllDrivers;
PVOID _imp_mmsystemGetVersion;
PVOID _imp_NotifyCallbackData;
PVOID _imp_OpenDriver;
PVOID _imp_SendDriverMessage;
PVOID _imp_WOW32DriverCallback;
PVOID _imp_WOW32ResolveMultiMediaHandle;
PVOID _imp_WOWAppExit;
PVOID _imp_winmmDbgOut;
PVOID _imp_WinmmLogon;
PVOID _imp_WinmmLogoff;
PVOID _imp_winmmSetDebugLevel;

// Audio GFX support Functions
PVOID _imp_gfxCreateZoneFactoriesList;
PVOID _imp_gfxCreateGfxFactoriesList;
PVOID _imp_gfxDestroyDeviceInterfaceList;
PVOID _imp_gfxEnumerateGfxs;
PVOID _imp_gfxRemoveGfx;
PVOID _imp_gfxAddGfx;
PVOID _imp_gfxModifyGfx;
PVOID _imp_gfxOpenGfx;
PVOID _imp_gfxBatchChange;

// AUX Functions
PVOID _imp_auxGetDevCapsA;
PVOID _imp_auxGetDevCapsW;
PVOID _imp_auxGetNumDevs;
PVOID _imp_auxGetVolume;
PVOID _imp_auxOutMessage;
PVOID _imp_auxSetVolume;

// Joystick Functions
PVOID _imp_joyConfigChanged;
PVOID _imp_joySetCalibration;
PVOID _imp_joyGetDevCapsA;
PVOID _imp_joyGetDevCapsW;
PVOID _imp_joyGetNumDevs;
PVOID _imp_joyGetPos;
PVOID _imp_joyGetPosEx;
PVOID _imp_joyGetThreshold;
PVOID _imp_joyReleaseCapture;
PVOID _imp_joySetCapture;
PVOID _imp_joySetThreshold;

// MCI Functions
PVOID _imp_mciExecute;
PVOID _imp_mciDriverNotify;
PVOID _imp_mciDriverYield;
PVOID _imp_mciFreeCommandResource;
PVOID _imp_mciGetCreatorTask;
PVOID _imp_mciGetDeviceIDA;
PVOID _imp_mciGetDeviceIDFromElementIDA;
PVOID _imp_mciGetDeviceIDFromElementIDW;
PVOID _imp_mciGetDeviceIDW;
PVOID _imp_mciGetDriverData;
PVOID _imp_mciGetErrorStringA;
PVOID _imp_mciGetErrorStringW;
PVOID _imp_mciGetYieldProc;
PVOID _imp_mciLoadCommandResource;
PVOID _imp_mciSendCommandA;
PVOID _imp_mciSendCommandW;
PVOID _imp_mciSendStringA;
PVOID _imp_mciSendStringW;
PVOID _imp_mciSetDriverData;
PVOID _imp_mciSetYieldProc;
PVOID _imp_mciEatCommandEntry;
PVOID _imp_mciGetParamSize;
PVOID _imp_mciUnlockCommandTable;
PVOID _imp_FindCommandItem;

// MIDI Functions
PVOID _imp_midiConnect;
PVOID _imp_midiDisconnect;
PVOID _imp_midiInAddBuffer;
PVOID _imp_midiInClose;
PVOID _imp_midiInGetDevCapsA;
PVOID _imp_midiInGetDevCapsW;
PVOID _imp_midiInGetErrorTextA;
PVOID _imp_midiInGetErrorTextW;
PVOID _imp_midiInGetID;
PVOID _imp_midiInGetNumDevs;
PVOID _imp_midiInMessage;
PVOID _imp_midiInOpen;
PVOID _imp_midiInPrepareHeader;
PVOID _imp_midiInReset;
PVOID _imp_midiInStart;
PVOID _imp_midiInStop;
PVOID _imp_midiInUnprepareHeader;
PVOID _imp_midiOutCacheDrumPatches;
PVOID _imp_midiOutCachePatches;
PVOID _imp_midiOutClose;
PVOID _imp_midiOutGetDevCapsA;
PVOID _imp_midiOutGetDevCapsW;
PVOID _imp_midiOutGetErrorTextA;
PVOID _imp_midiOutGetErrorTextW;
PVOID _imp_midiOutGetID;
PVOID _imp_midiOutGetNumDevs;
PVOID _imp_midiOutGetVolume;
PVOID _imp_midiOutLongMsg;
PVOID _imp_midiOutMessage;
PVOID _imp_midiOutOpen;
PVOID _imp_midiOutPrepareHeader;
PVOID _imp_midiOutReset;
PVOID _imp_midiOutSetVolume;
PVOID _imp_midiOutShortMsg;
PVOID _imp_midiOutUnprepareHeader;
PVOID _imp_midiStreamClose;
PVOID _imp_midiStreamOpen;
PVOID _imp_midiStreamOut;
PVOID _imp_midiStreamPause;
PVOID _imp_midiStreamPosition;
PVOID _imp_midiStreamProperty;
PVOID _imp_midiStreamRestart;
PVOID _imp_midiStreamStop;

// Mixer Functions
PVOID _imp_mixerClose;
PVOID _imp_mixerGetControlDetailsA;
PVOID _imp_mixerGetControlDetailsW;
PVOID _imp_mixerGetDevCapsA;
PVOID _imp_mixerGetDevCapsW;
PVOID _imp_mixerGetID;
PVOID _imp_mixerGetLineControlsA;
PVOID _imp_mixerGetLineControlsW;
PVOID _imp_mixerGetLineInfoA;
PVOID _imp_mixerGetLineInfoW;
PVOID _imp_mixerGetNumDevs;
PVOID _imp_mixerMessage;
PVOID _imp_mixerOpen;
PVOID _imp_mixerSetControlDetails;

// Task Functions
PVOID _imp_mmDrvInstall;
PVOID _imp_mmGetCurrentTask;
PVOID _imp_mmTaskBlock;
PVOID _imp_mmTaskCreate;
PVOID _imp_mmTaskSignal;
PVOID _imp_mmTaskYield;

// Multimedia io Functions
PVOID _imp_mmioAdvance;
PVOID _imp_mmioAscend;
PVOID _imp_mmioClose;
PVOID _imp_mmioCreateChunk;
PVOID _imp_mmioDescend;
PVOID _imp_mmioFlush;
PVOID _imp_mmioGetInfo;
PVOID _imp_mmioInstallIOProcA;
PVOID _imp_mmioInstallIOProcW;
PVOID _imp_mmioOpenA;
PVOID _imp_mmioOpenW;
PVOID _imp_mmioRead;
PVOID _imp_mmioRenameA;
PVOID _imp_mmioRenameW;
PVOID _imp_mmioSeek;
PVOID _imp_mmioSendMessage;
PVOID _imp_mmioSetBuffer;
PVOID _imp_mmioSetInfo;
PVOID _imp_mmioStringToFOURCCA;
PVOID _imp_mmioStringToFOURCCW;
PVOID _imp_mmioWrite;

// Time Functions
PVOID _imp_timeBeginPeriod;
PVOID _imp_timeEndPeriod;
PVOID _imp_timeGetDevCaps;
PVOID _imp_timeGetSystemTime;
PVOID _imp_timeGetTime;
PVOID _imp_timeKillEvent;
PVOID _imp_timeSetEvent;

// Wave Functions
PVOID _imp_waveInAddBuffer;
PVOID _imp_waveInClose;
PVOID _imp_waveInGetDevCapsA;
PVOID _imp_waveInGetDevCapsW;
PVOID _imp_waveInGetErrorTextA;
PVOID _imp_waveInGetErrorTextW;
PVOID _imp_waveInGetID;
PVOID _imp_waveInGetNumDevs;
PVOID _imp_waveInGetPosition;
PVOID _imp_waveInMessage;
PVOID _imp_waveInOpen;
PVOID _imp_waveInPrepareHeader;
PVOID _imp_waveInReset;
PVOID _imp_waveInStart;
PVOID _imp_waveInStop;
PVOID _imp_waveInUnprepareHeader;
PVOID _imp_waveOutBreakLoop;
PVOID _imp_waveOutClose;
PVOID _imp_waveOutGetDevCapsA;
PVOID _imp_waveOutGetDevCapsW;
PVOID _imp_waveOutGetErrorTextA;
PVOID _imp_waveOutGetErrorTextW;
PVOID _imp_waveOutGetID;
PVOID _imp_waveOutGetNumDevs;
PVOID _imp_waveOutGetPitch;
PVOID _imp_waveOutGetPlaybackRate;
PVOID _imp_waveOutGetPosition;
PVOID _imp_waveOutGetVolume;
PVOID _imp_waveOutMessage;
PVOID _imp_waveOutOpen;
PVOID _imp_waveOutPause;
PVOID _imp_waveOutPrepareHeader;
PVOID _imp_waveOutReset;
PVOID _imp_waveOutRestart;
PVOID _imp_waveOutSetPitch;
PVOID _imp_waveOutSetPlaybackRate;
PVOID _imp_waveOutSetVolume;
PVOID _imp_waveOutUnprepareHeader;
PVOID _imp_waveOutWrite;

// Message Functions
PVOID _imp_aux32Message;
PVOID _imp_mid32Message;
PVOID _imp_joy32Message;
PVOID _imp_mci32Message;
PVOID _imp_mod32Message;
PVOID _imp_mxd32Message;
PVOID _imp_tid32Message;
PVOID _imp_wid32Message;
PVOID _imp_wod32Message;
}

#undef PlaySound
#define GETADDRESS(s) _imp_ ## s = GetProcAddress(hLibModule, #s)
#define UPDATEPOINTER(a, b) if (!_imp_ ## a) _imp_ ## a = _imp_ ## b; else if (!_imp_ ## b) _imp_ ## b = _imp_ ## a
BOOL WINAPI NsGetAddress(HMODULE hLibModule) {
    GETADDRESS(NT4PlaySound);
    GETADDRESS(PlaySound);
    GETADDRESS(sndPlaySoundA);
    GETADDRESS(sndPlaySoundW);
    GETADDRESS(PlaySoundA);
    GETADDRESS(PlaySoundW);

    // Driver and Drv help Functions
    GETADDRESS(CloseDriver);
    GETADDRESS(DefDriverProc);
    GETADDRESS(DriverCallback);
    GETADDRESS(DrvClose);
    GETADDRESS(DrvGetModuleHandle);
    GETADDRESS(DrvOpen);
    GETADDRESS(DrvSendMessage);
    GETADDRESS(GetDriverModuleHandle);
    GETADDRESS(MigrateSoundEvents);
    GETADDRESS(MigrateAllDrivers);
    GETADDRESS(mmsystemGetVersion);
    GETADDRESS(NotifyCallbackData);
    GETADDRESS(OpenDriver);
    GETADDRESS(SendDriverMessage);
    GETADDRESS(WOW32DriverCallback);
    GETADDRESS(WOW32ResolveMultiMediaHandle);
    GETADDRESS(WOWAppExit);
    GETADDRESS(winmmDbgOut);
    GETADDRESS(WinmmLogon);
    GETADDRESS(WinmmLogoff);
    GETADDRESS(winmmSetDebugLevel);

    // Audio GFX support Functions
    GETADDRESS(gfxCreateZoneFactoriesList);
    GETADDRESS(gfxCreateGfxFactoriesList);
    GETADDRESS(gfxDestroyDeviceInterfaceList);
    GETADDRESS(gfxEnumerateGfxs);
    GETADDRESS(gfxRemoveGfx);
    GETADDRESS(gfxAddGfx);
    GETADDRESS(gfxModifyGfx);
    GETADDRESS(gfxOpenGfx);
    GETADDRESS(gfxBatchChange);

    // AUX Functions
    GETADDRESS(auxGetDevCapsA);
    GETADDRESS(auxGetDevCapsW);
    GETADDRESS(auxGetNumDevs);
    GETADDRESS(auxGetVolume);
    GETADDRESS(auxOutMessage);
    GETADDRESS(auxSetVolume);

    // Joystick Functions
    GETADDRESS(joyConfigChanged);
    GETADDRESS(joySetCalibration);
    GETADDRESS(joyGetDevCapsA);
    GETADDRESS(joyGetDevCapsW);
    GETADDRESS(joyGetNumDevs);
    GETADDRESS(joyGetPos);
    GETADDRESS(joyGetPosEx);
    GETADDRESS(joyGetThreshold);
    GETADDRESS(joyReleaseCapture);
    GETADDRESS(joySetCapture);
    GETADDRESS(joySetThreshold);

    // MCI Functions
    GETADDRESS(mciExecute);
    GETADDRESS(mciDriverNotify);
    GETADDRESS(mciDriverYield);
    GETADDRESS(mciFreeCommandResource);
    GETADDRESS(mciGetCreatorTask);
    GETADDRESS(mciGetDeviceIDA);
    GETADDRESS(mciGetDeviceIDFromElementIDA);
    GETADDRESS(mciGetDeviceIDFromElementIDW);
    GETADDRESS(mciGetDeviceIDW);
    GETADDRESS(mciGetDriverData);
    GETADDRESS(mciGetErrorStringA);
    GETADDRESS(mciGetErrorStringW);
    GETADDRESS(mciGetYieldProc);
    GETADDRESS(mciLoadCommandResource);
    GETADDRESS(mciSendCommandA);
    GETADDRESS(mciSendCommandW);
    GETADDRESS(mciSendStringA);
    GETADDRESS(mciSendStringW);
    GETADDRESS(mciSetDriverData);
    GETADDRESS(mciSetYieldProc);
    GETADDRESS(mciEatCommandEntry);
    GETADDRESS(mciGetParamSize);
    GETADDRESS(mciUnlockCommandTable);
    GETADDRESS(FindCommandItem);

    // MIDI Functions
    GETADDRESS(midiConnect);
    GETADDRESS(midiDisconnect);
    GETADDRESS(midiInAddBuffer);
    GETADDRESS(midiInClose);
    GETADDRESS(midiInGetDevCapsA);
    GETADDRESS(midiInGetDevCapsW);
    GETADDRESS(midiInGetErrorTextA);
    GETADDRESS(midiInGetErrorTextW);
    GETADDRESS(midiInGetID);
    GETADDRESS(midiInGetNumDevs);
    GETADDRESS(midiInMessage);
    GETADDRESS(midiInOpen);
    GETADDRESS(midiInPrepareHeader);
    GETADDRESS(midiInReset);
    GETADDRESS(midiInStart);
    GETADDRESS(midiInStop);
    GETADDRESS(midiInUnprepareHeader);
    GETADDRESS(midiOutCacheDrumPatches);
    GETADDRESS(midiOutCachePatches);
    GETADDRESS(midiOutClose);
    GETADDRESS(midiOutGetDevCapsA);
    GETADDRESS(midiOutGetDevCapsW);
    GETADDRESS(midiOutGetErrorTextA);
    GETADDRESS(midiOutGetErrorTextW);
    GETADDRESS(midiOutGetID);
    GETADDRESS(midiOutGetNumDevs);
    GETADDRESS(midiOutGetVolume);
    GETADDRESS(midiOutLongMsg);
    GETADDRESS(midiOutMessage);
    GETADDRESS(midiOutOpen);
    GETADDRESS(midiOutPrepareHeader);
    GETADDRESS(midiOutReset);
    GETADDRESS(midiOutSetVolume);
    GETADDRESS(midiOutShortMsg);
    GETADDRESS(midiOutUnprepareHeader);
    GETADDRESS(midiStreamClose);
    GETADDRESS(midiStreamOpen);
    GETADDRESS(midiStreamOut);
    GETADDRESS(midiStreamPause);
    GETADDRESS(midiStreamPosition);
    GETADDRESS(midiStreamProperty);
    GETADDRESS(midiStreamRestart);
    GETADDRESS(midiStreamStop);

    // Mixer Functions
    GETADDRESS(mixerClose);
    GETADDRESS(mixerGetControlDetailsA);
    GETADDRESS(mixerGetControlDetailsW);
    GETADDRESS(mixerGetDevCapsA);
    GETADDRESS(mixerGetDevCapsW);
    GETADDRESS(mixerGetID);
    GETADDRESS(mixerGetLineControlsA);
    GETADDRESS(mixerGetLineControlsW);
    GETADDRESS(mixerGetLineInfoA);
    GETADDRESS(mixerGetLineInfoW);
    GETADDRESS(mixerGetNumDevs);
    GETADDRESS(mixerMessage);
    GETADDRESS(mixerOpen);
    GETADDRESS(mixerSetControlDetails);

    // Task Functions
    GETADDRESS(mmDrvInstall);
    GETADDRESS(mmGetCurrentTask);
    GETADDRESS(mmTaskBlock);
    GETADDRESS(mmTaskCreate);
    GETADDRESS(mmTaskSignal);
    GETADDRESS(mmTaskYield);

    // Multimedia io Functions
    GETADDRESS(mmioAdvance);
    GETADDRESS(mmioAscend);
    GETADDRESS(mmioClose);
    GETADDRESS(mmioCreateChunk);
    GETADDRESS(mmioDescend);
    GETADDRESS(mmioFlush);
    GETADDRESS(mmioGetInfo);
    GETADDRESS(mmioInstallIOProcA);
    GETADDRESS(mmioInstallIOProcW);
    GETADDRESS(mmioOpenA);
    GETADDRESS(mmioOpenW);
    GETADDRESS(mmioRead);
    GETADDRESS(mmioRenameA);
    GETADDRESS(mmioRenameW);
    GETADDRESS(mmioSeek);
    GETADDRESS(mmioSendMessage);
    GETADDRESS(mmioSetBuffer);
    GETADDRESS(mmioSetInfo);
    GETADDRESS(mmioStringToFOURCCA);
    GETADDRESS(mmioStringToFOURCCW);
    GETADDRESS(mmioWrite);

    // Time Functions
    GETADDRESS(timeBeginPeriod);
    GETADDRESS(timeEndPeriod);
    GETADDRESS(timeGetDevCaps);
    GETADDRESS(timeGetSystemTime);
    GETADDRESS(timeGetTime);
    GETADDRESS(timeKillEvent);
    GETADDRESS(timeSetEvent);

    // Wave Functions
    GETADDRESS(waveInAddBuffer);
    GETADDRESS(waveInClose);
    GETADDRESS(waveInGetDevCapsA);
    GETADDRESS(waveInGetDevCapsW);
    GETADDRESS(waveInGetErrorTextA);
    GETADDRESS(waveInGetErrorTextW);
    GETADDRESS(waveInGetID);
    GETADDRESS(waveInGetNumDevs);
    GETADDRESS(waveInGetPosition);
    GETADDRESS(waveInMessage);
    GETADDRESS(waveInOpen);
    GETADDRESS(waveInPrepareHeader);
    GETADDRESS(waveInReset);
    GETADDRESS(waveInStart);
    GETADDRESS(waveInStop);
    GETADDRESS(waveInUnprepareHeader);
    GETADDRESS(waveOutBreakLoop);
    GETADDRESS(waveOutClose);
    GETADDRESS(waveOutGetDevCapsA);
    GETADDRESS(waveOutGetDevCapsW);
    GETADDRESS(waveOutGetErrorTextA);
    GETADDRESS(waveOutGetErrorTextW);
    GETADDRESS(waveOutGetID);
    GETADDRESS(waveOutGetNumDevs);
    GETADDRESS(waveOutGetPitch);
    GETADDRESS(waveOutGetPlaybackRate);
    GETADDRESS(waveOutGetPosition);
    GETADDRESS(waveOutGetVolume);
    GETADDRESS(waveOutMessage);
    GETADDRESS(waveOutOpen);
    GETADDRESS(waveOutPause);
    GETADDRESS(waveOutPrepareHeader);
    GETADDRESS(waveOutReset);
    GETADDRESS(waveOutRestart);
    GETADDRESS(waveOutSetPitch);
    GETADDRESS(waveOutSetPlaybackRate);
    GETADDRESS(waveOutSetVolume);
    GETADDRESS(waveOutUnprepareHeader);
    GETADDRESS(waveOutWrite);

    // Message Functions
    GETADDRESS(aux32Message);
    GETADDRESS(mid32Message);
    GETADDRESS(joy32Message);
    GETADDRESS(mci32Message);
    GETADDRESS(mod32Message);
    GETADDRESS(mxd32Message);
    GETADDRESS(tid32Message);
    GETADDRESS(wid32Message);
    GETADDRESS(wod32Message);

    UPDATEPOINTER(PlaySound, PlaySoundA);
    if (!_imp_NT4PlaySound) _imp_NT4PlaySound = _imp_PlaySound;
    else if (!_imp_PlaySoundA) _imp_PlaySound = _imp_PlaySoundA = _imp_NT4PlaySound;
    UPDATEPOINTER(CloseDriver, DrvClose);
    UPDATEPOINTER(OpenDriver, DrvOpen);
    UPDATEPOINTER(SendDriverMessage, DrvSendMessage);
    UPDATEPOINTER(GetDriverModuleHandle, DrvGetModuleHandle);
    return TRUE;
}
#undef UPDATEPOINTER
#undef GETADDRESS
#ifdef UNICODE
#define PlaySound  PlaySoundW
#else
#define PlaySound  PlaySoundA
#endif
#pragma pack(pop)
