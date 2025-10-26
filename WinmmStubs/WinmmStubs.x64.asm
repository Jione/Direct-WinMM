; asm code for WinMM.dll x64 binary
;
; Build Command: ml64.exe /Fo $(ProjectDir)$(IntDir)%(Filename).obj /c /Cp $(ProjectDir)%(Filename).asm
; Output: $(ProjectDir)$(IntDir)%(Filename).obj

.DATA
; PlaySound Functions
EXTERN _imp_NT4PlaySound:DQ
EXTERN _imp_PlaySound:DQ
EXTERN _imp_sndPlaySoundA:DQ
EXTERN _imp_sndPlaySoundW:DQ
EXTERN _imp_PlaySoundA:DQ
EXTERN _imp_PlaySoundW:DQ

; Driver and Drv help Functions
EXTERN _imp_CloseDriver:DQ
EXTERN _imp_OpenDriver:DQ
EXTERN _imp_SendDriverMessage:DQ
EXTERN _imp_DrvGetModuleHandle:DQ
EXTERN _imp_GetDriverModuleHandle:DQ
EXTERN _imp_DefDriverProc:DQ
EXTERN _imp_DrvClose:DQ
EXTERN _imp_DriverCallback:DQ
EXTERN _imp_DrvOpen:DQ
EXTERN _imp_DrvSendMessage:DQ
EXTERN _imp_mmsystemGetVersion:DQ
EXTERN _imp_NotifyCallbackData:DQ
EXTERN _imp_winmmDbgOut:DQ
EXTERN _imp_winmmSetDebugLevel:DQ
EXTERN _imp_MigrateAllDrivers:DQ
EXTERN _imp_MigrateSoundEvents:DQ
EXTERN _imp_WinmmLogon:DQ
EXTERN _imp_WinmmLogoff:DQ
EXTERN _imp_WOW32DriverCallback:DQ
EXTERN _imp_WOW32ResolveMultiMediaHandle:DQ
EXTERN _imp_WOWAppExit:DQ

; Audio GFX support Functions
EXTERN _imp_gfxCreateZoneFactoriesList:DQ
EXTERN _imp_gfxCreateGfxFactoriesList:DQ
EXTERN _imp_gfxDestroyDeviceInterfaceList:DQ
EXTERN _imp_gfxEnumerateGfxs:DQ
EXTERN _imp_gfxRemoveGfx:DQ
EXTERN _imp_gfxAddGfx:DQ
EXTERN _imp_gfxModifyGfx:DQ
EXTERN _imp_gfxOpenGfx:DQ
EXTERN _imp_gfxBatchChange:DQ

; AUX Functions
EXTERN _imp_auxGetDevCapsA:DQ
EXTERN _imp_auxGetDevCapsW:DQ
EXTERN _imp_auxGetNumDevs:DQ
EXTERN _imp_auxGetVolume:DQ
EXTERN _imp_auxOutMessage:DQ
EXTERN _imp_auxSetVolume:DQ

; Joystick Functions
EXTERN _imp_joyGetNumDevs:DQ
EXTERN _imp_joyGetDevCapsA:DQ
EXTERN _imp_joyGetDevCapsW:DQ
EXTERN _imp_joyGetPos:DQ
EXTERN _imp_joyGetPosEx:DQ
EXTERN _imp_joyGetThreshold:DQ
EXTERN _imp_joyReleaseCapture:DQ
EXTERN _imp_joySetCapture:DQ
EXTERN _imp_joySetThreshold:DQ
EXTERN _imp_joySetCalibration:DQ
EXTERN _imp_joyConfigChanged:DQ

; MCI Functions
EXTERN _imp_mciExecute:DQ
EXTERN _imp_mciSendCommandA:DQ
EXTERN _imp_mciSendCommandW:DQ
EXTERN _imp_mciSendStringA:DQ
EXTERN _imp_mciSendStringW:DQ
EXTERN _imp_mciGetDeviceIDA:DQ
EXTERN _imp_mciGetDeviceIDW:DQ
EXTERN _imp_mciGetDeviceIDFromElementIDA:DQ
EXTERN _imp_mciGetDeviceIDFromElementIDW:DQ
EXTERN _imp_mciGetErrorStringA:DQ
EXTERN _imp_mciGetErrorStringW:DQ
EXTERN _imp_mciSetYieldProc:DQ
EXTERN _imp_mciGetCreatorTask:DQ
EXTERN _imp_mciGetYieldProc:DQ
EXTERN _imp_mciDriverNotify:DQ
EXTERN _imp_mciDriverYield:DQ
EXTERN _imp_mciFreeCommandResource:DQ
EXTERN _imp_mciGetDriverData:DQ
EXTERN _imp_mciLoadCommandResource:DQ
EXTERN _imp_mciSetDriverData:DQ
EXTERN _imp_mciEatCommandEntry:DQ
EXTERN _imp_mciGetParamSize:DQ
EXTERN _imp_mciUnlockCommandTable:DQ
EXTERN _imp_FindCommandItem:DQ

; MIDI Functions
EXTERN _imp_midiOutGetNumDevs:DQ
EXTERN _imp_midiStreamOpen:DQ
EXTERN _imp_midiStreamClose:DQ
EXTERN _imp_midiStreamProperty:DQ
EXTERN _imp_midiStreamPosition:DQ
EXTERN _imp_midiStreamOut:DQ
EXTERN _imp_midiStreamPause:DQ
EXTERN _imp_midiStreamRestart:DQ
EXTERN _imp_midiStreamStop:DQ
EXTERN _imp_midiConnect:DQ
EXTERN _imp_midiDisconnect:DQ
EXTERN _imp_midiOutGetDevCapsA:DQ
EXTERN _imp_midiOutGetDevCapsW:DQ
EXTERN _imp_midiOutGetVolume:DQ
EXTERN _imp_midiOutSetVolume:DQ
EXTERN _imp_midiOutGetErrorTextA:DQ
EXTERN _imp_midiOutGetErrorTextW:DQ
EXTERN _imp_midiOutOpen:DQ
EXTERN _imp_midiOutClose:DQ
EXTERN _imp_midiOutPrepareHeader:DQ
EXTERN _imp_midiOutUnprepareHeader:DQ
EXTERN _imp_midiOutShortMsg:DQ
EXTERN _imp_midiOutLongMsg:DQ
EXTERN _imp_midiOutReset:DQ
EXTERN _imp_midiOutCachePatches:DQ
EXTERN _imp_midiOutCacheDrumPatches:DQ
EXTERN _imp_midiOutGetID:DQ
EXTERN _imp_midiOutMessage:DQ
EXTERN _imp_midiInGetNumDevs:DQ
EXTERN _imp_midiInGetDevCapsA:DQ
EXTERN _imp_midiInGetDevCapsW:DQ
EXTERN _imp_midiInGetErrorTextA:DQ
EXTERN _imp_midiInGetErrorTextW:DQ
EXTERN _imp_midiInOpen:DQ
EXTERN _imp_midiInClose:DQ
EXTERN _imp_midiInPrepareHeader:DQ
EXTERN _imp_midiInUnprepareHeader:DQ
EXTERN _imp_midiInAddBuffer:DQ
EXTERN _imp_midiInStart:DQ
EXTERN _imp_midiInStop:DQ
EXTERN _imp_midiInReset:DQ
EXTERN _imp_midiInGetID:DQ
EXTERN _imp_midiInMessage:DQ

; Mixer Functions
EXTERN _imp_mixerGetNumDevs:DQ
EXTERN _imp_mixerGetDevCapsA:DQ
EXTERN _imp_mixerGetDevCapsW:DQ
EXTERN _imp_mixerOpen:DQ
EXTERN _imp_mixerClose:DQ
EXTERN _imp_mixerMessage:DQ
EXTERN _imp_mixerGetLineInfoA:DQ
EXTERN _imp_mixerGetLineInfoW:DQ
EXTERN _imp_mixerGetID:DQ
EXTERN _imp_mixerGetLineControlsA:DQ
EXTERN _imp_mixerGetLineControlsW:DQ
EXTERN _imp_mixerGetControlDetailsA:DQ
EXTERN _imp_mixerGetControlDetailsW:DQ
EXTERN _imp_mixerSetControlDetails:DQ

; Task Functions
EXTERN _imp_mmDrvInstall:DQ
EXTERN _imp_mmGetCurrentTask:DQ
EXTERN _imp_mmTaskBlock:DQ
EXTERN _imp_mmTaskCreate:DQ
EXTERN _imp_mmTaskSignal:DQ
EXTERN _imp_mmTaskYield:DQ

; Multimedia io Functions
EXTERN _imp_mmioStringToFOURCCA:DQ
EXTERN _imp_mmioStringToFOURCCW:DQ
EXTERN _imp_mmioInstallIOProcA:DQ
EXTERN _imp_mmioInstallIOProcW:DQ
EXTERN _imp_mmioOpenA:DQ
EXTERN _imp_mmioOpenW:DQ
EXTERN _imp_mmioRenameA:DQ
EXTERN _imp_mmioRenameW:DQ
EXTERN _imp_mmioClose:DQ
EXTERN _imp_mmioRead:DQ
EXTERN _imp_mmioWrite:DQ
EXTERN _imp_mmioSeek:DQ
EXTERN _imp_mmioGetInfo:DQ
EXTERN _imp_mmioSetInfo:DQ
EXTERN _imp_mmioSetBuffer:DQ
EXTERN _imp_mmioFlush:DQ
EXTERN _imp_mmioAdvance:DQ
EXTERN _imp_mmioSendMessage:DQ
EXTERN _imp_mmioDescend:DQ
EXTERN _imp_mmioAscend:DQ
EXTERN _imp_mmioCreateChunk:DQ

; Time Functions
EXTERN _imp_timeGetSystemTime:DQ
EXTERN _imp_timeGetTime:DQ
EXTERN _imp_timeSetEvent:DQ
EXTERN _imp_timeKillEvent:DQ
EXTERN _imp_timeGetDevCaps:DQ
EXTERN _imp_timeBeginPeriod:DQ
EXTERN _imp_timeEndPeriod:DQ

; Wave Functions
EXTERN _imp_waveOutGetNumDevs:DQ
EXTERN _imp_waveOutGetDevCapsA:DQ
EXTERN _imp_waveOutGetDevCapsW:DQ
EXTERN _imp_waveOutGetVolume:DQ
EXTERN _imp_waveOutSetVolume:DQ
EXTERN _imp_waveOutGetErrorTextA:DQ
EXTERN _imp_waveOutGetErrorTextW:DQ
EXTERN _imp_waveOutOpen:DQ
EXTERN _imp_waveOutClose:DQ
EXTERN _imp_waveOutPrepareHeader:DQ
EXTERN _imp_waveOutUnprepareHeader:DQ
EXTERN _imp_waveOutWrite:DQ
EXTERN _imp_waveOutPause:DQ
EXTERN _imp_waveOutRestart:DQ
EXTERN _imp_waveOutReset:DQ
EXTERN _imp_waveOutBreakLoop:DQ
EXTERN _imp_waveOutGetPosition:DQ
EXTERN _imp_waveOutGetPitch:DQ
EXTERN _imp_waveOutSetPitch:DQ
EXTERN _imp_waveOutGetPlaybackRate:DQ
EXTERN _imp_waveOutSetPlaybackRate:DQ
EXTERN _imp_waveOutGetID:DQ
EXTERN _imp_waveOutMessage:DQ
EXTERN _imp_waveInGetNumDevs:DQ
EXTERN _imp_waveInGetDevCapsA:DQ
EXTERN _imp_waveInGetDevCapsW:DQ
EXTERN _imp_waveInGetErrorTextA:DQ
EXTERN _imp_waveInGetErrorTextW:DQ
EXTERN _imp_waveInOpen:DQ
EXTERN _imp_waveInClose:DQ
EXTERN _imp_waveInPrepareHeader:DQ
EXTERN _imp_waveInUnprepareHeader:DQ
EXTERN _imp_waveInAddBuffer:DQ
EXTERN _imp_waveInStart:DQ
EXTERN _imp_waveInStop:DQ
EXTERN _imp_waveInReset:DQ
EXTERN _imp_waveInGetPosition:DQ
EXTERN _imp_waveInGetID:DQ
EXTERN _imp_waveInMessage:DQ

; Message Functions
EXTERN _imp_aux32Message:DQ
EXTERN _imp_joy32Message:DQ
EXTERN _imp_mci32Message:DQ
EXTERN _imp_mid32Message:DQ
EXTERN _imp_mod32Message:DQ
EXTERN _imp_mxd32Message:DQ
EXTERN _imp_tid32Message:DQ
EXTERN _imp_wid32Message:DQ
EXTERN _imp_wod32Message:DQ


.CODE
; PlaySound Functions
NT4PlaySound PROC
  cmp _imp_NT4PlaySound, 0
  jz ERROR
  jmp _imp_NT4PlaySound
ERROR:
  xor RAX, RAX
  ret
NT4PlaySound ENDP
PlaySound PROC
  cmp _imp_PlaySound, 0
  jz ERROR
  jmp _imp_PlaySound
ERROR:
  xor RAX, RAX
  ret
PlaySound ENDP
sndPlaySoundA PROC
  cmp _imp_sndPlaySoundA, 0
  jz ERROR
  jmp _imp_sndPlaySoundA
ERROR:
  xor RAX, RAX
  ret
sndPlaySoundA ENDP
sndPlaySoundW PROC
  cmp _imp_sndPlaySoundW, 0
  jz ERROR
  jmp _imp_sndPlaySoundW
ERROR:
  xor RAX, RAX
  ret
sndPlaySoundW ENDP
PlaySoundA PROC
  cmp _imp_PlaySoundA, 0
  jz ERROR
  jmp _imp_PlaySoundA
ERROR:
  xor RAX, RAX
  ret
PlaySoundA ENDP
PlaySoundW PROC
  cmp _imp_PlaySoundW, 0
  jz ERROR
  jmp _imp_PlaySoundW
ERROR:
  xor RAX, RAX
  ret
PlaySoundW ENDP

; Driver and Drv help Functions
CloseDriver PROC
  cmp _imp_CloseDriver, 0
  jz ERROR
  jmp _imp_CloseDriver
ERROR:
  mov RAX, -1
  ret
CloseDriver ENDP
OpenDriver PROC
  cmp _imp_OpenDriver, 0
  jz ERROR
  jmp _imp_OpenDriver
ERROR:
  xor RAX, RAX
  ret
OpenDriver ENDP
SendDriverMessage PROC
  cmp _imp_SendDriverMessage, 0
  jz ERROR
  jmp _imp_SendDriverMessage
ERROR:
  mov RAX, -1
  ret
SendDriverMessage ENDP
DrvGetModuleHandle PROC
  cmp _imp_DrvGetModuleHandle, 0
  jz ERROR
  jmp _imp_DrvGetModuleHandle
ERROR:
  xor RAX, RAX
  ret
DrvGetModuleHandle ENDP
GetDriverModuleHandle PROC
  cmp _imp_GetDriverModuleHandle, 0
  jz ERROR
  jmp _imp_GetDriverModuleHandle
ERROR:
  xor RAX, RAX
  ret
GetDriverModuleHandle ENDP
DefDriverProc PROC
  cmp _imp_DefDriverProc, 0
  jz ERROR
  jmp _imp_DefDriverProc
ERROR:
  mov RAX, -1
  ret
DefDriverProc ENDP
DrvClose PROC
  cmp _imp_DrvClose, 0
  jz ERROR
  jmp _imp_DrvClose
ERROR:
  mov RAX, -1
  ret
DrvClose ENDP
DriverCallback PROC
  cmp _imp_DriverCallback, 0
  jz ERROR
  jmp _imp_DriverCallback
ERROR:
  xor RAX, RAX
  ret
DriverCallback ENDP
DrvOpen PROC
  cmp _imp_DrvOpen, 0
  jz ERROR
  jmp _imp_DrvOpen
ERROR:
  xor RAX, RAX
  ret
DrvOpen ENDP
DrvSendMessage PROC
  cmp _imp_DrvSendMessage, 0
  jz ERROR
  jmp _imp_DrvSendMessage
ERROR:
  mov RAX, -1
  ret
DrvSendMessage ENDP
mmsystemGetVersion PROC
  cmp _imp_mmsystemGetVersion, 0
  jz ERROR
  jmp _imp_mmsystemGetVersion
ERROR:
  xor RAX, RAX
  ret
mmsystemGetVersion ENDP
NotifyCallbackData PROC
  cmp _imp_NotifyCallbackData, 0
  jz ERROR
  jmp _imp_NotifyCallbackData
ERROR:
  xor RAX, RAX
  ret
NotifyCallbackData ENDP
winmmDbgOut PROC
  cmp _imp_winmmDbgOut, 0
  jz ERROR
  jmp _imp_winmmDbgOut
ERROR:
  ret
winmmDbgOut ENDP
winmmSetDebugLevel PROC
  cmp _imp_winmmSetDebugLevel, 0
  jz ERROR
  jmp _imp_winmmSetDebugLevel
ERROR:
  ret
winmmSetDebugLevel ENDP
MigrateAllDrivers PROC
  cmp _imp_MigrateAllDrivers, 0
  jz ERROR
  jmp _imp_MigrateAllDrivers
ERROR:
  ret
MigrateAllDrivers ENDP
MigrateSoundEvents PROC
  cmp _imp_MigrateSoundEvents, 0
  jz ERROR
  jmp _imp_MigrateSoundEvents
ERROR:
  ret
MigrateSoundEvents ENDP
WinmmLogon PROC
  cmp _imp_WinmmLogon, 0
  jz ERROR
  jmp _imp_WinmmLogon
ERROR:
  ret
WinmmLogon ENDP
WinmmLogoff PROC
  cmp _imp_WinmmLogoff, 0
  jz ERROR
  jmp _imp_WinmmLogoff
ERROR:
  ret
WinmmLogoff ENDP
WOW32DriverCallback PROC
  cmp _imp_WOW32DriverCallback, 0
  jz ERROR
  jmp _imp_WOW32DriverCallback
ERROR:
  xor RAX, RAX
  ret
WOW32DriverCallback ENDP
WOW32ResolveMultiMediaHandle PROC
  cmp _imp_WOW32ResolveMultiMediaHandle, 0
  jz ERROR
  jmp _imp_WOW32ResolveMultiMediaHandle
ERROR:
  xor RAX, RAX
  ret
WOW32ResolveMultiMediaHandle ENDP
WOWAppExit PROC
  cmp _imp_WOWAppExit, 0
  jz ERROR
  jmp _imp_WOWAppExit
ERROR:
  ret
WOWAppExit ENDP

; Audio GFX support Functions
gfxCreateZoneFactoriesList PROC
  cmp _imp_gfxCreateZoneFactoriesList, 0
  jz ERROR
  jmp _imp_gfxCreateZoneFactoriesList
ERROR:
  xor RAX, RAX
  ret
gfxCreateZoneFactoriesList ENDP
gfxCreateGfxFactoriesList PROC
  cmp _imp_gfxCreateGfxFactoriesList, 0
  jz ERROR
  jmp _imp_gfxCreateGfxFactoriesList
ERROR:
  xor RAX, RAX
  ret
gfxCreateGfxFactoriesList ENDP
gfxDestroyDeviceInterfaceList PROC
  cmp _imp_gfxDestroyDeviceInterfaceList, 0
  jz ERROR
  jmp _imp_gfxDestroyDeviceInterfaceList
ERROR:
  xor RAX, RAX
  ret
gfxDestroyDeviceInterfaceList ENDP
gfxEnumerateGfxs PROC
  cmp _imp_gfxEnumerateGfxs, 0
  jz ERROR
  jmp _imp_gfxEnumerateGfxs
ERROR:
  xor RAX, RAX
  ret
gfxEnumerateGfxs ENDP
gfxRemoveGfx PROC
  cmp _imp_gfxRemoveGfx, 0
  jz ERROR
  jmp _imp_gfxRemoveGfx
ERROR:
  xor RAX, RAX
  ret
gfxRemoveGfx ENDP
gfxAddGfx PROC
  cmp _imp_gfxAddGfx, 0
  jz ERROR
  jmp _imp_gfxAddGfx
ERROR:
  xor RAX, RAX
  ret
gfxAddGfx ENDP
gfxModifyGfx PROC
  cmp _imp_gfxModifyGfx, 0
  jz ERROR
  jmp _imp_gfxModifyGfx
ERROR:
  xor RAX, RAX
  ret
gfxModifyGfx ENDP
gfxOpenGfx PROC
  cmp _imp_gfxOpenGfx, 0
  jz ERROR
  jmp _imp_gfxOpenGfx
ERROR:
  xor RAX, RAX
  ret
gfxOpenGfx ENDP
gfxBatchChange PROC
  cmp _imp_gfxBatchChange, 0
  jz ERROR
  jmp _imp_gfxBatchChange
ERROR:
  xor RAX, RAX
  ret
gfxBatchChange ENDP

; AUX Functions
auxGetDevCapsA PROC
  cmp _imp_auxGetDevCapsA, 0
  jz ERROR
  jmp _imp_auxGetDevCapsA
ERROR:
  mov RAX, 1
  ret
auxGetDevCapsA ENDP
auxGetDevCapsW PROC
  cmp _imp_auxGetDevCapsW, 0
  jz ERROR
  jmp _imp_auxGetDevCapsW
ERROR:
  mov RAX, 1
  ret
auxGetDevCapsW ENDP
auxGetNumDevs PROC
  cmp _imp_auxGetNumDevs, 0
  jz ERROR
  jmp _imp_auxGetNumDevs
ERROR:
  xor RAX, RAX
  ret
auxGetNumDevs ENDP
auxGetVolume PROC
  cmp _imp_auxGetVolume, 0
  jz ERROR
  jmp _imp_auxGetVolume
ERROR:
  mov RAX, 1
  ret
auxGetVolume ENDP
auxOutMessage PROC
  cmp _imp_auxOutMessage, 0
  jz ERROR
  jmp _imp_auxOutMessage
ERROR:
  mov RAX, 1
  ret
auxOutMessage ENDP
auxSetVolume PROC
  cmp _imp_auxSetVolume, 0
  jz ERROR
  jmp _imp_auxSetVolume
ERROR:
  mov RAX, 1
  ret
auxSetVolume ENDP

; Joystick Functions
joyGetNumDevs PROC
  cmp _imp_joyGetNumDevs, 0
  jz ERROR
  jmp _imp_joyGetNumDevs
ERROR:
  xor RAX, RAX
  ret
joyGetNumDevs ENDP
joyGetDevCapsA PROC
  cmp _imp_joyGetDevCapsA, 0
  jz ERROR
  jmp _imp_joyGetDevCapsA
ERROR:
  mov RAX, 1
  ret
joyGetDevCapsA ENDP
joyGetDevCapsW PROC
  cmp _imp_joyGetDevCapsW, 0
  jz ERROR
  jmp _imp_joyGetDevCapsW
ERROR:
  mov RAX, 1
  ret
joyGetDevCapsW ENDP
joyGetPos PROC
  cmp _imp_joyGetPos, 0
  jz ERROR
  jmp _imp_joyGetPos
ERROR:
  mov RAX, 1
  ret
joyGetPos ENDP
joyGetPosEx PROC
  cmp _imp_joyGetPosEx, 0
  jz ERROR
  jmp _imp_joyGetPosEx
ERROR:
  mov RAX, 1
  ret
joyGetPosEx ENDP
joyGetThreshold PROC
  cmp _imp_joyGetThreshold, 0
  jz ERROR
  jmp _imp_joyGetThreshold
ERROR:
  mov RAX, 1
  ret
joyGetThreshold ENDP
joyReleaseCapture PROC
  cmp _imp_joyReleaseCapture, 0
  jz ERROR
  jmp _imp_joyReleaseCapture
ERROR:
  mov RAX, 1
  ret
joyReleaseCapture ENDP
joySetCapture PROC
  cmp _imp_joySetCapture, 0
  jz ERROR
  jmp _imp_joySetCapture
ERROR:
  mov RAX, 1
  ret
joySetCapture ENDP
joySetThreshold PROC
  cmp _imp_joySetThreshold, 0
  jz ERROR
  jmp _imp_joySetThreshold
ERROR:
  mov RAX, 1
  ret
joySetThreshold ENDP
joySetCalibration PROC
  cmp _imp_joySetCalibration, 0
  jz ERROR
  jmp _imp_joySetCalibration
ERROR:
  mov RAX, -1
  ret
joySetCalibration ENDP
joyConfigChanged PROC
  cmp _imp_joyConfigChanged, 0
  jz ERROR
  jmp _imp_joyConfigChanged
ERROR:
  mov RAX, 1
  ret
joyConfigChanged ENDP

; MCI Functions
mciExecute PROC
  cmp _imp_mciExecute, 0
  jz ERROR
  jmp _imp_mciExecute
ERROR:
  xor RAX, RAX
  ret
mciExecute ENDP
mciSendCommandA PROC
  cmp _imp_mciSendCommandA, 0
  jz ERROR
  jmp _imp_mciSendCommandA
ERROR:
  mov RAX, 1
  ret
mciSendCommandA ENDP
mciSendCommandW PROC
  cmp _imp_mciSendCommandW, 0
  jz ERROR
  jmp _imp_mciSendCommandW
ERROR:
  mov RAX, 1
  ret
mciSendCommandW ENDP
mciSendStringA PROC
  cmp _imp_mciSendStringA, 0
  jz ERROR
  jmp _imp_mciSendStringA
ERROR:
  mov RAX, 1
  ret
mciSendStringA ENDP
mciSendStringW PROC
  cmp _imp_mciSendStringW, 0
  jz ERROR
  jmp _imp_mciSendStringW
ERROR:
  mov RAX, 1
  ret
mciSendStringW ENDP
mciGetDeviceIDA PROC
  cmp _imp_mciGetDeviceIDA, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDA
ERROR:
  xor RAX, RAX
  ret
mciGetDeviceIDA ENDP
mciGetDeviceIDW PROC
  cmp _imp_mciGetDeviceIDW, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDW
ERROR:
  xor RAX, RAX
  ret
mciGetDeviceIDW ENDP
mciGetDeviceIDFromElementIDA PROC
  cmp _imp_mciGetDeviceIDFromElementIDA, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDFromElementIDA
ERROR:
  xor RAX, RAX
  ret
mciGetDeviceIDFromElementIDA ENDP
mciGetDeviceIDFromElementIDW PROC
  cmp _imp_mciGetDeviceIDFromElementIDW, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDFromElementIDW
ERROR:
  xor RAX, RAX
  ret
mciGetDeviceIDFromElementIDW ENDP
mciGetErrorStringA PROC
  cmp _imp_mciGetErrorStringA, 0
  jz ERROR
  jmp _imp_mciGetErrorStringA
ERROR:
  xor RAX, RAX
  ret
mciGetErrorStringA ENDP
mciGetErrorStringW PROC
  cmp _imp_mciGetErrorStringW, 0
  jz ERROR
  jmp _imp_mciGetErrorStringW
ERROR:
  xor RAX, RAX
  ret
mciGetErrorStringW ENDP
mciSetYieldProc PROC
  cmp _imp_mciSetYieldProc, 0
  jz ERROR
  jmp _imp_mciSetYieldProc
ERROR:
  xor RAX, RAX
  ret
mciSetYieldProc ENDP
mciGetCreatorTask PROC
  cmp _imp_mciGetCreatorTask, 0
  jz ERROR
  jmp _imp_mciGetCreatorTask
ERROR:
  xor RAX, RAX
  ret
mciGetCreatorTask ENDP
mciGetYieldProc PROC
  cmp _imp_mciGetYieldProc, 0
  jz ERROR
  jmp _imp_mciGetYieldProc
ERROR:
  xor RAX, RAX
  ret
mciGetYieldProc ENDP
mciDriverNotify PROC
  cmp _imp_mciDriverNotify, 0
  jz ERROR
  jmp _imp_mciDriverNotify
ERROR:
  xor RAX, RAX
  ret
mciDriverNotify ENDP
mciDriverYield PROC
  cmp _imp_mciDriverYield, 0
  jz ERROR
  jmp _imp_mciDriverYield
ERROR:
  xor RAX, RAX
  ret
mciDriverYield ENDP
mciFreeCommandResource PROC
  cmp _imp_mciFreeCommandResource, 0
  jz ERROR
  jmp _imp_mciFreeCommandResource
ERROR:
  xor RAX, RAX
  ret
mciFreeCommandResource ENDP
mciGetDriverData PROC
  cmp _imp_mciGetDriverData, 0
  jz ERROR
  jmp _imp_mciGetDriverData
ERROR:
  xor RAX, RAX
  ret
mciGetDriverData ENDP
mciLoadCommandResource PROC
  cmp _imp_mciLoadCommandResource, 0
  jz ERROR
  jmp _imp_mciLoadCommandResource
ERROR:
  xor RAX, RAX
  ret
mciLoadCommandResource ENDP
mciSetDriverData PROC
  cmp _imp_mciSetDriverData, 0
  jz ERROR
  jmp _imp_mciSetDriverData
ERROR:
  xor RAX, RAX
  ret
mciSetDriverData ENDP
mciEatCommandEntry PROC
  cmp _imp_mciEatCommandEntry, 0
  jz ERROR
  jmp _imp_mciEatCommandEntry
ERROR:
  xor RAX, RAX
  ret
mciEatCommandEntry ENDP
mciGetParamSize PROC
  cmp _imp_mciGetParamSize, 0
  jz ERROR
  jmp _imp_mciGetParamSize
ERROR:
  xor RAX, RAX
  ret
mciGetParamSize ENDP
mciUnlockCommandTable PROC
  cmp _imp_mciUnlockCommandTable, 0
  jz ERROR
  jmp _imp_mciUnlockCommandTable
ERROR:
  xor RAX, RAX
  ret
mciUnlockCommandTable ENDP
FindCommandItem PROC
  cmp _imp_FindCommandItem, 0
  jz ERROR
  jmp _imp_FindCommandItem
ERROR:
  xor RAX, RAX
  ret
FindCommandItem ENDP

; MIDI Functions
midiOutGetNumDevs PROC
  cmp _imp_midiOutGetNumDevs, 0
  jz ERROR
  jmp _imp_midiOutGetNumDevs
ERROR:
  xor RAX, RAX
  ret
midiOutGetNumDevs ENDP
midiStreamOpen PROC
  cmp _imp_midiStreamOpen, 0
  jz ERROR
  jmp _imp_midiStreamOpen
ERROR:
  mov RAX, 1
  ret
midiStreamOpen ENDP
midiStreamClose PROC
  cmp _imp_midiStreamClose, 0
  jz ERROR
  jmp _imp_midiStreamClose
ERROR:
  mov RAX, 1
  ret
midiStreamClose ENDP
midiStreamProperty PROC
  cmp _imp_midiStreamProperty, 0
  jz ERROR
  jmp _imp_midiStreamProperty
ERROR:
  mov RAX, 1
  ret
midiStreamProperty ENDP
midiStreamPosition PROC
  cmp _imp_midiStreamPosition, 0
  jz ERROR
  jmp _imp_midiStreamPosition
ERROR:
  mov RAX, 1
  ret
midiStreamPosition ENDP
midiStreamOut PROC
  cmp _imp_midiStreamOut, 0
  jz ERROR
  jmp _imp_midiStreamOut
ERROR:
  mov RAX, 1
  ret
midiStreamOut ENDP
midiStreamPause PROC
  cmp _imp_midiStreamPause, 0
  jz ERROR
  jmp _imp_midiStreamPause
ERROR:
  mov RAX, 1
  ret
midiStreamPause ENDP
midiStreamRestart PROC
  cmp _imp_midiStreamRestart, 0
  jz ERROR
  jmp _imp_midiStreamRestart
ERROR:
  mov RAX, 1
  ret
midiStreamRestart ENDP
midiStreamStop PROC
  cmp _imp_midiStreamStop, 0
  jz ERROR
  jmp _imp_midiStreamStop
ERROR:
  mov RAX, 1
  ret
midiStreamStop ENDP
midiConnect PROC
  cmp _imp_midiConnect, 0
  jz ERROR
  jmp _imp_midiConnect
ERROR:
  mov RAX, 1
  ret
midiConnect ENDP
midiDisconnect PROC
  cmp _imp_midiDisconnect, 0
  jz ERROR
  jmp _imp_midiDisconnect
ERROR:
  mov RAX, 1
  ret
midiDisconnect ENDP
midiOutGetDevCapsA PROC
  cmp _imp_midiOutGetDevCapsA, 0
  jz ERROR
  jmp _imp_midiOutGetDevCapsA
ERROR:
  mov RAX, 1
  ret
midiOutGetDevCapsA ENDP
midiOutGetDevCapsW PROC
  cmp _imp_midiOutGetDevCapsW, 0
  jz ERROR
  jmp _imp_midiOutGetDevCapsW
ERROR:
  mov RAX, 1
  ret
midiOutGetDevCapsW ENDP
midiOutGetVolume PROC
  cmp _imp_midiOutGetVolume, 0
  jz ERROR
  jmp _imp_midiOutGetVolume
ERROR:
  mov RAX, 1
  ret
midiOutGetVolume ENDP
midiOutSetVolume PROC
  cmp _imp_midiOutSetVolume, 0
  jz ERROR
  jmp _imp_midiOutSetVolume
ERROR:
  mov RAX, 1
  ret
midiOutSetVolume ENDP
midiOutGetErrorTextA PROC
  cmp _imp_midiOutGetErrorTextA, 0
  jz ERROR
  jmp _imp_midiOutGetErrorTextA
ERROR:
  mov RAX, 1
  ret
midiOutGetErrorTextA ENDP
midiOutGetErrorTextW PROC
  cmp _imp_midiOutGetErrorTextW, 0
  jz ERROR
  jmp _imp_midiOutGetErrorTextW
ERROR:
  mov RAX, 1
  ret
midiOutGetErrorTextW ENDP
midiOutOpen PROC
  cmp _imp_midiOutOpen, 0
  jz ERROR
  jmp _imp_midiOutOpen
ERROR:
  mov RAX, 1
  ret
midiOutOpen ENDP
midiOutClose PROC
  cmp _imp_midiOutClose, 0
  jz ERROR
  jmp _imp_midiOutClose
ERROR:
  mov RAX, 1
  ret
midiOutClose ENDP
midiOutPrepareHeader PROC
  cmp _imp_midiOutPrepareHeader, 0
  jz ERROR
  jmp _imp_midiOutPrepareHeader
ERROR:
  mov RAX, 1
  ret
midiOutPrepareHeader ENDP
midiOutUnprepareHeader PROC
  cmp _imp_midiOutUnprepareHeader, 0
  jz ERROR
  jmp _imp_midiOutUnprepareHeader
ERROR:
  mov RAX, 1
  ret
midiOutUnprepareHeader ENDP
midiOutShortMsg PROC
  cmp _imp_midiOutShortMsg, 0
  jz ERROR
  jmp _imp_midiOutShortMsg
ERROR:
  mov RAX, 1
  ret
midiOutShortMsg ENDP
midiOutLongMsg PROC
  cmp _imp_midiOutLongMsg, 0
  jz ERROR
  jmp _imp_midiOutLongMsg
ERROR:
  mov RAX, 1
  ret
midiOutLongMsg ENDP
midiOutReset PROC
  cmp _imp_midiOutReset, 0
  jz ERROR
  jmp _imp_midiOutReset
ERROR:
  mov RAX, 1
  ret
midiOutReset ENDP
midiOutCachePatches PROC
  cmp _imp_midiOutCachePatches, 0
  jz ERROR
  jmp _imp_midiOutCachePatches
ERROR:
  mov RAX, 1
  ret
midiOutCachePatches ENDP
midiOutCacheDrumPatches PROC
  cmp _imp_midiOutCacheDrumPatches, 0
  jz ERROR
  jmp _imp_midiOutCacheDrumPatches
ERROR:
  mov RAX, 1
  ret
midiOutCacheDrumPatches ENDP
midiOutGetID PROC
  cmp _imp_midiOutGetID, 0
  jz ERROR
  jmp _imp_midiOutGetID
ERROR:
  mov RAX, 1
  ret
midiOutGetID ENDP
midiOutMessage PROC
  cmp _imp_midiOutMessage, 0
  jz ERROR
  jmp _imp_midiOutMessage
ERROR:
  mov RAX, 1
  ret
midiOutMessage ENDP
midiInGetNumDevs PROC
  cmp _imp_midiInGetNumDevs, 0
  jz ERROR
  jmp _imp_midiInGetNumDevs
ERROR:
  xor RAX, RAX
  ret
midiInGetNumDevs ENDP
midiInGetDevCapsA PROC
  cmp _imp_midiInGetDevCapsA, 0
  jz ERROR
  jmp _imp_midiInGetDevCapsA
ERROR:
  mov RAX, 1
  ret
midiInGetDevCapsA ENDP
midiInGetDevCapsW PROC
  cmp _imp_midiInGetDevCapsW, 0
  jz ERROR
  jmp _imp_midiInGetDevCapsW
ERROR:
  mov RAX, 1
  ret
midiInGetDevCapsW ENDP
midiInGetErrorTextA PROC
  cmp _imp_midiInGetErrorTextA, 0
  jz ERROR
  jmp _imp_midiInGetErrorTextA
ERROR:
  mov RAX, 1
  ret
midiInGetErrorTextA ENDP
midiInGetErrorTextW PROC
  cmp _imp_midiInGetErrorTextW, 0
  jz ERROR
  jmp _imp_midiInGetErrorTextW
ERROR:
  mov RAX, 1
  ret
midiInGetErrorTextW ENDP
midiInOpen PROC
  cmp _imp_midiInOpen, 0
  jz ERROR
  jmp _imp_midiInOpen
ERROR:
  mov RAX, 1
  ret
midiInOpen ENDP
midiInClose PROC
  cmp _imp_midiInClose, 0
  jz ERROR
  jmp _imp_midiInClose
ERROR:
  mov RAX, 1
  ret
midiInClose ENDP
midiInPrepareHeader PROC
  cmp _imp_midiInPrepareHeader, 0
  jz ERROR
  jmp _imp_midiInPrepareHeader
ERROR:
  mov RAX, 1
  ret
midiInPrepareHeader ENDP
midiInUnprepareHeader PROC
  cmp _imp_midiInUnprepareHeader, 0
  jz ERROR
  jmp _imp_midiInUnprepareHeader
ERROR:
  mov RAX, 1
  ret
midiInUnprepareHeader ENDP
midiInAddBuffer PROC
  cmp _imp_midiInAddBuffer, 0
  jz ERROR
  jmp _imp_midiInAddBuffer
ERROR:
  mov RAX, 1
  ret
midiInAddBuffer ENDP
midiInStart PROC
  cmp _imp_midiInStart, 0
  jz ERROR
  jmp _imp_midiInStart
ERROR:
  mov RAX, 1
  ret
midiInStart ENDP
midiInStop PROC
  cmp _imp_midiInStop, 0
  jz ERROR
  jmp _imp_midiInStop
ERROR:
  mov RAX, 1
  ret
midiInStop ENDP
midiInReset PROC
  cmp _imp_midiInReset, 0
  jz ERROR
  jmp _imp_midiInReset
ERROR:
  mov RAX, 1
  ret
midiInReset ENDP
midiInGetID PROC
  cmp _imp_midiInGetID, 0
  jz ERROR
  jmp _imp_midiInGetID
ERROR:
  mov RAX, 1
  ret
midiInGetID ENDP
midiInMessage PROC
  cmp _imp_midiInMessage, 0
  jz ERROR
  jmp _imp_midiInMessage
ERROR:
  mov RAX, 1
  ret
midiInMessage ENDP

; Mixer Functions
mixerGetNumDevs PROC
  cmp _imp_mixerGetNumDevs, 0
  jz ERROR
  jmp _imp_mixerGetNumDevs
ERROR:
  xor RAX, RAX
  ret
mixerGetNumDevs ENDP
mixerGetDevCapsA PROC
  cmp _imp_mixerGetDevCapsA, 0
  jz ERROR
  jmp _imp_mixerGetDevCapsA
ERROR:
  mov RAX, 1
  ret
mixerGetDevCapsA ENDP
mixerGetDevCapsW PROC
  cmp _imp_mixerGetDevCapsW, 0
  jz ERROR
  jmp _imp_mixerGetDevCapsW
ERROR:
  mov RAX, 1
  ret
mixerGetDevCapsW ENDP
mixerOpen PROC
  cmp _imp_mixerOpen, 0
  jz ERROR
  jmp _imp_mixerOpen
ERROR:
  mov RAX, 1
  ret
mixerOpen ENDP
mixerClose PROC
  cmp _imp_mixerClose, 0
  jz ERROR
  jmp _imp_mixerClose
ERROR:
  mov RAX, 1
  ret
mixerClose ENDP
mixerMessage PROC
  cmp _imp_mixerMessage, 0
  jz ERROR
  jmp _imp_mixerMessage
ERROR:
  mov RAX, 1
  ret
mixerMessage ENDP
mixerGetLineInfoA PROC
  cmp _imp_mixerGetLineInfoA, 0
  jz ERROR
  jmp _imp_mixerGetLineInfoA
ERROR:
  mov RAX, 1
  ret
mixerGetLineInfoA ENDP
mixerGetLineInfoW PROC
  cmp _imp_mixerGetLineInfoW, 0
  jz ERROR
  jmp _imp_mixerGetLineInfoW
ERROR:
  mov RAX, 1
  ret
mixerGetLineInfoW ENDP
mixerGetID PROC
  cmp _imp_mixerGetID, 0
  jz ERROR
  jmp _imp_mixerGetID
ERROR:
  mov RAX, 1
  ret
mixerGetID ENDP
mixerGetLineControlsA PROC
  cmp _imp_mixerGetLineControlsA, 0
  jz ERROR
  jmp _imp_mixerGetLineControlsA
ERROR:
  mov RAX, 1
  ret
mixerGetLineControlsA ENDP
mixerGetLineControlsW PROC
  cmp _imp_mixerGetLineControlsW, 0
  jz ERROR
  jmp _imp_mixerGetLineControlsW
ERROR:
  mov RAX, 1
  ret
mixerGetLineControlsW ENDP
mixerGetControlDetailsA PROC
  cmp _imp_mixerGetControlDetailsA, 0
  jz ERROR
  jmp _imp_mixerGetControlDetailsA
ERROR:
  mov RAX, 1
  ret
mixerGetControlDetailsA ENDP
mixerGetControlDetailsW PROC
  cmp _imp_mixerGetControlDetailsW, 0
  jz ERROR
  jmp _imp_mixerGetControlDetailsW
ERROR:
  mov RAX, 1
  ret
mixerGetControlDetailsW ENDP
mixerSetControlDetails PROC
  cmp _imp_mixerSetControlDetails, 0
  jz ERROR
  jmp _imp_mixerSetControlDetails
ERROR:
  mov RAX, 1
  ret
mixerSetControlDetails ENDP

; Task Functions
mmDrvInstall PROC
  cmp _imp_mmDrvInstall, 0
  jz ERROR
  jmp _imp_mmDrvInstall
ERROR:
  xor RAX, RAX
  ret
mmDrvInstall ENDP
mmGetCurrentTask PROC
  cmp _imp_mmGetCurrentTask, 0
  jz ERROR
  jmp _imp_mmGetCurrentTask
ERROR:
  xor RAX, RAX
  ret
mmGetCurrentTask ENDP
mmTaskBlock PROC
  cmp _imp_mmTaskBlock, 0
  jz ERROR
  jmp _imp_mmTaskBlock
ERROR:
  ret
mmTaskBlock ENDP
mmTaskCreate PROC
  cmp _imp_mmTaskCreate, 0
  jz ERROR
  jmp _imp_mmTaskCreate
ERROR:
  xor RAX, RAX
  ret
mmTaskCreate ENDP
mmTaskSignal PROC
  cmp _imp_mmTaskSignal, 0
  jz ERROR
  jmp _imp_mmTaskSignal
ERROR:
  xor RAX, RAX
  ret
mmTaskSignal ENDP
mmTaskYield PROC
  cmp _imp_mmTaskYield, 0
  jz ERROR
  jmp _imp_mmTaskYield
ERROR:
  ret
mmTaskYield ENDP

; Multimedia io Functions
mmioStringToFOURCCA PROC
  cmp _imp_mmioStringToFOURCCA, 0
  jz ERROR
  jmp _imp_mmioStringToFOURCCA
ERROR:
  xor RAX, RAX
  ret
mmioStringToFOURCCA ENDP
mmioStringToFOURCCW PROC
  cmp _imp_mmioStringToFOURCCW, 0
  jz ERROR
  jmp _imp_mmioStringToFOURCCW
ERROR:
  xor RAX, RAX
  ret
mmioStringToFOURCCW ENDP
mmioInstallIOProcA PROC
  cmp _imp_mmioInstallIOProcA, 0
  jz ERROR
  jmp _imp_mmioInstallIOProcA
ERROR:
  xor RAX, RAX
  ret
mmioInstallIOProcA ENDP
mmioInstallIOProcW PROC
  cmp _imp_mmioInstallIOProcW, 0
  jz ERROR
  jmp _imp_mmioInstallIOProcW
ERROR:
  xor RAX, RAX
  ret
mmioInstallIOProcW ENDP
mmioOpenA PROC
  cmp _imp_mmioOpenA, 0
  jz ERROR
  jmp _imp_mmioOpenA
ERROR:
  mov RAX, 1
  ret
mmioOpenA ENDP
mmioOpenW PROC
  cmp _imp_mmioOpenW, 0
  jz ERROR
  jmp _imp_mmioOpenW
ERROR:
  mov RAX, 1
  ret
mmioOpenW ENDP
mmioRenameA PROC
  cmp _imp_mmioRenameA, 0
  jz ERROR
  jmp _imp_mmioRenameA
ERROR:
  mov RAX, 1
  ret
mmioRenameA ENDP
mmioRenameW PROC
  cmp _imp_mmioRenameW, 0
  jz ERROR
  jmp _imp_mmioRenameW
ERROR:
  mov RAX, 1
  ret
mmioRenameW ENDP
mmioClose PROC
  cmp _imp_mmioClose, 0
  jz ERROR
  jmp _imp_mmioClose
ERROR:
  mov RAX, 1
  ret
mmioClose ENDP
mmioRead PROC
  cmp _imp_mmioRead, 0
  jz ERROR
  jmp _imp_mmioRead
ERROR:
  mov RAX, -1
  ret
mmioRead ENDP
mmioWrite PROC
  cmp _imp_mmioWrite, 0
  jz ERROR
  jmp _imp_mmioWrite
ERROR:
  mov RAX, -1
  ret
mmioWrite ENDP
mmioSeek PROC
  cmp _imp_mmioSeek, 0
  jz ERROR
  jmp _imp_mmioSeek
ERROR:
  mov RAX, -1
  ret
mmioSeek ENDP
mmioGetInfo PROC
  cmp _imp_mmioGetInfo, 0
  jz ERROR
  jmp _imp_mmioGetInfo
ERROR:
  mov RAX, 1
  ret
mmioGetInfo ENDP
mmioSetInfo PROC
  cmp _imp_mmioSetInfo, 0
  jz ERROR
  jmp _imp_mmioSetInfo
ERROR:
  mov RAX, 1
  ret
mmioSetInfo ENDP
mmioSetBuffer PROC
  cmp _imp_mmioSetBuffer, 0
  jz ERROR
  jmp _imp_mmioSetBuffer
ERROR:
  mov RAX, 1
  ret
mmioSetBuffer ENDP
mmioFlush PROC
  cmp _imp_mmioFlush, 0
  jz ERROR
  jmp _imp_mmioFlush
ERROR:
  mov RAX, 1
  ret
mmioFlush ENDP
mmioAdvance PROC
  cmp _imp_mmioAdvance, 0
  jz ERROR
  jmp _imp_mmioAdvance
ERROR:
  mov RAX, 1
  ret
mmioAdvance ENDP
mmioSendMessage PROC
  cmp _imp_mmioSendMessage, 0
  jz ERROR
  jmp _imp_mmioSendMessage
ERROR:
  mov RAX, -1
  ret
mmioSendMessage ENDP
mmioDescend PROC
  cmp _imp_mmioDescend, 0
  jz ERROR
  jmp _imp_mmioDescend
ERROR:
  mov RAX, 1
  ret
mmioDescend ENDP
mmioAscend PROC
  cmp _imp_mmioAscend, 0
  jz ERROR
  jmp _imp_mmioAscend
ERROR:
  mov RAX, 1
  ret
mmioAscend ENDP
mmioCreateChunk PROC
  cmp _imp_mmioCreateChunk, 0
  jz ERROR
  jmp _imp_mmioCreateChunk
ERROR:
  mov RAX, 1
  ret
mmioCreateChunk ENDP

; Time Functions
timeGetSystemTime PROC
  cmp _imp_timeGetSystemTime, 0
  jz ERROR
  jmp _imp_timeGetSystemTime
ERROR:
  mov RAX, 1
  ret
timeGetSystemTime ENDP
timeGetTime PROC
  cmp _imp_timeGetTime, 0
  jz ERROR
  jmp _imp_timeGetTime
ERROR:
  xor RAX, RAX
  ret
timeGetTime ENDP
timeSetEvent PROC
  cmp _imp_timeSetEvent, 0
  jz ERROR
  jmp _imp_timeSetEvent
ERROR:
  mov RAX, 1
  ret
timeSetEvent ENDP
timeKillEvent PROC
  cmp _imp_timeKillEvent, 0
  jz ERROR
  jmp _imp_timeKillEvent
ERROR:
  mov RAX, 1
  ret
timeKillEvent ENDP
timeGetDevCaps PROC
  cmp _imp_timeGetDevCaps, 0
  jz ERROR
  jmp _imp_timeGetDevCaps
ERROR:
  mov RAX, 1
  ret
timeGetDevCaps ENDP
timeBeginPeriod PROC
  cmp _imp_timeBeginPeriod, 0
  jz ERROR
  jmp _imp_timeBeginPeriod
ERROR:
  mov RAX, 1
  ret
timeBeginPeriod ENDP
timeEndPeriod PROC
  cmp _imp_timeEndPeriod, 0
  jz ERROR
  jmp _imp_timeEndPeriod
ERROR:
  mov RAX, 1
  ret
timeEndPeriod ENDP

; Wave Functions
waveOutGetNumDevs PROC
  cmp _imp_waveOutGetNumDevs, 0
  jz ERROR
  jmp _imp_waveOutGetNumDevs
ERROR:
  xor RAX, RAX
  ret
waveOutGetNumDevs ENDP
waveOutGetDevCapsA PROC
  cmp _imp_waveOutGetDevCapsA, 0
  jz ERROR
  jmp _imp_waveOutGetDevCapsA
ERROR:
  mov RAX, 1
  ret
waveOutGetDevCapsA ENDP
waveOutGetDevCapsW PROC
  cmp _imp_waveOutGetDevCapsW, 0
  jz ERROR
  jmp _imp_waveOutGetDevCapsW
ERROR:
  mov RAX, 1
  ret
waveOutGetDevCapsW ENDP
waveOutGetVolume PROC
  cmp _imp_waveOutGetVolume, 0
  jz ERROR
  jmp _imp_waveOutGetVolume
ERROR:
  mov RAX, 1
  ret
waveOutGetVolume ENDP
waveOutSetVolume PROC
  cmp _imp_waveOutSetVolume, 0
  jz ERROR
  jmp _imp_waveOutSetVolume
ERROR:
  mov RAX, 1
  ret
waveOutSetVolume ENDP
waveOutGetErrorTextA PROC
  cmp _imp_waveOutGetErrorTextA, 0
  jz ERROR
  jmp _imp_waveOutGetErrorTextA
ERROR:
  mov RAX, 1
  ret
waveOutGetErrorTextA ENDP
waveOutGetErrorTextW PROC
  cmp _imp_waveOutGetErrorTextW, 0
  jz ERROR
  jmp _imp_waveOutGetErrorTextW
ERROR:
  mov RAX, 1
  ret
waveOutGetErrorTextW ENDP
waveOutOpen PROC
  cmp _imp_waveOutOpen, 0
  jz ERROR
  jmp _imp_waveOutOpen
ERROR:
  mov RAX, 1
  ret
waveOutOpen ENDP
waveOutClose PROC
  cmp _imp_waveOutClose, 0
  jz ERROR
  jmp _imp_waveOutClose
ERROR:
  mov RAX, 1
  ret
waveOutClose ENDP
waveOutPrepareHeader PROC
  cmp _imp_waveOutPrepareHeader, 0
  jz ERROR
  jmp _imp_waveOutPrepareHeader
ERROR:
  mov RAX, 1
  ret
waveOutPrepareHeader ENDP
waveOutUnprepareHeader PROC
  cmp _imp_waveOutUnprepareHeader, 0
  jz ERROR
  jmp _imp_waveOutUnprepareHeader
ERROR:
  mov RAX, 1
  ret
waveOutUnprepareHeader ENDP
waveOutWrite PROC
  cmp _imp_waveOutWrite, 0
  jz ERROR
  jmp _imp_waveOutWrite
ERROR:
  mov RAX, 1
  ret
waveOutWrite ENDP
waveOutPause PROC
  cmp _imp_waveOutPause, 0
  jz ERROR
  jmp _imp_waveOutPause
ERROR:
  mov RAX, 1
  ret
waveOutPause ENDP
waveOutRestart PROC
  cmp _imp_waveOutRestart, 0
  jz ERROR
  jmp _imp_waveOutRestart
ERROR:
  mov RAX, 1
  ret
waveOutRestart ENDP
waveOutReset PROC
  cmp _imp_waveOutReset, 0
  jz ERROR
  jmp _imp_waveOutReset
ERROR:
  mov RAX, 1
  ret
waveOutReset ENDP
waveOutBreakLoop PROC
  cmp _imp_waveOutBreakLoop, 0
  jz ERROR
  jmp _imp_waveOutBreakLoop
ERROR:
  mov RAX, 1
  ret
waveOutBreakLoop ENDP
waveOutGetPosition PROC
  cmp _imp_waveOutGetPosition, 0
  jz ERROR
  jmp _imp_waveOutGetPosition
ERROR:
  mov RAX, 1
  ret
waveOutGetPosition ENDP
waveOutGetPitch PROC
  cmp _imp_waveOutGetPitch, 0
  jz ERROR
  jmp _imp_waveOutGetPitch
ERROR:
  mov RAX, 1
  ret
waveOutGetPitch ENDP
waveOutSetPitch PROC
  cmp _imp_waveOutSetPitch, 0
  jz ERROR
  jmp _imp_waveOutSetPitch
ERROR:
  mov RAX, 1
  ret
waveOutSetPitch ENDP
waveOutGetPlaybackRate PROC
  cmp _imp_waveOutGetPlaybackRate, 0
  jz ERROR
  jmp _imp_waveOutGetPlaybackRate
ERROR:
  mov RAX, 1
  ret
waveOutGetPlaybackRate ENDP
waveOutSetPlaybackRate PROC
  cmp _imp_waveOutSetPlaybackRate, 0
  jz ERROR
  jmp _imp_waveOutSetPlaybackRate
ERROR:
  mov RAX, 1
  ret
waveOutSetPlaybackRate ENDP
waveOutGetID PROC
  cmp _imp_waveOutGetID, 0
  jz ERROR
  jmp _imp_waveOutGetID
ERROR:
  mov RAX, 1
  ret
waveOutGetID ENDP
waveOutMessage PROC
  cmp _imp_waveOutMessage, 0
  jz ERROR
  jmp _imp_waveOutMessage
ERROR:
  mov RAX, 1
  ret
waveOutMessage ENDP
waveInGetNumDevs PROC
  cmp _imp_waveInGetNumDevs, 0
  jz ERROR
  jmp _imp_waveInGetNumDevs
ERROR:
  xor RAX, RAX
  ret
waveInGetNumDevs ENDP
waveInGetDevCapsA PROC
  cmp _imp_waveInGetDevCapsA, 0
  jz ERROR
  jmp _imp_waveInGetDevCapsA
ERROR:
  mov RAX, 1
  ret
waveInGetDevCapsA ENDP
waveInGetDevCapsW PROC
  cmp _imp_waveInGetDevCapsW, 0
  jz ERROR
  jmp _imp_waveInGetDevCapsW
ERROR:
  mov RAX, 1
  ret
waveInGetDevCapsW ENDP
waveInGetErrorTextA PROC
  cmp _imp_waveInGetErrorTextA, 0
  jz ERROR
  jmp _imp_waveInGetErrorTextA
ERROR:
  mov RAX, 1
  ret
waveInGetErrorTextA ENDP
waveInGetErrorTextW PROC
  cmp _imp_waveInGetErrorTextW, 0
  jz ERROR
  jmp _imp_waveInGetErrorTextW
ERROR:
  mov RAX, 1
  ret
waveInGetErrorTextW ENDP
waveInOpen PROC
  cmp _imp_waveInOpen, 0
  jz ERROR
  jmp _imp_waveInOpen
ERROR:
  mov RAX, 1
  ret
waveInOpen ENDP
waveInClose PROC
  cmp _imp_waveInClose, 0
  jz ERROR
  jmp _imp_waveInClose
ERROR:
  mov RAX, 1
  ret
waveInClose ENDP
waveInPrepareHeader PROC
  cmp _imp_waveInPrepareHeader, 0
  jz ERROR
  jmp _imp_waveInPrepareHeader
ERROR:
  mov RAX, 1
  ret
waveInPrepareHeader ENDP
waveInUnprepareHeader PROC
  cmp _imp_waveInUnprepareHeader, 0
  jz ERROR
  jmp _imp_waveInUnprepareHeader
ERROR:
  mov RAX, 1
  ret
waveInUnprepareHeader ENDP
waveInAddBuffer PROC
  cmp _imp_waveInAddBuffer, 0
  jz ERROR
  jmp _imp_waveInAddBuffer
ERROR:
  mov RAX, 1
  ret
waveInAddBuffer ENDP
waveInStart PROC
  cmp _imp_waveInStart, 0
  jz ERROR
  jmp _imp_waveInStart
ERROR:
  mov RAX, 1
  ret
waveInStart ENDP
waveInStop PROC
  cmp _imp_waveInStop, 0
  jz ERROR
  jmp _imp_waveInStop
ERROR:
  mov RAX, 1
  ret
waveInStop ENDP
waveInReset PROC
  cmp _imp_waveInReset, 0
  jz ERROR
  jmp _imp_waveInReset
ERROR:
  mov RAX, 1
  ret
waveInReset ENDP
waveInGetPosition PROC
  cmp _imp_waveInGetPosition, 0
  jz ERROR
  jmp _imp_waveInGetPosition
ERROR:
  mov RAX, 1
  ret
waveInGetPosition ENDP
waveInGetID PROC
  cmp _imp_waveInGetID, 0
  jz ERROR
  jmp _imp_waveInGetID
ERROR:
  mov RAX, 1
  ret
waveInGetID ENDP
waveInMessage PROC
  cmp _imp_waveInMessage, 0
  jz ERROR
  jmp _imp_waveInMessage
ERROR:
  mov RAX, 1
  ret
waveInMessage ENDP

; Message Functions
aux32Message PROC
  cmp _imp_aux32Message, 0
  jz ERROR
  jmp _imp_aux32Message
ERROR:
  xor RAX, RAX
  ret
aux32Message ENDP
joy32Message PROC
  cmp _imp_joy32Message, 0
  jz ERROR
  jmp _imp_joy32Message
ERROR:
  xor RAX, RAX
  ret
joy32Message ENDP
mci32Message PROC
  cmp _imp_mci32Message, 0
  jz ERROR
  jmp _imp_mci32Message
ERROR:
  xor RAX, RAX
  ret
mci32Message ENDP
mid32Message PROC
  cmp _imp_mid32Message, 0
  jz ERROR
  jmp _imp_mid32Message
ERROR:
  xor RAX, RAX
  ret
mid32Message ENDP
mod32Message PROC
  cmp _imp_mod32Message, 0
  jz ERROR
  jmp _imp_mod32Message
ERROR:
  xor RAX, RAX
  ret
mod32Message ENDP
mxd32Message PROC
  cmp _imp_mxd32Message, 0
  jz ERROR
  jmp _imp_mxd32Message
ERROR:
  xor RAX, RAX
  ret
mxd32Message ENDP
tid32Message PROC
  cmp _imp_tid32Message, 0
  jz ERROR
  jmp _imp_tid32Message
ERROR:
  xor RAX, RAX
  ret
tid32Message ENDP
wid32Message PROC
  cmp _imp_wid32Message, 0
  jz ERROR
  jmp _imp_wid32Message
ERROR:
  xor RAX, RAX
  ret
wid32Message ENDP
wod32Message PROC
  cmp _imp_wod32Message, 0
  jz ERROR
  jmp _imp_wod32Message
ERROR:
  xor RAX, RAX
  ret
wod32Message ENDP
END
