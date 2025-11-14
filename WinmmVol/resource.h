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

// Custom minimize and close bottom file
#define IDI_BTN_CLOSE           111 // btn_close.ico

// Tray Menu Commands
#define IDM_EXIT                1001 // Exit menu item ID
#define IDM_SHOW_ALL_APPS       1002 // View All Apps mode menu item ID
#define IDM_MODE_CLEAR          1003 // Reset all settings mode menu item ID
#define IDM_INFO_LICENSE        1004 // Info OSL Dialog menu item ID

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

// Custom title buttons (top-right)
#define IDC_BTN_CLOSE           2011

// About Dialog IDs
#define IDD_ABOUT               3000

// UILang packs (UTF-16 key=value)
#define IDR_UILANG_EN           9001 // English
#define IDR_UILANG_KO           9002 // Korean
#define IDR_UILANG_CA           9003 // Catalan, Valencian
#define IDR_UILANG_CS           9004 // Czech
#define IDR_UILANG_DE           9005 // German
#define IDR_UILANG_ES           9006 // Spanish, Castilian
#define IDR_UILANG_FR           9007 // French
#define IDR_UILANG_ID           9008 // Indonesian
#define IDR_UILANG_IT           9009 // Italiano
#define IDR_UILANG_JA           9010 // Japanese
#define IDR_UILANG_KA           9011 // Georgian
#define IDR_UILANG_LT           9012 // Lithuanian
#define IDR_UILANG_NB           9013 // Norwegian Bokmal
#define IDR_UILANG_NL           9014 // Dutch, Flemish
#define IDR_UILANG_PL           9015 // Polish
#define IDR_UILANG_PT_BR        9016 // Portuguese (Brazil)
#define IDR_UILANG_RU           9017 // Russian
#define IDR_UILANG_TH           9018 // Thai
#define IDR_UILANG_TR           9019 // Turkish
#define IDR_UILANG_ZH_CN        9020 // Chinese(zh-CN)
#define IDR_UILANG_ZH_HK        9021 // Chinese(zh-HK)
#define IDR_UILANG_ZH_TW        9022 // Chinese(zh-TW)

// Example full license text (UTF-16)
#define IDR_LICENSE_MINIMP3     9201 // License\minimp3.txt
#define IDR_LICENSE_LIBOGG      9202 // License\libogg.txt
#define IDR_LICENSE_LIBVORBIS   9203 // License\libvorbis.txt
#define IDR_LICENSE_LIBFLAC     9204 // License\libflac.txt
#define IDR_LICENSE_LIBSAMP     9205 // License\libsamplerate.txt
