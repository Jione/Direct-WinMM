#pragma once

// Resources
#define IDI_APPICON         101 // Icon resource ID for icon.ico
#define IDR_TRAYMENU        102 // Menu resource ID for tray context menu

// Replaced speaker icons with 4 distinct IDs for the multi-icon file
#define IDI_SPEAKER_MUTE    103 // Mute or 0%
#define IDI_SPEAKER_LVL1    104 // 1-32%
#define IDI_SPEAKER_LVL2    105 // 33-65%
#define IDI_SPEAKER_LVL3    106 // 66-100%

// Tray Menu Commands
#define IDM_EXIT            1001 // Exit menu item ID

// Engine Mode Menu IDs
#define IDM_MODE_AUTO       1002 // Buffer: Auto-detect
#define IDM_MODE_STREAMING  1003 // Buffer: Streaming
#define IDM_MODE_FULLBUFFER 1004 // Buffer: Full Buffer

// Engine Mode Menu IDs
#define IDM_ENGINE_AUTO     1005 // Engine: Auto-detect
#define IDM_ENGINE_DS       1006 // Engine: Force DirectSound
#define IDM_ENGINE_WASAPI   1007 // Engine: Force WASAPI

// Volume Slider Controls
#define IDC_VOLUMESLIDER    2001 // Trackbar control ID
#define IDC_VOLUMETEXT      2002 // Static text for percentage ID
#define IDC_VOLUMEICON      2003 // Static control for volume icon
#define IDC_VOLUMETITLE     2004 // Static text for the title
