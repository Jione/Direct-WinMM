#pragma once

// Constant definitions
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define _WINMM_

// Headers
#include <windows.h>
#include <mmsystem.h>

// Logging function for debugging
#ifdef _DEBUG
void dprintf(const char* formatstring, ...);
void dprintf(const wchar_t* formatstring, ...);
#else
#define dprintf(...)
#endif
