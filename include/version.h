#pragma once

// Single source: numeric only
#define VER_MAJOR 2
#define VER_MINOR 2
#define VER_PATCH 0
#define VER_BUILD 0

// stringify helpers
#define VER_STR_HELPER(x) #x
#define VER_STR(x) VER_STR_HELPER(x)

#define VER_FILE_VERSION_STR    VER_STR(VER_MAJOR) "." VER_STR(VER_MINOR) "." VER_STR(VER_PATCH) "." VER_STR(VER_BUILD)
#define VER_PRODUCT_VERSION_STR VER_FILE_VERSION_STR

// Shared metadata
#define VER_COMPANY_NAME        "WinMM CD-DA Stubs Project"
#define VER_PRODUCT_NAME        "Direct-WinMM"
