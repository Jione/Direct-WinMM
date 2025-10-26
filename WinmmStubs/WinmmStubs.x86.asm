; asm code for WinMM.dll x86(Win32) binary
;
; Build Command: ml.exe /Fo $(ProjectDir)$(IntDir)%(Filename).obj /c /Cp $(ProjectDir)%(Filename).asm
; Output: $(ProjectDir)$(IntDir)%(Filename).obj

.686p
.mmx
.model flat,stdcall
OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

.DATA
; PlaySound Functions
EXTERN _imp_NT4PlaySound:DWORD
EXTERN _imp_PlaySound:DWORD
EXTERN _imp_sndPlaySoundA:DWORD
EXTERN _imp_sndPlaySoundW:DWORD
EXTERN _imp_PlaySoundA:DWORD
EXTERN _imp_PlaySoundW:DWORD

; Driver and Drv help Functions
EXTERN _imp_CloseDriver:DWORD
EXTERN _imp_OpenDriver:DWORD
EXTERN _imp_SendDriverMessage:DWORD
EXTERN _imp_DrvGetModuleHandle:DWORD
EXTERN _imp_GetDriverModuleHandle:DWORD
EXTERN _imp_DefDriverProc:DWORD
EXTERN _imp_DrvClose:DWORD
EXTERN _imp_DriverCallback:DWORD
EXTERN _imp_DrvOpen:DWORD
EXTERN _imp_DrvSendMessage:DWORD
EXTERN _imp_mmsystemGetVersion:DWORD
EXTERN _imp_NotifyCallbackData:DWORD
EXTERN _imp_winmmDbgOut:DWORD
EXTERN _imp_winmmSetDebugLevel:DWORD
EXTERN _imp_MigrateAllDrivers:DWORD
EXTERN _imp_MigrateSoundEvents:DWORD
EXTERN _imp_WinmmLogon:DWORD
EXTERN _imp_WinmmLogoff:DWORD
EXTERN _imp_WOW32DriverCallback:DWORD
EXTERN _imp_WOW32ResolveMultiMediaHandle:DWORD
EXTERN _imp_WOWAppExit:DWORD

; Audio GFX support Functions
EXTERN _imp_gfxCreateZoneFactoriesList:DWORD
EXTERN _imp_gfxCreateGfxFactoriesList:DWORD
EXTERN _imp_gfxDestroyDeviceInterfaceList:DWORD
EXTERN _imp_gfxEnumerateGfxs:DWORD
EXTERN _imp_gfxRemoveGfx:DWORD
EXTERN _imp_gfxAddGfx:DWORD
EXTERN _imp_gfxModifyGfx:DWORD
EXTERN _imp_gfxOpenGfx:DWORD
EXTERN _imp_gfxBatchChange:DWORD

; AUX Functions
EXTERN _imp_auxGetDevCapsA:DWORD
EXTERN _imp_auxGetDevCapsW:DWORD
EXTERN _imp_auxGetNumDevs:DWORD
EXTERN _imp_auxGetVolume:DWORD
EXTERN _imp_auxOutMessage:DWORD
EXTERN _imp_auxSetVolume:DWORD

; Joystick Functions
EXTERN _imp_joyGetNumDevs:DWORD
EXTERN _imp_joyGetDevCapsA:DWORD
EXTERN _imp_joyGetDevCapsW:DWORD
EXTERN _imp_joyGetPos:DWORD
EXTERN _imp_joyGetPosEx:DWORD
EXTERN _imp_joyGetThreshold:DWORD
EXTERN _imp_joyReleaseCapture:DWORD
EXTERN _imp_joySetCapture:DWORD
EXTERN _imp_joySetThreshold:DWORD
EXTERN _imp_joySetCalibration:DWORD
EXTERN _imp_joyConfigChanged:DWORD

; MCI Functions
EXTERN _imp_mciExecute:DWORD
EXTERN _imp_mciSendCommandA:DWORD
EXTERN _imp_mciSendCommandW:DWORD
EXTERN _imp_mciSendStringA:DWORD
EXTERN _imp_mciSendStringW:DWORD
EXTERN _imp_mciGetDeviceIDA:DWORD
EXTERN _imp_mciGetDeviceIDW:DWORD
EXTERN _imp_mciGetDeviceIDFromElementIDA:DWORD
EXTERN _imp_mciGetDeviceIDFromElementIDW:DWORD
EXTERN _imp_mciGetErrorStringA:DWORD
EXTERN _imp_mciGetErrorStringW:DWORD
EXTERN _imp_mciSetYieldProc:DWORD
EXTERN _imp_mciGetCreatorTask:DWORD
EXTERN _imp_mciGetYieldProc:DWORD
EXTERN _imp_mciDriverNotify:DWORD
EXTERN _imp_mciDriverYield:DWORD
EXTERN _imp_mciFreeCommandResource:DWORD
EXTERN _imp_mciGetDriverData:DWORD
EXTERN _imp_mciLoadCommandResource:DWORD
EXTERN _imp_mciSetDriverData:DWORD
EXTERN _imp_mciEatCommandEntry:DWORD
EXTERN _imp_mciGetParamSize:DWORD
EXTERN _imp_mciUnlockCommandTable:DWORD
EXTERN _imp_FindCommandItem:DWORD

; MIDI Functions
EXTERN _imp_midiOutGetNumDevs:DWORD
EXTERN _imp_midiStreamOpen:DWORD
EXTERN _imp_midiStreamClose:DWORD
EXTERN _imp_midiStreamProperty:DWORD
EXTERN _imp_midiStreamPosition:DWORD
EXTERN _imp_midiStreamOut:DWORD
EXTERN _imp_midiStreamPause:DWORD
EXTERN _imp_midiStreamRestart:DWORD
EXTERN _imp_midiStreamStop:DWORD
EXTERN _imp_midiConnect:DWORD
EXTERN _imp_midiDisconnect:DWORD
EXTERN _imp_midiOutGetDevCapsA:DWORD
EXTERN _imp_midiOutGetDevCapsW:DWORD
EXTERN _imp_midiOutGetVolume:DWORD
EXTERN _imp_midiOutSetVolume:DWORD
EXTERN _imp_midiOutGetErrorTextA:DWORD
EXTERN _imp_midiOutGetErrorTextW:DWORD
EXTERN _imp_midiOutOpen:DWORD
EXTERN _imp_midiOutClose:DWORD
EXTERN _imp_midiOutPrepareHeader:DWORD
EXTERN _imp_midiOutUnprepareHeader:DWORD
EXTERN _imp_midiOutShortMsg:DWORD
EXTERN _imp_midiOutLongMsg:DWORD
EXTERN _imp_midiOutReset:DWORD
EXTERN _imp_midiOutCachePatches:DWORD
EXTERN _imp_midiOutCacheDrumPatches:DWORD
EXTERN _imp_midiOutGetID:DWORD
EXTERN _imp_midiOutMessage:DWORD
EXTERN _imp_midiInGetNumDevs:DWORD
EXTERN _imp_midiInGetDevCapsA:DWORD
EXTERN _imp_midiInGetDevCapsW:DWORD
EXTERN _imp_midiInGetErrorTextA:DWORD
EXTERN _imp_midiInGetErrorTextW:DWORD
EXTERN _imp_midiInOpen:DWORD
EXTERN _imp_midiInClose:DWORD
EXTERN _imp_midiInPrepareHeader:DWORD
EXTERN _imp_midiInUnprepareHeader:DWORD
EXTERN _imp_midiInAddBuffer:DWORD
EXTERN _imp_midiInStart:DWORD
EXTERN _imp_midiInStop:DWORD
EXTERN _imp_midiInReset:DWORD
EXTERN _imp_midiInGetID:DWORD
EXTERN _imp_midiInMessage:DWORD

; Mixer Functions
EXTERN _imp_mixerGetNumDevs:DWORD
EXTERN _imp_mixerGetDevCapsA:DWORD
EXTERN _imp_mixerGetDevCapsW:DWORD
EXTERN _imp_mixerOpen:DWORD
EXTERN _imp_mixerClose:DWORD
EXTERN _imp_mixerMessage:DWORD
EXTERN _imp_mixerGetLineInfoA:DWORD
EXTERN _imp_mixerGetLineInfoW:DWORD
EXTERN _imp_mixerGetID:DWORD
EXTERN _imp_mixerGetLineControlsA:DWORD
EXTERN _imp_mixerGetLineControlsW:DWORD
EXTERN _imp_mixerGetControlDetailsA:DWORD
EXTERN _imp_mixerGetControlDetailsW:DWORD
EXTERN _imp_mixerSetControlDetails:DWORD

; Task Functions
EXTERN _imp_mmDrvInstall:DWORD
EXTERN _imp_mmGetCurrentTask:DWORD
EXTERN _imp_mmTaskBlock:DWORD
EXTERN _imp_mmTaskCreate:DWORD
EXTERN _imp_mmTaskSignal:DWORD
EXTERN _imp_mmTaskYield:DWORD

; Multimedia io Functions
EXTERN _imp_mmioStringToFOURCCA:DWORD
EXTERN _imp_mmioStringToFOURCCW:DWORD
EXTERN _imp_mmioInstallIOProcA:DWORD
EXTERN _imp_mmioInstallIOProcW:DWORD
EXTERN _imp_mmioOpenA:DWORD
EXTERN _imp_mmioOpenW:DWORD
EXTERN _imp_mmioRenameA:DWORD
EXTERN _imp_mmioRenameW:DWORD
EXTERN _imp_mmioClose:DWORD
EXTERN _imp_mmioRead:DWORD
EXTERN _imp_mmioWrite:DWORD
EXTERN _imp_mmioSeek:DWORD
EXTERN _imp_mmioGetInfo:DWORD
EXTERN _imp_mmioSetInfo:DWORD
EXTERN _imp_mmioSetBuffer:DWORD
EXTERN _imp_mmioFlush:DWORD
EXTERN _imp_mmioAdvance:DWORD
EXTERN _imp_mmioSendMessage:DWORD
EXTERN _imp_mmioDescend:DWORD
EXTERN _imp_mmioAscend:DWORD
EXTERN _imp_mmioCreateChunk:DWORD

; Time Functions
EXTERN _imp_timeGetSystemTime:DWORD
EXTERN _imp_timeGetTime:DWORD
EXTERN _imp_timeSetEvent:DWORD
EXTERN _imp_timeKillEvent:DWORD
EXTERN _imp_timeGetDevCaps:DWORD
EXTERN _imp_timeBeginPeriod:DWORD
EXTERN _imp_timeEndPeriod:DWORD

; Wave Functions
EXTERN _imp_waveOutGetNumDevs:DWORD
EXTERN _imp_waveOutGetDevCapsA:DWORD
EXTERN _imp_waveOutGetDevCapsW:DWORD
EXTERN _imp_waveOutGetVolume:DWORD
EXTERN _imp_waveOutSetVolume:DWORD
EXTERN _imp_waveOutGetErrorTextA:DWORD
EXTERN _imp_waveOutGetErrorTextW:DWORD
EXTERN _imp_waveOutOpen:DWORD
EXTERN _imp_waveOutClose:DWORD
EXTERN _imp_waveOutPrepareHeader:DWORD
EXTERN _imp_waveOutUnprepareHeader:DWORD
EXTERN _imp_waveOutWrite:DWORD
EXTERN _imp_waveOutPause:DWORD
EXTERN _imp_waveOutRestart:DWORD
EXTERN _imp_waveOutReset:DWORD
EXTERN _imp_waveOutBreakLoop:DWORD
EXTERN _imp_waveOutGetPosition:DWORD
EXTERN _imp_waveOutGetPitch:DWORD
EXTERN _imp_waveOutSetPitch:DWORD
EXTERN _imp_waveOutGetPlaybackRate:DWORD
EXTERN _imp_waveOutSetPlaybackRate:DWORD
EXTERN _imp_waveOutGetID:DWORD
EXTERN _imp_waveOutMessage:DWORD
EXTERN _imp_waveInGetNumDevs:DWORD
EXTERN _imp_waveInGetDevCapsA:DWORD
EXTERN _imp_waveInGetDevCapsW:DWORD
EXTERN _imp_waveInGetErrorTextA:DWORD
EXTERN _imp_waveInGetErrorTextW:DWORD
EXTERN _imp_waveInOpen:DWORD
EXTERN _imp_waveInClose:DWORD
EXTERN _imp_waveInPrepareHeader:DWORD
EXTERN _imp_waveInUnprepareHeader:DWORD
EXTERN _imp_waveInAddBuffer:DWORD
EXTERN _imp_waveInStart:DWORD
EXTERN _imp_waveInStop:DWORD
EXTERN _imp_waveInReset:DWORD
EXTERN _imp_waveInGetPosition:DWORD
EXTERN _imp_waveInGetID:DWORD
EXTERN _imp_waveInMessage:DWORD

; Message Functions
EXTERN _imp_aux32Message:DWORD
EXTERN _imp_joy32Message:DWORD
EXTERN _imp_mci32Message:DWORD
EXTERN _imp_mid32Message:DWORD
EXTERN _imp_mod32Message:DWORD
EXTERN _imp_mxd32Message:DWORD
EXTERN _imp_tid32Message:DWORD
EXTERN _imp_wid32Message:DWORD
EXTERN _imp_wod32Message:DWORD


.CODE
; PlaySound Functions
NT4PlaySound PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_NT4PlaySound, 0
  jz ERROR
  jmp _imp_NT4PlaySound
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
NT4PlaySound ENDP
PlaySound PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_PlaySound, 0
  jz ERROR
  jmp _imp_PlaySound
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
PlaySound ENDP
sndPlaySoundA PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_sndPlaySoundA, 0
  jz ERROR
  jmp _imp_sndPlaySoundA
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
sndPlaySoundA ENDP
sndPlaySoundW PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_sndPlaySoundW, 0
  jz ERROR
  jmp _imp_sndPlaySoundW
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
sndPlaySoundW ENDP
PlaySoundA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_PlaySoundA, 0
  jz ERROR
  jmp _imp_PlaySoundA
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
PlaySoundA ENDP
PlaySoundW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_PlaySoundW, 0
  jz ERROR
  jmp _imp_PlaySoundW
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
PlaySoundW ENDP

; Driver and Drv help Functions
CloseDriver PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_CloseDriver, 0
  jz ERROR
  jmp _imp_CloseDriver
ERROR:
  mov EAX, -1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
CloseDriver ENDP
OpenDriver PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_OpenDriver, 0
  jz ERROR
  jmp _imp_OpenDriver
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
OpenDriver ENDP
SendDriverMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_SendDriverMessage, 0
  jz ERROR
  jmp _imp_SendDriverMessage
ERROR:
  mov EAX, -1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
SendDriverMessage ENDP
DrvGetModuleHandle PROC, arg1:DWORD
  cmp _imp_DrvGetModuleHandle, 0
  jz ERROR
  jmp _imp_DrvGetModuleHandle
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DrvGetModuleHandle ENDP
GetDriverModuleHandle PROC, arg1:DWORD
  cmp _imp_GetDriverModuleHandle, 0
  jz ERROR
  jmp _imp_GetDriverModuleHandle
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
GetDriverModuleHandle ENDP
DefDriverProc PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_DefDriverProc, 0
  jz ERROR
  jmp _imp_DefDriverProc
ERROR:
  mov EAX, -1
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DefDriverProc ENDP
DrvClose PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_DrvClose, 0
  jz ERROR
  jmp _imp_DrvClose
ERROR:
  mov EAX, -1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DrvClose ENDP
DriverCallback PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD, arg7:DWORD
  cmp _imp_DriverCallback, 0
  jz ERROR
  jmp _imp_DriverCallback
ERROR:
  xor EAX, EAX
  ret 28
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DriverCallback ENDP
DrvOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_DrvOpen, 0
  jz ERROR
  jmp _imp_DrvOpen
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DrvOpen ENDP
DrvSendMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_DrvSendMessage, 0
  jz ERROR
  jmp _imp_DrvSendMessage
ERROR:
  mov EAX, -1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
DrvSendMessage ENDP
mmsystemGetVersion PROC
  cmp _imp_mmsystemGetVersion, 0
  jz ERROR
  jmp _imp_mmsystemGetVersion
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmsystemGetVersion ENDP
NotifyCallbackData PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_NotifyCallbackData, 0
  jz ERROR
  jmp _imp_NotifyCallbackData
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
NotifyCallbackData ENDP
winmmDbgOut PROC
  cmp _imp_winmmDbgOut, 0
  jz ERROR
  jmp _imp_winmmDbgOut
ERROR:
  ret
winmmDbgOut ENDP
winmmSetDebugLevel PROC, arg1:DWORD
  cmp _imp_winmmSetDebugLevel, 0
  jz ERROR
  jmp _imp_winmmSetDebugLevel
ERROR:
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
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
WinmmLogon PROC, arg1:DWORD
  cmp _imp_WinmmLogon, 0
  jz ERROR
  jmp _imp_WinmmLogon
ERROR:
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
WinmmLogon ENDP
WinmmLogoff PROC
  cmp _imp_WinmmLogoff, 0
  jz ERROR
  jmp _imp_WinmmLogoff
ERROR:
  ret
WinmmLogoff ENDP
WOW32DriverCallback PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD, arg7:DWORD
  cmp _imp_WOW32DriverCallback, 0
  jz ERROR
  jmp _imp_WOW32DriverCallback
ERROR:
  xor EAX, EAX
  ret 28
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
WOW32DriverCallback ENDP
WOW32ResolveMultiMediaHandle PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD
  cmp _imp_WOW32ResolveMultiMediaHandle, 0
  jz ERROR
  jmp _imp_WOW32ResolveMultiMediaHandle
ERROR:
  xor EAX, EAX
  ret 24
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
WOW32ResolveMultiMediaHandle ENDP
WOWAppExit PROC, arg1:DWORD
  cmp _imp_WOWAppExit, 0
  jz ERROR
  jmp _imp_WOWAppExit
ERROR:
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
WOWAppExit ENDP

; Audio GFX support Functions
gfxCreateZoneFactoriesList PROC, arg1:DWORD
  cmp _imp_gfxCreateZoneFactoriesList, 0
  jz ERROR
  jmp _imp_gfxCreateZoneFactoriesList
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxCreateZoneFactoriesList ENDP
gfxCreateGfxFactoriesList PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_gfxCreateGfxFactoriesList, 0
  jz ERROR
  jmp _imp_gfxCreateGfxFactoriesList
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxCreateGfxFactoriesList ENDP
gfxDestroyDeviceInterfaceList PROC, arg1:DWORD
  cmp _imp_gfxDestroyDeviceInterfaceList, 0
  jz ERROR
  jmp _imp_gfxDestroyDeviceInterfaceList
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxDestroyDeviceInterfaceList ENDP
gfxEnumerateGfxs PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_gfxEnumerateGfxs, 0
  jz ERROR
  jmp _imp_gfxEnumerateGfxs
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxEnumerateGfxs ENDP
gfxRemoveGfx PROC, arg1:DWORD
  cmp _imp_gfxRemoveGfx, 0
  jz ERROR
  jmp _imp_gfxRemoveGfx
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxRemoveGfx ENDP
gfxAddGfx PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_gfxAddGfx, 0
  jz ERROR
  jmp _imp_gfxAddGfx
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxAddGfx ENDP
gfxModifyGfx PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_gfxModifyGfx, 0
  jz ERROR
  jmp _imp_gfxModifyGfx
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxModifyGfx ENDP
gfxOpenGfx PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_gfxOpenGfx, 0
  jz ERROR
  jmp _imp_gfxOpenGfx
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxOpenGfx ENDP
gfxBatchChange PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD
  cmp _imp_gfxBatchChange, 0
  jz ERROR
  jmp _imp_gfxBatchChange
ERROR:
  xor EAX, EAX
  ret 24
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
gfxBatchChange ENDP

; AUX Functions
auxGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_auxGetDevCapsA, 0
  jz ERROR
  jmp _imp_auxGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxGetDevCapsA ENDP
auxGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_auxGetDevCapsW, 0
  jz ERROR
  jmp _imp_auxGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxGetDevCapsW ENDP
auxGetNumDevs PROC
  cmp _imp_auxGetNumDevs, 0
  jz ERROR
  jmp _imp_auxGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxGetNumDevs ENDP
auxGetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_auxGetVolume, 0
  jz ERROR
  jmp _imp_auxGetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxGetVolume ENDP
auxOutMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_auxOutMessage, 0
  jz ERROR
  jmp _imp_auxOutMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxOutMessage ENDP
auxSetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_auxSetVolume, 0
  jz ERROR
  jmp _imp_auxSetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
auxSetVolume ENDP

; Joystick Functions
joyGetNumDevs PROC
  cmp _imp_joyGetNumDevs, 0
  jz ERROR
  jmp _imp_joyGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetNumDevs ENDP
joyGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_joyGetDevCapsA, 0
  jz ERROR
  jmp _imp_joyGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetDevCapsA ENDP
joyGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_joyGetDevCapsW, 0
  jz ERROR
  jmp _imp_joyGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetDevCapsW ENDP
joyGetPos PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_joyGetPos, 0
  jz ERROR
  jmp _imp_joyGetPos
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetPos ENDP
joyGetPosEx PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_joyGetPosEx, 0
  jz ERROR
  jmp _imp_joyGetPosEx
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetPosEx ENDP
joyGetThreshold PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_joyGetThreshold, 0
  jz ERROR
  jmp _imp_joyGetThreshold
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyGetThreshold ENDP
joyReleaseCapture PROC, arg1:DWORD
  cmp _imp_joyReleaseCapture, 0
  jz ERROR
  jmp _imp_joyReleaseCapture
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyReleaseCapture ENDP
joySetCapture PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_joySetCapture, 0
  jz ERROR
  jmp _imp_joySetCapture
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joySetCapture ENDP
joySetThreshold PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_joySetThreshold, 0
  jz ERROR
  jmp _imp_joySetThreshold
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joySetThreshold ENDP
joySetCalibration PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD, arg7:DWORD
  cmp _imp_joySetCalibration, 0
  jz ERROR
  jmp _imp_joySetCalibration
ERROR:
  mov EAX, -1
  ret 28
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joySetCalibration ENDP
joyConfigChanged PROC, arg1:DWORD
  cmp _imp_joyConfigChanged, 0
  jz ERROR
  jmp _imp_joyConfigChanged
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joyConfigChanged ENDP

; MCI Functions
mciExecute PROC, arg1:DWORD
  cmp _imp_mciExecute, 0
  jz ERROR
  jmp _imp_mciExecute
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciExecute ENDP
mciSendCommandA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mciSendCommandA, 0
  jz ERROR
  jmp _imp_mciSendCommandA
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSendCommandA ENDP
mciSendCommandW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mciSendCommandW, 0
  jz ERROR
  jmp _imp_mciSendCommandW
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSendCommandW ENDP
mciSendStringA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mciSendStringA, 0
  jz ERROR
  jmp _imp_mciSendStringA
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSendStringA ENDP
mciSendStringW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mciSendStringW, 0
  jz ERROR
  jmp _imp_mciSendStringW
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSendStringW ENDP
mciGetDeviceIDA PROC, arg1:DWORD
  cmp _imp_mciGetDeviceIDA, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDA
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetDeviceIDA ENDP
mciGetDeviceIDW PROC, arg1:DWORD
  cmp _imp_mciGetDeviceIDW, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDW
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetDeviceIDW ENDP
mciGetDeviceIDFromElementIDA PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mciGetDeviceIDFromElementIDA, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDFromElementIDA
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetDeviceIDFromElementIDA ENDP
mciGetDeviceIDFromElementIDW PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mciGetDeviceIDFromElementIDW, 0
  jz ERROR
  jmp _imp_mciGetDeviceIDFromElementIDW
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetDeviceIDFromElementIDW ENDP
mciGetErrorStringA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciGetErrorStringA, 0
  jz ERROR
  jmp _imp_mciGetErrorStringA
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetErrorStringA ENDP
mciGetErrorStringW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciGetErrorStringW, 0
  jz ERROR
  jmp _imp_mciGetErrorStringW
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetErrorStringW ENDP
mciSetYieldProc PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciSetYieldProc, 0
  jz ERROR
  jmp _imp_mciSetYieldProc
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSetYieldProc ENDP
mciGetCreatorTask PROC, arg1:DWORD
  cmp _imp_mciGetCreatorTask, 0
  jz ERROR
  jmp _imp_mciGetCreatorTask
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetCreatorTask ENDP
mciGetYieldProc PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mciGetYieldProc, 0
  jz ERROR
  jmp _imp_mciGetYieldProc
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetYieldProc ENDP
mciDriverNotify PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciDriverNotify, 0
  jz ERROR
  jmp _imp_mciDriverNotify
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciDriverNotify ENDP
mciDriverYield PROC, arg1:DWORD
  cmp _imp_mciDriverYield, 0
  jz ERROR
  jmp _imp_mciDriverYield
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciDriverYield ENDP
mciFreeCommandResource PROC, arg1:DWORD
  cmp _imp_mciFreeCommandResource, 0
  jz ERROR
  jmp _imp_mciFreeCommandResource
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciFreeCommandResource ENDP
mciGetDriverData PROC, arg1:DWORD
  cmp _imp_mciGetDriverData, 0
  jz ERROR
  jmp _imp_mciGetDriverData
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetDriverData ENDP
mciLoadCommandResource PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciLoadCommandResource, 0
  jz ERROR
  jmp _imp_mciLoadCommandResource
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciLoadCommandResource ENDP
mciSetDriverData PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mciSetDriverData, 0
  jz ERROR
  jmp _imp_mciSetDriverData
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciSetDriverData ENDP
mciEatCommandEntry PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mciEatCommandEntry, 0
  jz ERROR
  jmp _imp_mciEatCommandEntry
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciEatCommandEntry ENDP
mciGetParamSize PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mciGetParamSize, 0
  jz ERROR
  jmp _imp_mciGetParamSize
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciGetParamSize ENDP
mciUnlockCommandTable PROC, arg1:DWORD
  cmp _imp_mciUnlockCommandTable, 0
  jz ERROR
  jmp _imp_mciUnlockCommandTable
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mciUnlockCommandTable ENDP
FindCommandItem PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_FindCommandItem, 0
  jz ERROR
  jmp _imp_FindCommandItem
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
FindCommandItem ENDP

; MIDI Functions
midiOutGetNumDevs PROC
  cmp _imp_midiOutGetNumDevs, 0
  jz ERROR
  jmp _imp_midiOutGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetNumDevs ENDP
midiStreamOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD
  cmp _imp_midiStreamOpen, 0
  jz ERROR
  jmp _imp_midiStreamOpen
ERROR:
  mov EAX, 1
  ret 24
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamOpen ENDP
midiStreamClose PROC, arg1:DWORD
  cmp _imp_midiStreamClose, 0
  jz ERROR
  jmp _imp_midiStreamClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamClose ENDP
midiStreamProperty PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiStreamProperty, 0
  jz ERROR
  jmp _imp_midiStreamProperty
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamProperty ENDP
midiStreamPosition PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiStreamPosition, 0
  jz ERROR
  jmp _imp_midiStreamPosition
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamPosition ENDP
midiStreamOut PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiStreamOut, 0
  jz ERROR
  jmp _imp_midiStreamOut
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamOut ENDP
midiStreamPause PROC, arg1:DWORD
  cmp _imp_midiStreamPause, 0
  jz ERROR
  jmp _imp_midiStreamPause
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamPause ENDP
midiStreamRestart PROC, arg1:DWORD
  cmp _imp_midiStreamRestart, 0
  jz ERROR
  jmp _imp_midiStreamRestart
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamRestart ENDP
midiStreamStop PROC, arg1:DWORD
  cmp _imp_midiStreamStop, 0
  jz ERROR
  jmp _imp_midiStreamStop
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiStreamStop ENDP
midiConnect PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiConnect, 0
  jz ERROR
  jmp _imp_midiConnect
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiConnect ENDP
midiDisconnect PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiDisconnect, 0
  jz ERROR
  jmp _imp_midiDisconnect
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiDisconnect ENDP
midiOutGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutGetDevCapsA, 0
  jz ERROR
  jmp _imp_midiOutGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetDevCapsA ENDP
midiOutGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutGetDevCapsW, 0
  jz ERROR
  jmp _imp_midiOutGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetDevCapsW ENDP
midiOutGetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_midiOutGetVolume, 0
  jz ERROR
  jmp _imp_midiOutGetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetVolume ENDP
midiOutSetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_midiOutSetVolume, 0
  jz ERROR
  jmp _imp_midiOutSetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutSetVolume ENDP
midiOutGetErrorTextA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutGetErrorTextA, 0
  jz ERROR
  jmp _imp_midiOutGetErrorTextA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetErrorTextA ENDP
midiOutGetErrorTextW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutGetErrorTextW, 0
  jz ERROR
  jmp _imp_midiOutGetErrorTextW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetErrorTextW ENDP
midiOutOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_midiOutOpen, 0
  jz ERROR
  jmp _imp_midiOutOpen
ERROR:
  mov EAX, 1
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutOpen ENDP
midiOutClose PROC, arg1:DWORD
  cmp _imp_midiOutClose, 0
  jz ERROR
  jmp _imp_midiOutClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutClose ENDP
midiOutPrepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutPrepareHeader, 0
  jz ERROR
  jmp _imp_midiOutPrepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutPrepareHeader ENDP
midiOutUnprepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutUnprepareHeader, 0
  jz ERROR
  jmp _imp_midiOutUnprepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutUnprepareHeader ENDP
midiOutShortMsg PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_midiOutShortMsg, 0
  jz ERROR
  jmp _imp_midiOutShortMsg
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutShortMsg ENDP
midiOutLongMsg PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiOutLongMsg, 0
  jz ERROR
  jmp _imp_midiOutLongMsg
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutLongMsg ENDP
midiOutReset PROC, arg1:DWORD
  cmp _imp_midiOutReset, 0
  jz ERROR
  jmp _imp_midiOutReset
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutReset ENDP
midiOutCachePatches PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_midiOutCachePatches, 0
  jz ERROR
  jmp _imp_midiOutCachePatches
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutCachePatches ENDP
midiOutCacheDrumPatches PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_midiOutCacheDrumPatches, 0
  jz ERROR
  jmp _imp_midiOutCacheDrumPatches
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutCacheDrumPatches ENDP
midiOutGetID PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_midiOutGetID, 0
  jz ERROR
  jmp _imp_midiOutGetID
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutGetID ENDP
midiOutMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_midiOutMessage, 0
  jz ERROR
  jmp _imp_midiOutMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiOutMessage ENDP
midiInGetNumDevs PROC
  cmp _imp_midiInGetNumDevs, 0
  jz ERROR
  jmp _imp_midiInGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetNumDevs ENDP
midiInGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInGetDevCapsA, 0
  jz ERROR
  jmp _imp_midiInGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetDevCapsA ENDP
midiInGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInGetDevCapsW, 0
  jz ERROR
  jmp _imp_midiInGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetDevCapsW ENDP
midiInGetErrorTextA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInGetErrorTextA, 0
  jz ERROR
  jmp _imp_midiInGetErrorTextA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetErrorTextA ENDP
midiInGetErrorTextW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInGetErrorTextW, 0
  jz ERROR
  jmp _imp_midiInGetErrorTextW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetErrorTextW ENDP
midiInOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_midiInOpen, 0
  jz ERROR
  jmp _imp_midiInOpen
ERROR:
  mov EAX, 1
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInOpen ENDP
midiInClose PROC, arg1:DWORD
  cmp _imp_midiInClose, 0
  jz ERROR
  jmp _imp_midiInClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInClose ENDP
midiInPrepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInPrepareHeader, 0
  jz ERROR
  jmp _imp_midiInPrepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInPrepareHeader ENDP
midiInUnprepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInUnprepareHeader, 0
  jz ERROR
  jmp _imp_midiInUnprepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInUnprepareHeader ENDP
midiInAddBuffer PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_midiInAddBuffer, 0
  jz ERROR
  jmp _imp_midiInAddBuffer
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInAddBuffer ENDP
midiInStart PROC, arg1:DWORD
  cmp _imp_midiInStart, 0
  jz ERROR
  jmp _imp_midiInStart
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInStart ENDP
midiInStop PROC, arg1:DWORD
  cmp _imp_midiInStop, 0
  jz ERROR
  jmp _imp_midiInStop
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInStop ENDP
midiInReset PROC, arg1:DWORD
  cmp _imp_midiInReset, 0
  jz ERROR
  jmp _imp_midiInReset
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInReset ENDP
midiInGetID PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_midiInGetID, 0
  jz ERROR
  jmp _imp_midiInGetID
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInGetID ENDP
midiInMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_midiInMessage, 0
  jz ERROR
  jmp _imp_midiInMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
midiInMessage ENDP

; Mixer Functions
mixerGetNumDevs PROC
  cmp _imp_mixerGetNumDevs, 0
  jz ERROR
  jmp _imp_mixerGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetNumDevs ENDP
mixerGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetDevCapsA, 0
  jz ERROR
  jmp _imp_mixerGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetDevCapsA ENDP
mixerGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetDevCapsW, 0
  jz ERROR
  jmp _imp_mixerGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetDevCapsW ENDP
mixerOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_mixerOpen, 0
  jz ERROR
  jmp _imp_mixerOpen
ERROR:
  mov EAX, 1
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerOpen ENDP
mixerClose PROC, arg1:DWORD
  cmp _imp_mixerClose, 0
  jz ERROR
  jmp _imp_mixerClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerClose ENDP
mixerMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mixerMessage, 0
  jz ERROR
  jmp _imp_mixerMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerMessage ENDP
mixerGetLineInfoA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetLineInfoA, 0
  jz ERROR
  jmp _imp_mixerGetLineInfoA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetLineInfoA ENDP
mixerGetLineInfoW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetLineInfoW, 0
  jz ERROR
  jmp _imp_mixerGetLineInfoW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetLineInfoW ENDP
mixerGetID PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetID, 0
  jz ERROR
  jmp _imp_mixerGetID
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetID ENDP
mixerGetLineControlsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetLineControlsA, 0
  jz ERROR
  jmp _imp_mixerGetLineControlsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetLineControlsA ENDP
mixerGetLineControlsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetLineControlsW, 0
  jz ERROR
  jmp _imp_mixerGetLineControlsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetLineControlsW ENDP
mixerGetControlDetailsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetControlDetailsA, 0
  jz ERROR
  jmp _imp_mixerGetControlDetailsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetControlDetailsA ENDP
mixerGetControlDetailsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerGetControlDetailsW, 0
  jz ERROR
  jmp _imp_mixerGetControlDetailsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerGetControlDetailsW ENDP
mixerSetControlDetails PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mixerSetControlDetails, 0
  jz ERROR
  jmp _imp_mixerSetControlDetails
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mixerSetControlDetails ENDP

; Task Functions
mmDrvInstall PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmDrvInstall, 0
  jz ERROR
  jmp _imp_mmDrvInstall
ERROR:
  xor EAX, EAX
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmDrvInstall ENDP
mmGetCurrentTask PROC
  cmp _imp_mmGetCurrentTask, 0
  jz ERROR
  jmp _imp_mmGetCurrentTask
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmGetCurrentTask ENDP
mmTaskBlock PROC, arg1:DWORD
  cmp _imp_mmTaskBlock, 0
  jz ERROR
  jmp _imp_mmTaskBlock
ERROR:
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmTaskBlock ENDP
mmTaskCreate PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmTaskCreate, 0
  jz ERROR
  jmp _imp_mmTaskCreate
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmTaskCreate ENDP
mmTaskSignal PROC, arg1:DWORD
  cmp _imp_mmTaskSignal, 0
  jz ERROR
  jmp _imp_mmTaskSignal
ERROR:
  xor EAX, EAX
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmTaskSignal ENDP
mmTaskYield PROC
  cmp _imp_mmTaskYield, 0
  jz ERROR
  jmp _imp_mmTaskYield
ERROR:
  ret
mmTaskYield ENDP

; Multimedia io Functions
mmioStringToFOURCCA PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mmioStringToFOURCCA, 0
  jz ERROR
  jmp _imp_mmioStringToFOURCCA
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioStringToFOURCCA ENDP
mmioStringToFOURCCW PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mmioStringToFOURCCW, 0
  jz ERROR
  jmp _imp_mmioStringToFOURCCW
ERROR:
  xor EAX, EAX
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioStringToFOURCCW ENDP
mmioInstallIOProcA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioInstallIOProcA, 0
  jz ERROR
  jmp _imp_mmioInstallIOProcA
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioInstallIOProcA ENDP
mmioInstallIOProcW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioInstallIOProcW, 0
  jz ERROR
  jmp _imp_mmioInstallIOProcW
ERROR:
  xor EAX, EAX
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioInstallIOProcW ENDP
mmioOpenA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioOpenA, 0
  jz ERROR
  jmp _imp_mmioOpenA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioOpenA ENDP
mmioOpenW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioOpenW, 0
  jz ERROR
  jmp _imp_mmioOpenW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioOpenW ENDP
mmioRenameA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmioRenameA, 0
  jz ERROR
  jmp _imp_mmioRenameA
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioRenameA ENDP
mmioRenameW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmioRenameW, 0
  jz ERROR
  jmp _imp_mmioRenameW
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioRenameW ENDP
mmioClose PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mmioClose, 0
  jz ERROR
  jmp _imp_mmioClose
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioClose ENDP
mmioRead PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioRead, 0
  jz ERROR
  jmp _imp_mmioRead
ERROR:
  mov EAX, -1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioRead ENDP
mmioWrite PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioWrite, 0
  jz ERROR
  jmp _imp_mmioWrite
ERROR:
  mov EAX, -1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioWrite ENDP
mmioSeek PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioSeek, 0
  jz ERROR
  jmp _imp_mmioSeek
ERROR:
  mov EAX, -1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioSeek ENDP
mmioGetInfo PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioGetInfo, 0
  jz ERROR
  jmp _imp_mmioGetInfo
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioGetInfo ENDP
mmioSetInfo PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioSetInfo, 0
  jz ERROR
  jmp _imp_mmioSetInfo
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioSetInfo ENDP
mmioSetBuffer PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmioSetBuffer, 0
  jz ERROR
  jmp _imp_mmioSetBuffer
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioSetBuffer ENDP
mmioFlush PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_mmioFlush, 0
  jz ERROR
  jmp _imp_mmioFlush
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioFlush ENDP
mmioAdvance PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioAdvance, 0
  jz ERROR
  jmp _imp_mmioAdvance
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioAdvance ENDP
mmioSendMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmioSendMessage, 0
  jz ERROR
  jmp _imp_mmioSendMessage
ERROR:
  mov EAX, -1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioSendMessage ENDP
mmioDescend PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_mmioDescend, 0
  jz ERROR
  jmp _imp_mmioDescend
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioDescend ENDP
mmioAscend PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioAscend, 0
  jz ERROR
  jmp _imp_mmioAscend
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioAscend ENDP
mmioCreateChunk PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_mmioCreateChunk, 0
  jz ERROR
  jmp _imp_mmioCreateChunk
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mmioCreateChunk ENDP

; Time Functions
timeGetSystemTime PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_timeGetSystemTime, 0
  jz ERROR
  jmp _imp_timeGetSystemTime
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeGetSystemTime ENDP
timeGetTime PROC
  cmp _imp_timeGetTime, 0
  jz ERROR
  jmp _imp_timeGetTime
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeGetTime ENDP
timeSetEvent PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_timeSetEvent, 0
  jz ERROR
  jmp _imp_timeSetEvent
ERROR:
  mov EAX, 1
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeSetEvent ENDP
timeKillEvent PROC, arg1:DWORD
  cmp _imp_timeKillEvent, 0
  jz ERROR
  jmp _imp_timeKillEvent
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeKillEvent ENDP
timeGetDevCaps PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_timeGetDevCaps, 0
  jz ERROR
  jmp _imp_timeGetDevCaps
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeGetDevCaps ENDP
timeBeginPeriod PROC, arg1:DWORD
  cmp _imp_timeBeginPeriod, 0
  jz ERROR
  jmp _imp_timeBeginPeriod
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeBeginPeriod ENDP
timeEndPeriod PROC, arg1:DWORD
  cmp _imp_timeEndPeriod, 0
  jz ERROR
  jmp _imp_timeEndPeriod
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
timeEndPeriod ENDP

; Wave Functions
waveOutGetNumDevs PROC
  cmp _imp_waveOutGetNumDevs, 0
  jz ERROR
  jmp _imp_waveOutGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetNumDevs ENDP
waveOutGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutGetDevCapsA, 0
  jz ERROR
  jmp _imp_waveOutGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetDevCapsA ENDP
waveOutGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutGetDevCapsW, 0
  jz ERROR
  jmp _imp_waveOutGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetDevCapsW ENDP
waveOutGetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutGetVolume, 0
  jz ERROR
  jmp _imp_waveOutGetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetVolume ENDP
waveOutSetVolume PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutSetVolume, 0
  jz ERROR
  jmp _imp_waveOutSetVolume
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutSetVolume ENDP
waveOutGetErrorTextA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutGetErrorTextA, 0
  jz ERROR
  jmp _imp_waveOutGetErrorTextA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetErrorTextA ENDP
waveOutGetErrorTextW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutGetErrorTextW, 0
  jz ERROR
  jmp _imp_waveOutGetErrorTextW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetErrorTextW ENDP
waveOutOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD
  cmp _imp_waveOutOpen, 0
  jz ERROR
  jmp _imp_waveOutOpen
ERROR:
  mov EAX, 1
  ret 24
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutOpen ENDP
waveOutClose PROC, arg1:DWORD
  cmp _imp_waveOutClose, 0
  jz ERROR
  jmp _imp_waveOutClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutClose ENDP
waveOutPrepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutPrepareHeader, 0
  jz ERROR
  jmp _imp_waveOutPrepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutPrepareHeader ENDP
waveOutUnprepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutUnprepareHeader, 0
  jz ERROR
  jmp _imp_waveOutUnprepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutUnprepareHeader ENDP
waveOutWrite PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutWrite, 0
  jz ERROR
  jmp _imp_waveOutWrite
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutWrite ENDP
waveOutPause PROC, arg1:DWORD
  cmp _imp_waveOutPause, 0
  jz ERROR
  jmp _imp_waveOutPause
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutPause ENDP
waveOutRestart PROC, arg1:DWORD
  cmp _imp_waveOutRestart, 0
  jz ERROR
  jmp _imp_waveOutRestart
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutRestart ENDP
waveOutReset PROC, arg1:DWORD
  cmp _imp_waveOutReset, 0
  jz ERROR
  jmp _imp_waveOutReset
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutReset ENDP
waveOutBreakLoop PROC, arg1:DWORD
  cmp _imp_waveOutBreakLoop, 0
  jz ERROR
  jmp _imp_waveOutBreakLoop
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutBreakLoop ENDP
waveOutGetPosition PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveOutGetPosition, 0
  jz ERROR
  jmp _imp_waveOutGetPosition
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetPosition ENDP
waveOutGetPitch PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutGetPitch, 0
  jz ERROR
  jmp _imp_waveOutGetPitch
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetPitch ENDP
waveOutSetPitch PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutSetPitch, 0
  jz ERROR
  jmp _imp_waveOutSetPitch
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutSetPitch ENDP
waveOutGetPlaybackRate PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutGetPlaybackRate, 0
  jz ERROR
  jmp _imp_waveOutGetPlaybackRate
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetPlaybackRate ENDP
waveOutSetPlaybackRate PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutSetPlaybackRate, 0
  jz ERROR
  jmp _imp_waveOutSetPlaybackRate
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutSetPlaybackRate ENDP
waveOutGetID PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveOutGetID, 0
  jz ERROR
  jmp _imp_waveOutGetID
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutGetID ENDP
waveOutMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_waveOutMessage, 0
  jz ERROR
  jmp _imp_waveOutMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveOutMessage ENDP
waveInGetNumDevs PROC
  cmp _imp_waveInGetNumDevs, 0
  jz ERROR
  jmp _imp_waveInGetNumDevs
ERROR:
  xor EAX, EAX
  ret
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetNumDevs ENDP
waveInGetDevCapsA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInGetDevCapsA, 0
  jz ERROR
  jmp _imp_waveInGetDevCapsA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetDevCapsA ENDP
waveInGetDevCapsW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInGetDevCapsW, 0
  jz ERROR
  jmp _imp_waveInGetDevCapsW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetDevCapsW ENDP
waveInGetErrorTextA PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInGetErrorTextA, 0
  jz ERROR
  jmp _imp_waveInGetErrorTextA
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetErrorTextA ENDP
waveInGetErrorTextW PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInGetErrorTextW, 0
  jz ERROR
  jmp _imp_waveInGetErrorTextW
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetErrorTextW ENDP
waveInOpen PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD, arg6:DWORD
  cmp _imp_waveInOpen, 0
  jz ERROR
  jmp _imp_waveInOpen
ERROR:
  mov EAX, 1
  ret 24
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInOpen ENDP
waveInClose PROC, arg1:DWORD
  cmp _imp_waveInClose, 0
  jz ERROR
  jmp _imp_waveInClose
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInClose ENDP
waveInPrepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInPrepareHeader, 0
  jz ERROR
  jmp _imp_waveInPrepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInPrepareHeader ENDP
waveInUnprepareHeader PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInUnprepareHeader, 0
  jz ERROR
  jmp _imp_waveInUnprepareHeader
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInUnprepareHeader ENDP
waveInAddBuffer PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInAddBuffer, 0
  jz ERROR
  jmp _imp_waveInAddBuffer
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInAddBuffer ENDP
waveInStart PROC, arg1:DWORD
  cmp _imp_waveInStart, 0
  jz ERROR
  jmp _imp_waveInStart
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInStart ENDP
waveInStop PROC, arg1:DWORD
  cmp _imp_waveInStop, 0
  jz ERROR
  jmp _imp_waveInStop
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInStop ENDP
waveInReset PROC, arg1:DWORD
  cmp _imp_waveInReset, 0
  jz ERROR
  jmp _imp_waveInReset
ERROR:
  mov EAX, 1
  ret 4
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInReset ENDP
waveInGetPosition PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD
  cmp _imp_waveInGetPosition, 0
  jz ERROR
  jmp _imp_waveInGetPosition
ERROR:
  mov EAX, 1
  ret 12
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetPosition ENDP
waveInGetID PROC, arg1:DWORD, arg2:DWORD
  cmp _imp_waveInGetID, 0
  jz ERROR
  jmp _imp_waveInGetID
ERROR:
  mov EAX, 1
  ret 8
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInGetID ENDP
waveInMessage PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD
  cmp _imp_waveInMessage, 0
  jz ERROR
  jmp _imp_waveInMessage
ERROR:
  mov EAX, 1
  ret 16
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
waveInMessage ENDP

; Message Functions
aux32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_aux32Message, 0
  jz ERROR
  jmp _imp_aux32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
aux32Message ENDP
joy32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_joy32Message, 0
  jz ERROR
  jmp _imp_joy32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
joy32Message ENDP
mci32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_mci32Message, 0
  jz ERROR
  jmp _imp_mci32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mci32Message ENDP
mid32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_mid32Message, 0
  jz ERROR
  jmp _imp_mid32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mid32Message ENDP
mod32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_mod32Message, 0
  jz ERROR
  jmp _imp_mod32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mod32Message ENDP
mxd32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_mxd32Message, 0
  jz ERROR
  jmp _imp_mxd32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
mxd32Message ENDP
tid32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_tid32Message, 0
  jz ERROR
  jmp _imp_tid32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
tid32Message ENDP
wid32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_wid32Message, 0
  jz ERROR
  jmp _imp_wid32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
wid32Message ENDP
wod32Message PROC, arg1:DWORD, arg2:DWORD, arg3:DWORD, arg4:DWORD, arg5:DWORD
  cmp _imp_wod32Message, 0
  jz ERROR
  jmp _imp_wod32Message
ERROR:
  xor EAX, EAX
  ret 20
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
  int 3
wod32Message ENDP
END
