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
#define IDM_SHOW_ALL_APPS       1002 // View All Apps mode menu item ID
#define IDM_MODE_CLEAR          1003 // Reset all settings mode menu item ID
#define IDM_INFO_USAGE          1004 // Info Usage Dialog menu item ID
#define IDM_INFO_LICENSE        1005 // Info OSL Dialog menu item ID

// Volume Slider Controls
#define IDC_APP_COMBO           2000
#define IDC_VOLUMESLIDER        2001
#define IDC_VOLUMETEXT          2002
#define IDC_VOLUMEICON          2003
#define IDC_VOLUMETITLE         2004
#define IDC_ENGINE_COMBO        2005
#define IDC_BUFFER_COMBO        2006
#define IDC_LABEL_APP           2007
#define IDC_LABEL_VOLUME        2008
#define IDC_LABEL_ENGINE        2009
#define IDC_LABEL_BUFFER        2010

// About Dialog IDs
#define IDD_ABOUT               3000

// UILang packs (UTF-16 key=value)
#define IDR_UILANG_EN           9001 // en-US\Ui-en.txt
#define IDR_UILANG_KO           9002 // ko-KR\Ui-ko.txt

// RCDATA Resource IDs
#define IDR_USAGE_EN            9101 // en-US\Usage-en.txt
#define IDR_USAGE_KO            9102 // ko-KR\Usage-ko.txt

// Example full license text (UTF-16)
#define IDR_LICENSE_MINIMP3     9201 // License\minimp3.txt
#define IDR_LICENSE_LIBOGG      9202 // License\libogg.txt
#define IDR_LICENSE_LIBVORBIS   9203 // License\libvorbis.txt
#define IDR_LICENSE_LIBFLAC     9204 // License\libflac.txt
#define IDR_LICENSE_LIBSAMP     9205 // License\libsamplerate.txt
