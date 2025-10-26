// Enable function only during debugging
// Used only for logging, optimization not applied
#ifdef _DEBUG
#include "GlobalDefinitions.h"
#include <iostream>
#include <tlhelp32.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

static std::wstring gPath = L".\\";

static FILE* fpDebugLog = nullptr;
static FILE* pStreamOut = nullptr;
static char gDebugMessageA[1024];
static wchar_t gDebugMessageW[1024];

char* WcharToChar(const wchar_t* str) {
    char* ret;
    int len;

    if (!str) return nullptr;
    len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
    ret = (char*)HeapAlloc(GetProcessHeap(), 0, len);
    if (ret) WideCharToMultiByte(CP_ACP, 0, str, -1, ret, len, NULL, NULL);
    return ret;
}

wchar_t* CharToWchar(const char* str) {
    wchar_t* ret;
    int len;

    if (!str) return nullptr;
    len = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
    ret = (wchar_t*)HeapAlloc(GetProcessHeap(), 0, len * sizeof(wchar_t));
    if (ret) MultiByteToWideChar(CP_ACP, 0, str, -1, ret, len);
    return ret;
}

void dprintf(const char* formatstring, ...) {
    if (fpDebugLog == nullptr) {
        AllocConsole();
        freopen_s(&pStreamOut, "CONOUT$", "w", stdout);
        fpDebugLog = _wfopen((gPath + L"winmm.log").c_str(), L"w, ccs=UNICODE");
    }
    if (fpDebugLog) {
        int nSize = 0;
        memset(gDebugMessageA, 0, sizeof(gDebugMessageA));
        memset(gDebugMessageW, 0, sizeof(gDebugMessageW));
        va_list args;
        va_start(args, formatstring);
        nSize = _vsnprintf(gDebugMessageA, sizeof(gDebugMessageA), formatstring, args);
        va_end(args);
        OutputDebugStringA(gDebugMessageA);
        gDebugMessageA[nSize++] = '\n';
        printf(gDebugMessageA);
        int sizeNeeded = MultiByteToWideChar(CP_ACP, 0, gDebugMessageA, nSize, nullptr, 0);
        MultiByteToWideChar(CP_ACP, 0, gDebugMessageA, nSize, gDebugMessageW, sizeNeeded);
        fwrite(gDebugMessageW, sizeof(wchar_t), sizeNeeded, fpDebugLog);
        fflush(NULL);
    }
}
void dprintf(const wchar_t* formatstring, ...) {
    if (fpDebugLog == nullptr) {
        AllocConsole();
        freopen_s(&pStreamOut, "CONOUT$", "w", stdout);
        fpDebugLog = _wfopen((gPath + L"winmm.log").c_str(), L"w, ccs=UNICODE");
    }
    if (fpDebugLog) {
        int nSize = 0;
        memset(gDebugMessageW, 0, sizeof(gDebugMessageW));
        va_list args;
        va_start(args, formatstring);
        nSize = _vsnwprintf(gDebugMessageW, sizeof(gDebugMessageW), formatstring, args);
        va_end(args);
        OutputDebugStringW(gDebugMessageW);
        gDebugMessageW[nSize++] = '\n';
        fwrite(gDebugMessageW, sizeof(wchar_t), nSize, fpDebugLog);
        printf(WcharToChar(gDebugMessageW));
        fflush(NULL);
    }
}
#endif
