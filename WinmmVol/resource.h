#pragma once

// Resources
#define IDI_APPICON             101 // Icon resource ID for icon.ico
#define IDR_TRAYMENU            102 // Menu resource ID for tray context menu

// Replaced tray icons with 4 distinct IDs for the multi-icon file
#define IDI_TRAYICON_MUTE       103 // Mute or 0%
#define IDI_TRAYICON_LVL1       104 // 1-32%
#define IDI_TRAYICON_LVL2       105 // 33-65%
#define IDI_TRAYICON_LVL3       106 // 66-100%

// Replaced speaker icons with 4 distinct IDs for the multi-icon file
#define IDI_SPEAKER_MUTE        107 // Mute or 0%
#define IDI_SPEAKER_LVL1        108 // 1-32%
#define IDI_SPEAKER_LVL2        109 // 33-65%
#define IDI_SPEAKER_LVL3        110 // 66-100%

// Tray Menu Commands
#define IDM_EXIT                1001 // Exit menu item ID
#define IDM_ADVANCED            1002 // Advanced popup menu item ID

// Engine Mode Menu IDs
#define IDM_MODE_AUTO           1003 // Buffer: Auto-detect
#define IDM_MODE_STREAMING      1004 // Buffer: Streaming
#define IDM_MODE_FULLBUFFER     1005 // Buffer: Full Buffer
#define IDM_MODE_RESAMPLE       1006 // Buffer: Full Buffer + Resampling

// Engine Mode Menu IDs
#define IDM_ENGINE_AUTO         1007 // Engine: Auto-detect
#define IDM_ENGINE_DS           1008 // Engine: Force DirectSound
#define IDM_ENGINE_WASAPI       1009 // Engine: Force WASAPI

// Info Dialog
#define IDM_INFO_USAGE          1101
#define IDM_INFO_LICENSE        1102

// Volume Slider Controls
#define IDC_VOLUMESLIDER        2001 // Trackbar control ID
#define IDC_VOLUMETEXT          2002 // Static text for percentage ID
#define IDC_VOLUMEICON          2003 // Static control for volume icon
#define IDC_VOLUMETITLE         2004 // Static text for the title

// About Dialog IDs
#define IDD_ABOUT               3000

// RCDATA Resource IDs
#define IDR_USAGE_EN            3101 // en-US\Usage-en.txt
#define IDR_USAGE_KO            3102 // ko-KR\Usage-ko.txt

// Example full license text (UTF-16)
#define IDR_LICENSE_MINIMP3     3201 // License\minimp3.txt
#define IDR_LICENSE_LIBOGG      3202 // License\libogg.txt
#define IDR_LICENSE_LIBVORBIS   3203 // License\libvorbis.txt
#define IDR_LICENSE_LIBFLAC     3204 // License\libflac.txt
#define IDR_LICENSE_LIBSAMP     3205 // License\libsamplerate.txt

// UILang packs (UTF-16 key=value)
#define IDR_UILANG_EN           9001 // en-US\Ui-en.txt
#define IDR_UILANG_KO           9002 // ko-KR\Ui-ko.txt
