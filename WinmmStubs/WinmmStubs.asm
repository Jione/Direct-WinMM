; asm code for WinMM.dll x64 binary

.DATA
; PlaySound Functions
EXTERN pfn_NT4PlaySound:dq;
EXTERN pfn_PlaySound:dq;
EXTERN pfn_sndPlaySoundA:dq;
EXTERN pfn_sndPlaySoundW:dq;
EXTERN pfn_PlaySoundA:dq;
EXTERN pfn_PlaySoundW:dq;

; Driver and Drv help Functions
EXTERN pfn_CloseDriver:dq;
EXTERN pfn_OpenDriver:dq;
EXTERN pfn_SendDriverMessage:dq;
EXTERN pfn_DrvGetModuleHandle:dq;
EXTERN pfn_GetDriverModuleHandle:dq;
EXTERN pfn_DefDriverProc:dq;
EXTERN pfn_DrvClose:dq;
EXTERN pfn_DriverCallback:dq;
EXTERN pfn_DrvOpen:dq;
EXTERN pfn_DrvSendMessage:dq;
EXTERN pfn_mmsystemGetVersion:dq;
EXTERN pfn_NotifyCallbackData:dq;
EXTERN pfn_winmmDbgOut:dq;
EXTERN pfn_winmmSetDebugLevel:dq;
EXTERN pfn_MigrateAllDrivers:dq;
EXTERN pfn_MigrateSoundEvents:dq;
EXTERN pfn_WinmmLogon:dq;
EXTERN pfn_WinmmLogoff:dq;
EXTERN pfn_WOW32DriverCallback:dq;
EXTERN pfn_WOW32ResolveMultiMediaHandle:dq;
EXTERN pfn_WOWAppExit:dq;

; Audio GFX support Functions
EXTERN pfn_gfxCreateZoneFactoriesList:dq;
EXTERN pfn_gfxCreateGfxFactoriesList:dq;
EXTERN pfn_gfxDestroyDeviceInterfaceList:dq;
EXTERN pfn_gfxEnumerateGfxs:dq;
EXTERN pfn_gfxRemoveGfx:dq;
EXTERN pfn_gfxAddGfx:dq;
EXTERN pfn_gfxModifyGfx:dq;
EXTERN pfn_gfxOpenGfx:dq;
EXTERN pfn_gfxBatchChange:dq;

; AUX Functions
EXTERN pfn_auxGetDevCapsA:dq;
EXTERN pfn_auxGetDevCapsW:dq;
EXTERN pfn_auxGetNumDevs:dq;
EXTERN pfn_auxGetVolume:dq;
EXTERN pfn_auxOutMessage:dq;
EXTERN pfn_auxSetVolume:dq;

; Joystick Functions
EXTERN pfn_joyGetNumDevs:dq;
EXTERN pfn_joyGetDevCapsA:dq;
EXTERN pfn_joyGetDevCapsW:dq;
EXTERN pfn_joyGetPos:dq;
EXTERN pfn_joyGetPosEx:dq;
EXTERN pfn_joyGetThreshold:dq;
EXTERN pfn_joyReleaseCapture:dq;
EXTERN pfn_joySetCapture:dq;
EXTERN pfn_joySetThreshold:dq;
EXTERN pfn_joySetCalibration:dq;
EXTERN pfn_joyConfigChanged:dq;

; MCI Functions
EXTERN pfn_mciExecute:dq;
EXTERN pfn_mciSendCommandA:dq;
EXTERN pfn_mciSendCommandW:dq;
EXTERN pfn_mciSendStringA:dq;
EXTERN pfn_mciSendStringW:dq;
EXTERN pfn_mciGetDeviceIDA:dq;
EXTERN pfn_mciGetDeviceIDW:dq;
EXTERN pfn_mciGetDeviceIDFromElementIDA:dq;
EXTERN pfn_mciGetDeviceIDFromElementIDW:dq;
EXTERN pfn_mciGetErrorStringA:dq;
EXTERN pfn_mciGetErrorStringW:dq;
EXTERN pfn_mciSetYieldProc:dq;
EXTERN pfn_mciGetCreatorTask:dq;
EXTERN pfn_mciGetYieldProc:dq;
EXTERN pfn_mciDriverNotify:dq;
EXTERN pfn_mciDriverYield:dq;
EXTERN pfn_mciFreeCommandResource:dq;
EXTERN pfn_mciGetDriverData:dq;
EXTERN pfn_mciLoadCommandResource:dq;
EXTERN pfn_mciSetDriverData:dq;
EXTERN pfn_mciEatCommandEntry:dq;
EXTERN pfn_mciGetParamSize:dq;
EXTERN pfn_mciUnlockCommandTable:dq;
EXTERN pfn_FindCommandItem:dq;

; MIDI Functions
EXTERN pfn_midiOutGetNumDevs:dq;
EXTERN pfn_midiStreamOpen:dq;
EXTERN pfn_midiStreamClose:dq;
EXTERN pfn_midiStreamProperty:dq;
EXTERN pfn_midiStreamPosition:dq;
EXTERN pfn_midiStreamOut:dq;
EXTERN pfn_midiStreamPause:dq;
EXTERN pfn_midiStreamRestart:dq;
EXTERN pfn_midiStreamStop:dq;
EXTERN pfn_midiConnect:dq;
EXTERN pfn_midiDisconnect:dq;
EXTERN pfn_midiOutGetDevCapsA:dq;
EXTERN pfn_midiOutGetDevCapsW:dq;
EXTERN pfn_midiOutGetVolume:dq;
EXTERN pfn_midiOutSetVolume:dq;
EXTERN pfn_midiOutGetErrorTextA:dq;
EXTERN pfn_midiOutGetErrorTextW:dq;
EXTERN pfn_midiOutOpen:dq;
EXTERN pfn_midiOutClose:dq;
EXTERN pfn_midiOutPrepareHeader:dq;
EXTERN pfn_midiOutUnprepareHeader:dq;
EXTERN pfn_midiOutShortMsg:dq;
EXTERN pfn_midiOutLongMsg:dq;
EXTERN pfn_midiOutReset:dq;
EXTERN pfn_midiOutCachePatches:dq;
EXTERN pfn_midiOutCacheDrumPatches:dq;
EXTERN pfn_midiOutGetID:dq;
EXTERN pfn_midiOutMessage:dq;
EXTERN pfn_midiInGetNumDevs:dq;
EXTERN pfn_midiInGetDevCapsA:dq;
EXTERN pfn_midiInGetDevCapsW:dq;
EXTERN pfn_midiInGetErrorTextA:dq;
EXTERN pfn_midiInGetErrorTextW:dq;
EXTERN pfn_midiInOpen:dq;
EXTERN pfn_midiInClose:dq;
EXTERN pfn_midiInPrepareHeader:dq;
EXTERN pfn_midiInUnprepareHeader:dq;
EXTERN pfn_midiInAddBuffer:dq;
EXTERN pfn_midiInStart:dq;
EXTERN pfn_midiInStop:dq;
EXTERN pfn_midiInReset:dq;
EXTERN pfn_midiInGetID:dq;
EXTERN pfn_midiInMessage:dq;

; Mixer Functions
EXTERN pfn_mixerGetNumDevs:dq;
EXTERN pfn_mixerGetDevCapsA:dq;
EXTERN pfn_mixerGetDevCapsW:dq;
EXTERN pfn_mixerOpen:dq;
EXTERN pfn_mixerClose:dq;
EXTERN pfn_mixerMessage:dq;
EXTERN pfn_mixerGetLineInfoA:dq;
EXTERN pfn_mixerGetLineInfoW:dq;
EXTERN pfn_mixerGetID:dq;
EXTERN pfn_mixerGetLineControlsA:dq;
EXTERN pfn_mixerGetLineControlsW:dq;
EXTERN pfn_mixerGetControlDetailsA:dq;
EXTERN pfn_mixerGetControlDetailsW:dq;
EXTERN pfn_mixerSetControlDetails:dq;

; Task Functions
EXTERN pfn_mmDrvInstall:dq;
EXTERN pfn_mmGetCurrentTask:dq;
EXTERN pfn_mmTaskBlock:dq;
EXTERN pfn_mmTaskCreate:dq;
EXTERN pfn_mmTaskSignal:dq;
EXTERN pfn_mmTaskYield:dq;

; Multimedia io Functions
EXTERN pfn_mmioStringToFOURCCA:dq;
EXTERN pfn_mmioStringToFOURCCW:dq;
EXTERN pfn_mmioInstallIOProcA:dq;
EXTERN pfn_mmioInstallIOProcW:dq;
EXTERN pfn_mmioOpenA:dq;
EXTERN pfn_mmioOpenW:dq;
EXTERN pfn_mmioRenameA:dq;
EXTERN pfn_mmioRenameW:dq;
EXTERN pfn_mmioClose:dq;
EXTERN pfn_mmioRead:dq;
EXTERN pfn_mmioWrite:dq;
EXTERN pfn_mmioSeek:dq;
EXTERN pfn_mmioGetInfo:dq;
EXTERN pfn_mmioSetInfo:dq;
EXTERN pfn_mmioSetBuffer:dq;
EXTERN pfn_mmioFlush:dq;
EXTERN pfn_mmioAdvance:dq;
EXTERN pfn_mmioSendMessage:dq;
EXTERN pfn_mmioDescend:dq;
EXTERN pfn_mmioAscend:dq;
EXTERN pfn_mmioCreateChunk:dq;

; Time Functions
EXTERN pfn_timeGetSystemTime:dq;
EXTERN pfn_timeGetTime:dq;
EXTERN pfn_timeSetEvent:dq;
EXTERN pfn_timeKillEvent:dq;
EXTERN pfn_timeGetDevCaps:dq;
EXTERN pfn_timeBeginPeriod:dq;
EXTERN pfn_timeEndPeriod:dq;

; Wave Functions
EXTERN pfn_waveOutGetNumDevs:dq;
EXTERN pfn_waveOutGetDevCapsA:dq;
EXTERN pfn_waveOutGetDevCapsW:dq;
EXTERN pfn_waveOutGetVolume:dq;
EXTERN pfn_waveOutSetVolume:dq;
EXTERN pfn_waveOutGetErrorTextA:dq;
EXTERN pfn_waveOutGetErrorTextW:dq;
EXTERN pfn_waveOutOpen:dq;
EXTERN pfn_waveOutClose:dq;
EXTERN pfn_waveOutPrepareHeader:dq;
EXTERN pfn_waveOutUnprepareHeader:dq;
EXTERN pfn_waveOutWrite:dq;
EXTERN pfn_waveOutPause:dq;
EXTERN pfn_waveOutRestart:dq;
EXTERN pfn_waveOutReset:dq;
EXTERN pfn_waveOutBreakLoop:dq;
EXTERN pfn_waveOutGetPosition:dq;
EXTERN pfn_waveOutGetPitch:dq;
EXTERN pfn_waveOutSetPitch:dq;
EXTERN pfn_waveOutGetPlaybackRate:dq;
EXTERN pfn_waveOutSetPlaybackRate:dq;
EXTERN pfn_waveOutGetID:dq;
EXTERN pfn_waveOutMessage:dq;
EXTERN pfn_waveInGetNumDevs:dq;
EXTERN pfn_waveInGetDevCapsA:dq;
EXTERN pfn_waveInGetDevCapsW:dq;
EXTERN pfn_waveInGetErrorTextA:dq;
EXTERN pfn_waveInGetErrorTextW:dq;
EXTERN pfn_waveInOpen:dq;
EXTERN pfn_waveInClose:dq;
EXTERN pfn_waveInPrepareHeader:dq;
EXTERN pfn_waveInUnprepareHeader:dq;
EXTERN pfn_waveInAddBuffer:dq;
EXTERN pfn_waveInStart:dq;
EXTERN pfn_waveInStop:dq;
EXTERN pfn_waveInReset:dq;
EXTERN pfn_waveInGetPosition:dq;
EXTERN pfn_waveInGetID:dq;
EXTERN pfn_waveInMessage:dq;

; Message Functions
EXTERN pfn_aux32Message:dq;
EXTERN pfn_joy32Message:dq;
EXTERN pfn_mci32Message:dq;
EXTERN pfn_mid32Message:dq;
EXTERN pfn_mod32Message:dq;
EXTERN pfn_mxd32Message:dq;
EXTERN pfn_tid32Message:dq;
EXTERN pfn_wid32Message:dq;
EXTERN pfn_wod32Message:dq;


.CODE
; PlaySound Functions
NT4PlaySound PROC
  cmp pfn_NT4PlaySound, 0
  jz ERROR
  jmp pfn_NT4PlaySound
ERROR:
  mov RAX, 0
  ret
NT4PlaySound ENDP
PlaySound PROC
  cmp pfn_PlaySound, 0
  jz ERROR
  jmp pfn_PlaySound
ERROR:
  mov RAX, 0
  ret
PlaySound ENDP
sndPlaySoundA PROC
  cmp pfn_sndPlaySoundA, 0
  jz ERROR
  jmp pfn_sndPlaySoundA
ERROR:
  mov RAX, 0
  ret
sndPlaySoundA ENDP
sndPlaySoundW PROC
  cmp pfn_sndPlaySoundW, 0
  jz ERROR
  jmp pfn_sndPlaySoundW
ERROR:
  mov RAX, 0
  ret
sndPlaySoundW ENDP
PlaySoundA PROC
  cmp pfn_PlaySoundA, 0
  jz ERROR
  jmp pfn_PlaySoundA
ERROR:
  mov RAX, 0
  ret
PlaySoundA ENDP
PlaySoundW PROC
  cmp pfn_PlaySoundW, 0
  jz ERROR
  jmp pfn_PlaySoundW
ERROR:
  mov RAX, 0
  ret
PlaySoundW ENDP

; Driver and Drv help Functions
CloseDriver PROC
  cmp pfn_CloseDriver, 0
  jz ERROR
  jmp pfn_CloseDriver
ERROR:
  mov RAX, -1
  ret
CloseDriver ENDP
OpenDriver PROC
  cmp pfn_OpenDriver, 0
  jz ERROR
  jmp pfn_OpenDriver
ERROR:
  mov RAX, 0
  ret
OpenDriver ENDP
SendDriverMessage PROC
  cmp pfn_SendDriverMessage, 0
  jz ERROR
  jmp pfn_SendDriverMessage
ERROR:
  mov RAX, -1
  ret
SendDriverMessage ENDP
DrvGetModuleHandle PROC
  cmp pfn_DrvGetModuleHandle, 0
  jz ERROR
  jmp pfn_DrvGetModuleHandle
ERROR:
  mov RAX, 0
  ret
DrvGetModuleHandle ENDP
GetDriverModuleHandle PROC
  cmp pfn_GetDriverModuleHandle, 0
  jz ERROR
  jmp pfn_GetDriverModuleHandle
ERROR:
  mov RAX, 0
  ret
GetDriverModuleHandle ENDP
DefDriverProc PROC
  cmp pfn_DefDriverProc, 0
  jz ERROR
  jmp pfn_DefDriverProc
ERROR:
  mov RAX, -1
  ret
DefDriverProc ENDP
DrvClose PROC
  cmp pfn_DrvClose, 0
  jz ERROR
  jmp pfn_DrvClose
ERROR:
  mov RAX, -1
  ret
DrvClose ENDP
DriverCallback PROC
  cmp pfn_DriverCallback, 0
  jz ERROR
  jmp pfn_DriverCallback
ERROR:
  mov RAX, 0
  ret
DriverCallback ENDP
DrvOpen PROC
  cmp pfn_DrvOpen, 0
  jz ERROR
  jmp pfn_DrvOpen
ERROR:
  mov RAX, 0
  ret
DrvOpen ENDP
DrvSendMessage PROC
  cmp pfn_DrvSendMessage, 0
  jz ERROR
  jmp pfn_DrvSendMessage
ERROR:
  mov RAX, -1
  ret
DrvSendMessage ENDP
mmsystemGetVersion PROC
  cmp pfn_mmsystemGetVersion, 0
  jz ERROR
  jmp pfn_mmsystemGetVersion
ERROR:
  mov RAX, 0
  ret
mmsystemGetVersion ENDP
NotifyCallbackData PROC
  cmp pfn_NotifyCallbackData, 0
  jz ERROR
  jmp pfn_NotifyCallbackData
ERROR:
  mov RAX, 0
  ret
NotifyCallbackData ENDP
winmmDbgOut PROC
  cmp pfn_winmmDbgOut, 0
  jz ERROR
  jmp pfn_winmmDbgOut
ERROR:
  ret
winmmDbgOut ENDP
winmmSetDebugLevel PROC
  cmp pfn_winmmSetDebugLevel, 0
  jz ERROR
  jmp pfn_winmmSetDebugLevel
ERROR:
  ret
winmmSetDebugLevel ENDP
MigrateAllDrivers PROC
  cmp pfn_MigrateAllDrivers, 0
  jz ERROR
  jmp pfn_MigrateAllDrivers
ERROR:
  ret
MigrateAllDrivers ENDP
MigrateSoundEvents PROC
  cmp pfn_MigrateSoundEvents, 0
  jz ERROR
  jmp pfn_MigrateSoundEvents
ERROR:
  ret
MigrateSoundEvents ENDP
WinmmLogon PROC
  cmp pfn_WinmmLogon, 0
  jz ERROR
  jmp pfn_WinmmLogon
ERROR:
  ret
WinmmLogon ENDP
WinmmLogoff PROC
  cmp pfn_WinmmLogoff, 0
  jz ERROR
  jmp pfn_WinmmLogoff
ERROR:
  ret
WinmmLogoff ENDP
WOW32DriverCallback PROC
  cmp pfn_WOW32DriverCallback, 0
  jz ERROR
  jmp pfn_WOW32DriverCallback
ERROR:
  mov RAX, 0
  ret
WOW32DriverCallback ENDP
WOW32ResolveMultiMediaHandle PROC
  cmp pfn_WOW32ResolveMultiMediaHandle, 0
  jz ERROR
  jmp pfn_WOW32ResolveMultiMediaHandle
ERROR:
  mov RAX, 0
  ret
WOW32ResolveMultiMediaHandle ENDP
WOWAppExit PROC
  cmp pfn_WOWAppExit, 0
  jz ERROR
  jmp pfn_WOWAppExit
ERROR:
  ret
WOWAppExit ENDP

; Audio GFX support Functions
gfxCreateZoneFactoriesList PROC
  cmp pfn_gfxCreateZoneFactoriesList, 0
  jz ERROR
  jmp pfn_gfxCreateZoneFactoriesList
ERROR:
  mov RAX, 0
  ret
gfxCreateZoneFactoriesList ENDP
gfxCreateGfxFactoriesList PROC
  cmp pfn_gfxCreateGfxFactoriesList, 0
  jz ERROR
  jmp pfn_gfxCreateGfxFactoriesList
ERROR:
  mov RAX, 0
  ret
gfxCreateGfxFactoriesList ENDP
gfxDestroyDeviceInterfaceList PROC
  cmp pfn_gfxDestroyDeviceInterfaceList, 0
  jz ERROR
  jmp pfn_gfxDestroyDeviceInterfaceList
ERROR:
  mov RAX, 0
  ret
gfxDestroyDeviceInterfaceList ENDP
gfxEnumerateGfxs PROC
  cmp pfn_gfxEnumerateGfxs, 0
  jz ERROR
  jmp pfn_gfxEnumerateGfxs
ERROR:
  mov RAX, 0
  ret
gfxEnumerateGfxs ENDP
gfxRemoveGfx PROC
  cmp pfn_gfxRemoveGfx, 0
  jz ERROR
  jmp pfn_gfxRemoveGfx
ERROR:
  mov RAX, 0
  ret
gfxRemoveGfx ENDP
gfxAddGfx PROC
  cmp pfn_gfxAddGfx, 0
  jz ERROR
  jmp pfn_gfxAddGfx
ERROR:
  mov RAX, 0
  ret
gfxAddGfx ENDP
gfxModifyGfx PROC
  cmp pfn_gfxModifyGfx, 0
  jz ERROR
  jmp pfn_gfxModifyGfx
ERROR:
  mov RAX, 0
  ret
gfxModifyGfx ENDP
gfxOpenGfx PROC
  cmp pfn_gfxOpenGfx, 0
  jz ERROR
  jmp pfn_gfxOpenGfx
ERROR:
  mov RAX, 0
  ret
gfxOpenGfx ENDP
gfxBatchChange PROC
  cmp pfn_gfxBatchChange, 0
  jz ERROR
  jmp pfn_gfxBatchChange
ERROR:
  mov RAX, 0
  ret
gfxBatchChange ENDP

; AUX Functions
auxGetDevCapsA PROC
  cmp pfn_auxGetDevCapsA, 0
  jz ERROR
  jmp pfn_auxGetDevCapsA
ERROR:
  mov RAX, 1
  ret
auxGetDevCapsA ENDP
auxGetDevCapsW PROC
  cmp pfn_auxGetDevCapsW, 0
  jz ERROR
  jmp pfn_auxGetDevCapsW
ERROR:
  mov RAX, 1
  ret
auxGetDevCapsW ENDP
auxGetNumDevs PROC
  cmp pfn_auxGetNumDevs, 0
  jz ERROR
  jmp pfn_auxGetNumDevs
ERROR:
  mov RAX, 0
  ret
auxGetNumDevs ENDP
auxGetVolume PROC
  cmp pfn_auxGetVolume, 0
  jz ERROR
  jmp pfn_auxGetVolume
ERROR:
  mov RAX, 1
  ret
auxGetVolume ENDP
auxOutMessage PROC
  cmp pfn_auxOutMessage, 0
  jz ERROR
  jmp pfn_auxOutMessage
ERROR:
  mov RAX, 1
  ret
auxOutMessage ENDP
auxSetVolume PROC
  cmp pfn_auxSetVolume, 0
  jz ERROR
  jmp pfn_auxSetVolume
ERROR:
  mov RAX, 1
  ret
auxSetVolume ENDP

; Joystick Functions
joyGetNumDevs PROC
  cmp pfn_joyGetNumDevs, 0
  jz ERROR
  jmp pfn_joyGetNumDevs
ERROR:
  mov RAX, 0
  ret
joyGetNumDevs ENDP
joyGetDevCapsA PROC
  cmp pfn_joyGetDevCapsA, 0
  jz ERROR
  jmp pfn_joyGetDevCapsA
ERROR:
  mov RAX, 0
  ret
joyGetDevCapsA ENDP
joyGetDevCapsW PROC
  cmp pfn_joyGetDevCapsW, 0
  jz ERROR
  jmp pfn_joyGetDevCapsW
ERROR:
  mov RAX, 1
  ret
joyGetDevCapsW ENDP
joyGetPos PROC
  cmp pfn_joyGetPos, 0
  jz ERROR
  jmp pfn_joyGetPos
ERROR:
  mov RAX, 1
  ret
joyGetPos ENDP
joyGetPosEx PROC
  cmp pfn_joyGetPosEx, 0
  jz ERROR
  jmp pfn_joyGetPosEx
ERROR:
  mov RAX, 1
  ret
joyGetPosEx ENDP
joyGetThreshold PROC
  cmp pfn_joyGetThreshold, 0
  jz ERROR
  jmp pfn_joyGetThreshold
ERROR:
  mov RAX, 1
  ret
joyGetThreshold ENDP
joyReleaseCapture PROC
  cmp pfn_joyReleaseCapture, 0
  jz ERROR
  jmp pfn_joyReleaseCapture
ERROR:
  mov RAX, 1
  ret
joyReleaseCapture ENDP
joySetCapture PROC
  cmp pfn_joySetCapture, 0
  jz ERROR
  jmp pfn_joySetCapture
ERROR:
  mov RAX, 1
  ret
joySetCapture ENDP
joySetThreshold PROC
  cmp pfn_joySetThreshold, 0
  jz ERROR
  jmp pfn_joySetThreshold
ERROR:
  mov RAX, 1
  ret
joySetThreshold ENDP
joySetCalibration PROC
  cmp pfn_joySetCalibration, 0
  jz ERROR
  jmp pfn_joySetCalibration
ERROR:
  mov RAX, -1
  ret
joySetCalibration ENDP
joyConfigChanged PROC
  cmp pfn_joyConfigChanged, 0
  jz ERROR
  jmp pfn_joyConfigChanged
ERROR:
  mov RAX, 1
  ret
joyConfigChanged ENDP

; MCI Functions
mciExecute PROC
  cmp pfn_mciExecute, 0
  jz ERROR
  jmp pfn_mciExecute
ERROR:
  mov RAX, 0
  ret
mciExecute ENDP
mciSendCommandA PROC
  cmp pfn_mciSendCommandA, 0
  jz ERROR
  jmp pfn_mciSendCommandA
ERROR:
  mov RAX, 1
  ret
mciSendCommandA ENDP
mciSendCommandW PROC
  cmp pfn_mciSendCommandW, 0
  jz ERROR
  jmp pfn_mciSendCommandW
ERROR:
  mov RAX, 1
  ret
mciSendCommandW ENDP
mciSendStringA PROC
  cmp pfn_mciSendStringA, 0
  jz ERROR
  jmp pfn_mciSendStringA
ERROR:
  mov RAX, 1
  ret
mciSendStringA ENDP
mciSendStringW PROC
  cmp pfn_mciSendStringW, 0
  jz ERROR
  jmp pfn_mciSendStringW
ERROR:
  mov RAX, 1
  ret
mciSendStringW ENDP
mciGetDeviceIDA PROC
  cmp pfn_mciGetDeviceIDA, 0
  jz ERROR
  jmp pfn_mciGetDeviceIDA
ERROR:
  mov RAX, 0
  ret
mciGetDeviceIDA ENDP
mciGetDeviceIDW PROC
  cmp pfn_mciGetDeviceIDW, 0
  jz ERROR
  jmp pfn_mciGetDeviceIDW
ERROR:
  mov RAX, 0
  ret
mciGetDeviceIDW ENDP
mciGetDeviceIDFromElementIDA PROC
  cmp pfn_mciGetDeviceIDFromElementIDA, 0
  jz ERROR
  jmp pfn_mciGetDeviceIDFromElementIDA
ERROR:
  mov RAX, 0
  ret
mciGetDeviceIDFromElementIDA ENDP
mciGetDeviceIDFromElementIDW PROC
  cmp pfn_mciGetDeviceIDFromElementIDW, 0
  jz ERROR
  jmp pfn_mciGetDeviceIDFromElementIDW
ERROR:
  mov RAX, 0
  ret
mciGetDeviceIDFromElementIDW ENDP
mciGetErrorStringA PROC
  cmp pfn_mciGetErrorStringA, 0
  jz ERROR
  jmp pfn_mciGetErrorStringA
ERROR:
  mov RAX, 0
  ret
mciGetErrorStringA ENDP
mciGetErrorStringW PROC
  cmp pfn_mciGetErrorStringW, 0
  jz ERROR
  jmp pfn_mciGetErrorStringW
ERROR:
  mov RAX, 0
  ret
mciGetErrorStringW ENDP
mciSetYieldProc PROC
  cmp pfn_mciSetYieldProc, 0
  jz ERROR
  jmp pfn_mciSetYieldProc
ERROR:
  mov RAX, 0
  ret
mciSetYieldProc ENDP
mciGetCreatorTask PROC
  cmp pfn_mciGetCreatorTask, 0
  jz ERROR
  jmp pfn_mciGetCreatorTask
ERROR:
  mov RAX, 0
  ret
mciGetCreatorTask ENDP
mciGetYieldProc PROC
  cmp pfn_mciGetYieldProc, 0
  jz ERROR
  jmp pfn_mciGetYieldProc
ERROR:
  mov RAX, 0
  ret
mciGetYieldProc ENDP
mciDriverNotify PROC
  cmp pfn_mciDriverNotify, 0
  jz ERROR
  jmp pfn_mciDriverNotify
ERROR:
  mov RAX, 0
  ret
mciDriverNotify ENDP
mciDriverYield PROC
  cmp pfn_mciDriverYield, 0
  jz ERROR
  jmp pfn_mciDriverYield
ERROR:
  mov RAX, 0
  ret
mciDriverYield ENDP
mciFreeCommandResource PROC
  cmp pfn_mciFreeCommandResource, 0
  jz ERROR
  jmp pfn_mciFreeCommandResource
ERROR:
  mov RAX, 0
  ret
mciFreeCommandResource ENDP
mciGetDriverData PROC
  cmp pfn_mciGetDriverData, 0
  jz ERROR
  jmp pfn_mciGetDriverData
ERROR:
  mov RAX, 0
  ret
mciGetDriverData ENDP
mciLoadCommandResource PROC
  cmp pfn_mciLoadCommandResource, 0
  jz ERROR
  jmp pfn_mciLoadCommandResource
ERROR:
  mov RAX, 0
  ret
mciLoadCommandResource ENDP
mciSetDriverData PROC
  cmp pfn_mciSetDriverData, 0
  jz ERROR
  jmp pfn_mciSetDriverData
ERROR:
  mov RAX, 0
  ret
mciSetDriverData ENDP
mciEatCommandEntry PROC
  cmp pfn_mciEatCommandEntry, 0
  jz ERROR
  jmp pfn_mciEatCommandEntry
ERROR:
  mov RAX, 0
  ret
mciEatCommandEntry ENDP
mciGetParamSize PROC
  cmp pfn_mciGetParamSize, 0
  jz ERROR
  jmp pfn_mciGetParamSize
ERROR:
  mov RAX, 0
  ret
mciGetParamSize ENDP
mciUnlockCommandTable PROC
  cmp pfn_mciUnlockCommandTable, 0
  jz ERROR
  jmp pfn_mciUnlockCommandTable
ERROR:
  mov RAX, 0
  ret
mciUnlockCommandTable ENDP
FindCommandItem PROC
  cmp pfn_FindCommandItem, 0
  jz ERROR
  jmp pfn_FindCommandItem
ERROR:
  mov RAX, 0
  ret
FindCommandItem ENDP

; MIDI Functions
midiOutGetNumDevs PROC
  cmp pfn_midiOutGetNumDevs, 0
  jz ERROR
  jmp pfn_midiOutGetNumDevs
ERROR:
  mov RAX, 0
  ret
midiOutGetNumDevs ENDP
midiStreamOpen PROC
  cmp pfn_midiStreamOpen, 0
  jz ERROR
  jmp pfn_midiStreamOpen
ERROR:
  mov RAX, 1
  ret
midiStreamOpen ENDP
midiStreamClose PROC
  cmp pfn_midiStreamClose, 0
  jz ERROR
  jmp pfn_midiStreamClose
ERROR:
  mov RAX, 1
  ret
midiStreamClose ENDP
midiStreamProperty PROC
  cmp pfn_midiStreamProperty, 0
  jz ERROR
  jmp pfn_midiStreamProperty
ERROR:
  mov RAX, 1
  ret
midiStreamProperty ENDP
midiStreamPosition PROC
  cmp pfn_midiStreamPosition, 0
  jz ERROR
  jmp pfn_midiStreamPosition
ERROR:
  mov RAX, 1
  ret
midiStreamPosition ENDP
midiStreamOut PROC
  cmp pfn_midiStreamOut, 0
  jz ERROR
  jmp pfn_midiStreamOut
ERROR:
  mov RAX, 1
  ret
midiStreamOut ENDP
midiStreamPause PROC
  cmp pfn_midiStreamPause, 0
  jz ERROR
  jmp pfn_midiStreamPause
ERROR:
  mov RAX, 1
  ret
midiStreamPause ENDP
midiStreamRestart PROC
  cmp pfn_midiStreamRestart, 0
  jz ERROR
  jmp pfn_midiStreamRestart
ERROR:
  mov RAX, 1
  ret
midiStreamRestart ENDP
midiStreamStop PROC
  cmp pfn_midiStreamStop, 0
  jz ERROR
  jmp pfn_midiStreamStop
ERROR:
  mov RAX, 1
  ret
midiStreamStop ENDP
midiConnect PROC
  cmp pfn_midiConnect, 0
  jz ERROR
  jmp pfn_midiConnect
ERROR:
  mov RAX, 1
  ret
midiConnect ENDP
midiDisconnect PROC
  cmp pfn_midiDisconnect, 0
  jz ERROR
  jmp pfn_midiDisconnect
ERROR:
  mov RAX, 1
  ret
midiDisconnect ENDP
midiOutGetDevCapsA PROC
  cmp pfn_midiOutGetDevCapsA, 0
  jz ERROR
  jmp pfn_midiOutGetDevCapsA
ERROR:
  mov RAX, 1
  ret
midiOutGetDevCapsA ENDP
midiOutGetDevCapsW PROC
  cmp pfn_midiOutGetDevCapsW, 0
  jz ERROR
  jmp pfn_midiOutGetDevCapsW
ERROR:
  mov RAX, 1
  ret
midiOutGetDevCapsW ENDP
midiOutGetVolume PROC
  cmp pfn_midiOutGetVolume, 0
  jz ERROR
  jmp pfn_midiOutGetVolume
ERROR:
  mov RAX, 1
  ret
midiOutGetVolume ENDP
midiOutSetVolume PROC
  cmp pfn_midiOutSetVolume, 0
  jz ERROR
  jmp pfn_midiOutSetVolume
ERROR:
  mov RAX, 1
  ret
midiOutSetVolume ENDP
midiOutGetErrorTextA PROC
  cmp pfn_midiOutGetErrorTextA, 0
  jz ERROR
  jmp pfn_midiOutGetErrorTextA
ERROR:
  mov RAX, 1
  ret
midiOutGetErrorTextA ENDP
midiOutGetErrorTextW PROC
  cmp pfn_midiOutGetErrorTextW, 0
  jz ERROR
  jmp pfn_midiOutGetErrorTextW
ERROR:
  mov RAX, 1
  ret
midiOutGetErrorTextW ENDP
midiOutOpen PROC
  cmp pfn_midiOutOpen, 0
  jz ERROR
  jmp pfn_midiOutOpen
ERROR:
  mov RAX, 1
  ret
midiOutOpen ENDP
midiOutClose PROC
  cmp pfn_midiOutClose, 0
  jz ERROR
  jmp pfn_midiOutClose
ERROR:
  mov RAX, 1
  ret
midiOutClose ENDP
midiOutPrepareHeader PROC
  cmp pfn_midiOutPrepareHeader, 0
  jz ERROR
  jmp pfn_midiOutPrepareHeader
ERROR:
  mov RAX, 1
  ret
midiOutPrepareHeader ENDP
midiOutUnprepareHeader PROC
  cmp pfn_midiOutUnprepareHeader, 0
  jz ERROR
  jmp pfn_midiOutUnprepareHeader
ERROR:
  mov RAX, 1
  ret
midiOutUnprepareHeader ENDP
midiOutShortMsg PROC
  cmp pfn_midiOutShortMsg, 0
  jz ERROR
  jmp pfn_midiOutShortMsg
ERROR:
  mov RAX, 1
  ret
midiOutShortMsg ENDP
midiOutLongMsg PROC
  cmp pfn_midiOutLongMsg, 0
  jz ERROR
  jmp pfn_midiOutLongMsg
ERROR:
  mov RAX, 1
  ret
midiOutLongMsg ENDP
midiOutReset PROC
  cmp pfn_midiOutReset, 0
  jz ERROR
  jmp pfn_midiOutReset
ERROR:
  mov RAX, 1
  ret
midiOutReset ENDP
midiOutCachePatches PROC
  cmp pfn_midiOutCachePatches, 0
  jz ERROR
  jmp pfn_midiOutCachePatches
ERROR:
  mov RAX, 1
  ret
midiOutCachePatches ENDP
midiOutCacheDrumPatches PROC
  cmp pfn_midiOutCacheDrumPatches, 0
  jz ERROR
  jmp pfn_midiOutCacheDrumPatches
ERROR:
  mov RAX, 1
  ret
midiOutCacheDrumPatches ENDP
midiOutGetID PROC
  cmp pfn_midiOutGetID, 0
  jz ERROR
  jmp pfn_midiOutGetID
ERROR:
  mov RAX, 1
  ret
midiOutGetID ENDP
midiOutMessage PROC
  cmp pfn_midiOutMessage, 0
  jz ERROR
  jmp pfn_midiOutMessage
ERROR:
  mov RAX, 1
  ret
midiOutMessage ENDP
midiInGetNumDevs PROC
  cmp pfn_midiInGetNumDevs, 0
  jz ERROR
  jmp pfn_midiInGetNumDevs
ERROR:
  mov RAX, 0
  ret
midiInGetNumDevs ENDP
midiInGetDevCapsA PROC
  cmp pfn_midiInGetDevCapsA, 0
  jz ERROR
  jmp pfn_midiInGetDevCapsA
ERROR:
  mov RAX, 1
  ret
midiInGetDevCapsA ENDP
midiInGetDevCapsW PROC
  cmp pfn_midiInGetDevCapsW, 0
  jz ERROR
  jmp pfn_midiInGetDevCapsW
ERROR:
  mov RAX, 1
  ret
midiInGetDevCapsW ENDP
midiInGetErrorTextA PROC
  cmp pfn_midiInGetErrorTextA, 0
  jz ERROR
  jmp pfn_midiInGetErrorTextA
ERROR:
  mov RAX, 1
  ret
midiInGetErrorTextA ENDP
midiInGetErrorTextW PROC
  cmp pfn_midiInGetErrorTextW, 0
  jz ERROR
  jmp pfn_midiInGetErrorTextW
ERROR:
  mov RAX, 1
  ret
midiInGetErrorTextW ENDP
midiInOpen PROC
  cmp pfn_midiInOpen, 0
  jz ERROR
  jmp pfn_midiInOpen
ERROR:
  mov RAX, 1
  ret
midiInOpen ENDP
midiInClose PROC
  cmp pfn_midiInClose, 0
  jz ERROR
  jmp pfn_midiInClose
ERROR:
  mov RAX, 1
  ret
midiInClose ENDP
midiInPrepareHeader PROC
  cmp pfn_midiInPrepareHeader, 0
  jz ERROR
  jmp pfn_midiInPrepareHeader
ERROR:
  mov RAX, 1
  ret
midiInPrepareHeader ENDP
midiInUnprepareHeader PROC
  cmp pfn_midiInUnprepareHeader, 0
  jz ERROR
  jmp pfn_midiInUnprepareHeader
ERROR:
  mov RAX, 1
  ret
midiInUnprepareHeader ENDP
midiInAddBuffer PROC
  cmp pfn_midiInAddBuffer, 0
  jz ERROR
  jmp pfn_midiInAddBuffer
ERROR:
  mov RAX, 1
  ret
midiInAddBuffer ENDP
midiInStart PROC
  cmp pfn_midiInStart, 0
  jz ERROR
  jmp pfn_midiInStart
ERROR:
  mov RAX, 1
  ret
midiInStart ENDP
midiInStop PROC
  cmp pfn_midiInStop, 0
  jz ERROR
  jmp pfn_midiInStop
ERROR:
  mov RAX, 1
  ret
midiInStop ENDP
midiInReset PROC
  cmp pfn_midiInReset, 0
  jz ERROR
  jmp pfn_midiInReset
ERROR:
  mov RAX, 1
  ret
midiInReset ENDP
midiInGetID PROC
  cmp pfn_midiInGetID, 0
  jz ERROR
  jmp pfn_midiInGetID
ERROR:
  mov RAX, 1
  ret
midiInGetID ENDP
midiInMessage PROC
  cmp pfn_midiInMessage, 0
  jz ERROR
  jmp pfn_midiInMessage
ERROR:
  mov RAX, 1
  ret
midiInMessage ENDP

; Mixer Functions
mixerGetNumDevs PROC
  cmp pfn_mixerGetNumDevs, 0
  jz ERROR
  jmp pfn_mixerGetNumDevs
ERROR:
  mov RAX, 0
  ret
mixerGetNumDevs ENDP
mixerGetDevCapsA PROC
  cmp pfn_mixerGetDevCapsA, 0
  jz ERROR
  jmp pfn_mixerGetDevCapsA
ERROR:
  mov RAX, 1
  ret
mixerGetDevCapsA ENDP
mixerGetDevCapsW PROC
  cmp pfn_mixerGetDevCapsW, 0
  jz ERROR
  jmp pfn_mixerGetDevCapsW
ERROR:
  mov RAX, 1
  ret
mixerGetDevCapsW ENDP
mixerOpen PROC
  cmp pfn_mixerOpen, 0
  jz ERROR
  jmp pfn_mixerOpen
ERROR:
  mov RAX, 1
  ret
mixerOpen ENDP
mixerClose PROC
  cmp pfn_mixerClose, 0
  jz ERROR
  jmp pfn_mixerClose
ERROR:
  mov RAX, 1
  ret
mixerClose ENDP
mixerMessage PROC
  cmp pfn_mixerMessage, 0
  jz ERROR
  jmp pfn_mixerMessage
ERROR:
  mov RAX, 1
  ret
mixerMessage ENDP
mixerGetLineInfoA PROC
  cmp pfn_mixerGetLineInfoA, 0
  jz ERROR
  jmp pfn_mixerGetLineInfoA
ERROR:
  mov RAX, 1
  ret
mixerGetLineInfoA ENDP
mixerGetLineInfoW PROC
  cmp pfn_mixerGetLineInfoW, 0
  jz ERROR
  jmp pfn_mixerGetLineInfoW
ERROR:
  mov RAX, 1
  ret
mixerGetLineInfoW ENDP
mixerGetID PROC
  cmp pfn_mixerGetID, 0
  jz ERROR
  jmp pfn_mixerGetID
ERROR:
  mov RAX, 1
  ret
mixerGetID ENDP
mixerGetLineControlsA PROC
  cmp pfn_mixerGetLineControlsA, 0
  jz ERROR
  jmp pfn_mixerGetLineControlsA
ERROR:
  mov RAX, 1
  ret
mixerGetLineControlsA ENDP
mixerGetLineControlsW PROC
  cmp pfn_mixerGetLineControlsW, 0
  jz ERROR
  jmp pfn_mixerGetLineControlsW
ERROR:
  mov RAX, 1
  ret
mixerGetLineControlsW ENDP
mixerGetControlDetailsA PROC
  cmp pfn_mixerGetControlDetailsA, 0
  jz ERROR
  jmp pfn_mixerGetControlDetailsA
ERROR:
  mov RAX, 1
  ret
mixerGetControlDetailsA ENDP
mixerGetControlDetailsW PROC
  cmp pfn_mixerGetControlDetailsW, 0
  jz ERROR
  jmp pfn_mixerGetControlDetailsW
ERROR:
  mov RAX, 1
  ret
mixerGetControlDetailsW ENDP
mixerSetControlDetails PROC
  cmp pfn_mixerSetControlDetails, 0
  jz ERROR
  jmp pfn_mixerSetControlDetails
ERROR:
  mov RAX, 1
  ret
mixerSetControlDetails ENDP

; Task Functions
mmDrvInstall PROC
  cmp pfn_mmDrvInstall, 0
  jz ERROR
  jmp pfn_mmDrvInstall
ERROR:
  mov RAX, 0
  ret
mmDrvInstall ENDP
mmGetCurrentTask PROC
  cmp pfn_mmGetCurrentTask, 0
  jz ERROR
  jmp pfn_mmGetCurrentTask
ERROR:
  mov RAX, 0
  ret
mmGetCurrentTask ENDP
mmTaskBlock PROC
  cmp pfn_mmTaskBlock, 0
  jz ERROR
  jmp pfn_mmTaskBlock
ERROR:
  ret
mmTaskBlock ENDP
mmTaskCreate PROC
  cmp pfn_mmTaskCreate, 0
  jz ERROR
  jmp pfn_mmTaskCreate
ERROR:
  mov RAX, 0
  ret
mmTaskCreate ENDP
mmTaskSignal PROC
  cmp pfn_mmTaskSignal, 0
  jz ERROR
  jmp pfn_mmTaskSignal
ERROR:
  mov RAX, 0
  ret
mmTaskSignal ENDP
mmTaskYield PROC
  cmp pfn_mmTaskYield, 0
  jz ERROR
  jmp pfn_mmTaskYield
ERROR:
  ret
mmTaskYield ENDP

; Multimedia io Functions
mmioStringToFOURCCA PROC
  cmp pfn_mmioStringToFOURCCA, 0
  jz ERROR
  jmp pfn_mmioStringToFOURCCA
ERROR:
  mov RAX, 0
  ret
mmioStringToFOURCCA ENDP
mmioStringToFOURCCW PROC
  cmp pfn_mmioStringToFOURCCW, 0
  jz ERROR
  jmp pfn_mmioStringToFOURCCW
ERROR:
  mov RAX, 0
  ret
mmioStringToFOURCCW ENDP
mmioInstallIOProcA PROC
  cmp pfn_mmioInstallIOProcA, 0
  jz ERROR
  jmp pfn_mmioInstallIOProcA
ERROR:
  mov RAX, 0
  ret
mmioInstallIOProcA ENDP
mmioInstallIOProcW PROC
  cmp pfn_mmioInstallIOProcW, 0
  jz ERROR
  jmp pfn_mmioInstallIOProcW
ERROR:
  mov RAX, 0
  ret
mmioInstallIOProcW ENDP
mmioOpenA PROC
  cmp pfn_mmioOpenA, 0
  jz ERROR
  jmp pfn_mmioOpenA
ERROR:
  mov RAX, 1
  ret
mmioOpenA ENDP
mmioOpenW PROC
  cmp pfn_mmioOpenW, 0
  jz ERROR
  jmp pfn_mmioOpenW
ERROR:
  mov RAX, 1
  ret
mmioOpenW ENDP
mmioRenameA PROC
  cmp pfn_mmioRenameA, 0
  jz ERROR
  jmp pfn_mmioRenameA
ERROR:
  mov RAX, 1
  ret
mmioRenameA ENDP
mmioRenameW PROC
  cmp pfn_mmioRenameW, 0
  jz ERROR
  jmp pfn_mmioRenameW
ERROR:
  mov RAX, 1
  ret
mmioRenameW ENDP
mmioClose PROC
  cmp pfn_mmioClose, 0
  jz ERROR
  jmp pfn_mmioClose
ERROR:
  mov RAX, 1
  ret
mmioClose ENDP
mmioRead PROC
  cmp pfn_mmioRead, 0
  jz ERROR
  jmp pfn_mmioRead
ERROR:
  mov RAX, -1
  ret
mmioRead ENDP
mmioWrite PROC
  cmp pfn_mmioWrite, 0
  jz ERROR
  jmp pfn_mmioWrite
ERROR:
  mov RAX, -1
  ret
mmioWrite ENDP
mmioSeek PROC
  cmp pfn_mmioSeek, 0
  jz ERROR
  jmp pfn_mmioSeek
ERROR:
  mov RAX, -1
  ret
mmioSeek ENDP
mmioGetInfo PROC
  cmp pfn_mmioGetInfo, 0
  jz ERROR
  jmp pfn_mmioGetInfo
ERROR:
  mov RAX, 1
  ret
mmioGetInfo ENDP
mmioSetInfo PROC
  cmp pfn_mmioSetInfo, 0
  jz ERROR
  jmp pfn_mmioSetInfo
ERROR:
  mov RAX, 1
  ret
mmioSetInfo ENDP
mmioSetBuffer PROC
  cmp pfn_mmioSetBuffer, 0
  jz ERROR
  jmp pfn_mmioSetBuffer
ERROR:
  mov RAX, 1
  ret
mmioSetBuffer ENDP
mmioFlush PROC
  cmp pfn_mmioFlush, 0
  jz ERROR
  jmp pfn_mmioFlush
ERROR:
  mov RAX, 1
  ret
mmioFlush ENDP
mmioAdvance PROC
  cmp pfn_mmioAdvance, 0
  jz ERROR
  jmp pfn_mmioAdvance
ERROR:
  mov RAX, 1
  ret
mmioAdvance ENDP
mmioSendMessage PROC
  cmp pfn_mmioSendMessage, 0
  jz ERROR
  jmp pfn_mmioSendMessage
ERROR:
  mov RAX, -1
  ret
mmioSendMessage ENDP
mmioDescend PROC
  cmp pfn_mmioDescend, 0
  jz ERROR
  jmp pfn_mmioDescend
ERROR:
  mov RAX, 1
  ret
mmioDescend ENDP
mmioAscend PROC
  cmp pfn_mmioAscend, 0
  jz ERROR
  jmp pfn_mmioAscend
ERROR:
  mov RAX, 1
  ret
mmioAscend ENDP
mmioCreateChunk PROC
  cmp pfn_mmioCreateChunk, 0
  jz ERROR
  jmp pfn_mmioCreateChunk
ERROR:
  mov RAX, 1
  ret
mmioCreateChunk ENDP

; Time Functions
timeGetSystemTime PROC
  cmp pfn_timeGetSystemTime, 0
  jz ERROR
  jmp pfn_timeGetSystemTime
ERROR:
  mov RAX, 1
  ret
timeGetSystemTime ENDP
timeGetTime PROC
  cmp pfn_timeGetTime, 0
  jz ERROR
  jmp pfn_timeGetTime
ERROR:
  mov RAX, 0
  ret
timeGetTime ENDP
timeSetEvent PROC
  cmp pfn_timeSetEvent, 0
  jz ERROR
  jmp pfn_timeSetEvent
ERROR:
  mov RAX, 1
  ret
timeSetEvent ENDP
timeKillEvent PROC
  cmp pfn_timeKillEvent, 0
  jz ERROR
  jmp pfn_timeKillEvent
ERROR:
  mov RAX, 1
  ret
timeKillEvent ENDP
timeGetDevCaps PROC
  cmp pfn_timeGetDevCaps, 0
  jz ERROR
  jmp pfn_timeGetDevCaps
ERROR:
  mov RAX, 1
  ret
timeGetDevCaps ENDP
timeBeginPeriod PROC
  cmp pfn_timeBeginPeriod, 0
  jz ERROR
  jmp pfn_timeBeginPeriod
ERROR:
  mov RAX, 1
  ret
timeBeginPeriod ENDP
timeEndPeriod PROC
  cmp pfn_timeEndPeriod, 0
  jz ERROR
  jmp pfn_timeEndPeriod
ERROR:
  mov RAX, 1
  ret
timeEndPeriod ENDP

; Wave Functions
waveOutGetNumDevs PROC
  cmp pfn_waveOutGetNumDevs, 0
  jz ERROR
  jmp pfn_waveOutGetNumDevs
ERROR:
  mov RAX, 0
  ret
waveOutGetNumDevs ENDP
waveOutGetDevCapsA PROC
  cmp pfn_waveOutGetDevCapsA, 0
  jz ERROR
  jmp pfn_waveOutGetDevCapsA
ERROR:
  mov RAX, 1
  ret
waveOutGetDevCapsA ENDP
waveOutGetDevCapsW PROC
  cmp pfn_waveOutGetDevCapsW, 0
  jz ERROR
  jmp pfn_waveOutGetDevCapsW
ERROR:
  mov RAX, 1
  ret
waveOutGetDevCapsW ENDP
waveOutGetVolume PROC
  cmp pfn_waveOutGetVolume, 0
  jz ERROR
  jmp pfn_waveOutGetVolume
ERROR:
  mov RAX, 1
  ret
waveOutGetVolume ENDP
waveOutSetVolume PROC
  cmp pfn_waveOutSetVolume, 0
  jz ERROR
  jmp pfn_waveOutSetVolume
ERROR:
  mov RAX, 1
  ret
waveOutSetVolume ENDP
waveOutGetErrorTextA PROC
  cmp pfn_waveOutGetErrorTextA, 0
  jz ERROR
  jmp pfn_waveOutGetErrorTextA
ERROR:
  mov RAX, 1
  ret
waveOutGetErrorTextA ENDP
waveOutGetErrorTextW PROC
  cmp pfn_waveOutGetErrorTextW, 0
  jz ERROR
  jmp pfn_waveOutGetErrorTextW
ERROR:
  mov RAX, 1
  ret
waveOutGetErrorTextW ENDP
waveOutOpen PROC
  cmp pfn_waveOutOpen, 0
  jz ERROR
  jmp pfn_waveOutOpen
ERROR:
  mov RAX, 1
  ret
waveOutOpen ENDP
waveOutClose PROC
  cmp pfn_waveOutClose, 0
  jz ERROR
  jmp pfn_waveOutClose
ERROR:
  mov RAX, 1
  ret
waveOutClose ENDP
waveOutPrepareHeader PROC
  cmp pfn_waveOutPrepareHeader, 0
  jz ERROR
  jmp pfn_waveOutPrepareHeader
ERROR:
  mov RAX, 1
  ret
waveOutPrepareHeader ENDP
waveOutUnprepareHeader PROC
  cmp pfn_waveOutUnprepareHeader, 0
  jz ERROR
  jmp pfn_waveOutUnprepareHeader
ERROR:
  mov RAX, 1
  ret
waveOutUnprepareHeader ENDP
waveOutWrite PROC
  cmp pfn_waveOutWrite, 0
  jz ERROR
  jmp pfn_waveOutWrite
ERROR:
  mov RAX, 1
  ret
waveOutWrite ENDP
waveOutPause PROC
  cmp pfn_waveOutPause, 0
  jz ERROR
  jmp pfn_waveOutPause
ERROR:
  mov RAX, 1
  ret
waveOutPause ENDP
waveOutRestart PROC
  cmp pfn_waveOutRestart, 0
  jz ERROR
  jmp pfn_waveOutRestart
ERROR:
  mov RAX, 1
  ret
waveOutRestart ENDP
waveOutReset PROC
  cmp pfn_waveOutReset, 0
  jz ERROR
  jmp pfn_waveOutReset
ERROR:
  mov RAX, 1
  ret
waveOutReset ENDP
waveOutBreakLoop PROC
  cmp pfn_waveOutBreakLoop, 0
  jz ERROR
  jmp pfn_waveOutBreakLoop
ERROR:
  mov RAX, 1
  ret
waveOutBreakLoop ENDP
waveOutGetPosition PROC
  cmp pfn_waveOutGetPosition, 0
  jz ERROR
  jmp pfn_waveOutGetPosition
ERROR:
  mov RAX, 1
  ret
waveOutGetPosition ENDP
waveOutGetPitch PROC
  cmp pfn_waveOutGetPitch, 0
  jz ERROR
  jmp pfn_waveOutGetPitch
ERROR:
  mov RAX, 1
  ret
waveOutGetPitch ENDP
waveOutSetPitch PROC
  cmp pfn_waveOutSetPitch, 0
  jz ERROR
  jmp pfn_waveOutSetPitch
ERROR:
  mov RAX, 1
  ret
waveOutSetPitch ENDP
waveOutGetPlaybackRate PROC
  cmp pfn_waveOutGetPlaybackRate, 0
  jz ERROR
  jmp pfn_waveOutGetPlaybackRate
ERROR:
  mov RAX, 1
  ret
waveOutGetPlaybackRate ENDP
waveOutSetPlaybackRate PROC
  cmp pfn_waveOutSetPlaybackRate, 0
  jz ERROR
  jmp pfn_waveOutSetPlaybackRate
ERROR:
  mov RAX, 1
  ret
waveOutSetPlaybackRate ENDP
waveOutGetID PROC
  cmp pfn_waveOutGetID, 0
  jz ERROR
  jmp pfn_waveOutGetID
ERROR:
  mov RAX, 1
  ret
waveOutGetID ENDP
waveOutMessage PROC
  cmp pfn_waveOutMessage, 0
  jz ERROR
  jmp pfn_waveOutMessage
ERROR:
  mov RAX, 1
  ret
waveOutMessage ENDP
waveInGetNumDevs PROC
  cmp pfn_waveInGetNumDevs, 0
  jz ERROR
  jmp pfn_waveInGetNumDevs
ERROR:
  mov RAX, 0
  ret
waveInGetNumDevs ENDP
waveInGetDevCapsA PROC
  cmp pfn_waveInGetDevCapsA, 0
  jz ERROR
  jmp pfn_waveInGetDevCapsA
ERROR:
  mov RAX, 1
  ret
waveInGetDevCapsA ENDP
waveInGetDevCapsW PROC
  cmp pfn_waveInGetDevCapsW, 0
  jz ERROR
  jmp pfn_waveInGetDevCapsW
ERROR:
  mov RAX, 1
  ret
waveInGetDevCapsW ENDP
waveInGetErrorTextA PROC
  cmp pfn_waveInGetErrorTextA, 0
  jz ERROR
  jmp pfn_waveInGetErrorTextA
ERROR:
  mov RAX, 1
  ret
waveInGetErrorTextA ENDP
waveInGetErrorTextW PROC
  cmp pfn_waveInGetErrorTextW, 0
  jz ERROR
  jmp pfn_waveInGetErrorTextW
ERROR:
  mov RAX, 1
  ret
waveInGetErrorTextW ENDP
waveInOpen PROC
  cmp pfn_waveInOpen, 0
  jz ERROR
  jmp pfn_waveInOpen
ERROR:
  mov RAX, 1
  ret
waveInOpen ENDP
waveInClose PROC
  cmp pfn_waveInClose, 0
  jz ERROR
  jmp pfn_waveInClose
ERROR:
  mov RAX, 1
  ret
waveInClose ENDP
waveInPrepareHeader PROC
  cmp pfn_waveInPrepareHeader, 0
  jz ERROR
  jmp pfn_waveInPrepareHeader
ERROR:
  mov RAX, 1
  ret
waveInPrepareHeader ENDP
waveInUnprepareHeader PROC
  cmp pfn_waveInUnprepareHeader, 0
  jz ERROR
  jmp pfn_waveInUnprepareHeader
ERROR:
  mov RAX, 1
  ret
waveInUnprepareHeader ENDP
waveInAddBuffer PROC
  cmp pfn_waveInAddBuffer, 0
  jz ERROR
  jmp pfn_waveInAddBuffer
ERROR:
  mov RAX, 1
  ret
waveInAddBuffer ENDP
waveInStart PROC
  cmp pfn_waveInStart, 0
  jz ERROR
  jmp pfn_waveInStart
ERROR:
  mov RAX, 1
  ret
waveInStart ENDP
waveInStop PROC
  cmp pfn_waveInStop, 0
  jz ERROR
  jmp pfn_waveInStop
ERROR:
  mov RAX, 1
  ret
waveInStop ENDP
waveInReset PROC
  cmp pfn_waveInReset, 0
  jz ERROR
  jmp pfn_waveInReset
ERROR:
  mov RAX, 1
  ret
waveInReset ENDP
waveInGetPosition PROC
  cmp pfn_waveInGetPosition, 0
  jz ERROR
  jmp pfn_waveInGetPosition
ERROR:
  mov RAX, 1
  ret
waveInGetPosition ENDP
waveInGetID PROC
  cmp pfn_waveInGetID, 0
  jz ERROR
  jmp pfn_waveInGetID
ERROR:
  mov RAX, 1
  ret
waveInGetID ENDP
waveInMessage PROC
  cmp pfn_waveInMessage, 0
  jz ERROR
  jmp pfn_waveInMessage
ERROR:
  mov RAX, 1
  ret
waveInMessage ENDP

; Message Functions
aux32Message PROC
  cmp pfn_aux32Message, 0
  jz ERROR
  jmp pfn_aux32Message
ERROR:
  mov RAX, 0
  ret
aux32Message ENDP
joy32Message PROC
  cmp pfn_joy32Message, 0
  jz ERROR
  jmp pfn_joy32Message
ERROR:
  mov RAX, 0
  ret
joy32Message ENDP
mci32Message PROC
  cmp pfn_mci32Message, 0
  jz ERROR
  jmp pfn_mci32Message
ERROR:
  mov RAX, 0
  ret
mci32Message ENDP
mid32Message PROC
  cmp pfn_mid32Message, 0
  jz ERROR
  jmp pfn_mid32Message
ERROR:
  mov RAX, 0
  ret
mid32Message ENDP
mod32Message PROC
  cmp pfn_mod32Message, 0
  jz ERROR
  jmp pfn_mod32Message
ERROR:
  mov RAX, 0
  ret
mod32Message ENDP
mxd32Message PROC
  cmp pfn_mxd32Message, 0
  jz ERROR
  jmp pfn_mxd32Message
ERROR:
  mov RAX, 0
  ret
mxd32Message ENDP
tid32Message PROC
  cmp pfn_tid32Message, 0
  jz ERROR
  jmp pfn_tid32Message
ERROR:
  mov RAX, 0
  ret
tid32Message ENDP
wid32Message PROC
  cmp pfn_wid32Message, 0
  jz ERROR
  jmp pfn_wid32Message
ERROR:
  mov RAX, 0
  ret
wid32Message ENDP
wod32Message PROC
  cmp pfn_wod32Message, 0
  jz ERROR
  jmp pfn_wod32Message
ERROR:
  mov RAX, 0
  ret
wod32Message ENDP
END
