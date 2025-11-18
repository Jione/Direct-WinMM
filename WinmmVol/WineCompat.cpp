#include "WineCompat.h"
#include "UiLang.h"
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <winnt.h>
#include <winver.h>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "version.lib")

// Simple message helpers
namespace {
    enum PE_ERROR {
        // Convert Error section
        PE_ERR_BASE = 0,
        PE_NOERROR = 0,     // No error
        PE_GET_SIZE,        // Failed to get file size.
        PE_LARGE_MAP,       // File is too large to map.
        PE_CREATE_MAP,      // Failed to create file mapping.
        PE_FAIL_MAP,        // Failed to map file.
        PE_SMALL_HEADER,    // File is too small to contain a valid PE header.
        PE_MISSING_MZ,      // Selected file is not a valid PE (missing MZ header).
        PE_INVALID_OFFSET,  // Invalid PE header offset.
        PE_MISSING_PE,      // Selected file is not a valid PE (missing PE signature).
        PE_INVALID_OPTION,  // PE optional header is truncated or invalid.
        PE_UNKNOWN_OPTION,  // Unknown PE optional header type.
        PE_NO_SECTIONS,     // PE file has no sections.
        PE_OUT_OF_FILE,     // PE section headers are out of file bounds.
        PE_INVALID_TBL_PTR, // Import table pointer is invalid.
        PE_MODULE_PTR,      // Failed to resolve module name pointer.
        PE_MODULE_NAME,     // Invalid module name.
        PE_IMPORT_TRUNKS,   // Failed to resolve import thunks.

        // Convert Information section
        PE_NO_HAS_TARGET,   // Missing Import table(rdata) or not contained WINMM.DLL.
        PE_NO_HAS_FUNC,     // WINMM.DLL is imported, but no function names were patched (imports may be by ordinal).
        PE_ALREADY_IMPORT,  // File has already been converted.

        // error code of Open API
        INVALID_PATH,       // Failed to resolve directory from selected path.
        DLL_NOT_FOUND,      // Either _inmm.dll or winmm.dll must exist in the selected folder.
        FAILED_OPEN,        // Selected file cannot be opened with read/write access.
        FAILED_BACKUP,      // Failed to create .org backup file.

        // Success modify file
        SUCCESS_CONVERT,
        SUCCESS_RECOVER,

        // Unknown error
        PE_ERR_UNKNOWN
    };

    static const char* const gFunctions[] = {
        //PlaySound Functions
        "Unnamed2", "NT4PlaySound", "PlaySound", "PlaySoundA", "PlaySoundW", "mmsystemGetVersion",
        "sndPlaySoundA", "sndPlaySoundW", "winmmDbgOut", "winmmSetDebugLeve",

        //MCI Functions
        "FindCommandItem", "mciDriverNotify", "mciDriverYield", "mciEatCommandEntry", "mciExecute",
        "mciFreeCommandResource", "mciGetCreatorTask", "mciGetDeviceIDA", "mciGetDeviceIDW",
        "mciGetDeviceIDFromElementIDA", "mciGetDeviceIDFromElementIDW",
        "mciGetDriverData", "mciGetErrorStringA", "mciGetErrorStringW", "mciGetParamSize", "mciGetYieldProc",
        "mciLoadCommandResource", "mciSendCommandA", "mciSendCommandW", "mciSendStringA", "mciSendStringW",
        "mciSetDriverData", "mciSetYieldProc", "mciUnlockCommandTable",

        // Driver helper functions
        "DriverCallback", "mmDrvInstal",

        // MMIO functions
        "mmioAdvance", "mmioAscend", "mmioClose", "mmioCreateChunk", "mmioDescend", "mmioFlush",
        "mmioGetInfo", "mmioInstallIOProcA", "mmioInstallIOProcW", "mmioOpenA", "mmioOpenW",
        "mmioRead", "mmioRenameA", "mmioRenameW", "mmioSeek", "mmioSendMessage", "mmioSetBuffer",
        "mmioSetInfo", "mmioStringToFOURCCA", "mmioStringToFOURCCW", "mmioWrite",

        // JOY stick API
        "joyConfigChanged", "joyGetDevCapsA", "joyGetDevCapsW", "joyGetNumDevs", "joyGetPos", "joyGetPosEx",
        "joyGetThreshold", "joyReleaseCapture", "joySetCalibration", "joySetCapture", "joySetThreshold",

        // MIDI functions
        "midiConnect", "midiDisconnect", "midiInAddBuffer", "midiInClose", "midiInGetDevCapsA", "midiInGetDevCapsW",
        "midiInGetErrorTextA", "midiInGetErrorTextW", "midiInGetID", "midiInGetNumDevs", "midiInMessage",
        "midiInOpen", "midiInPrepareHeader", "midiInReset", "midiInStart", "midiInStop",
        "midiInUnprepareHeader", "midiOutCacheDrumPatches", "midiOutCachePatches", "midiOutClose",
        "midiOutGetDevCapsA", "midiOutGetDevCapsW", "midiOutGetErrorTextA", "midiOutGetErrorTextW",
        "midiOutGetID", "midiOutGetNumDevs", "midiOutGetVolume", "midiOutLongMsg", "midiOutMessage",
        "midiOutOpen", "midiOutPrepareHeader", "midiOutReset", "midiOutSetVolume", "midiOutShortMsg",
        "midiOutUnprepareHeader", "midiStreamClose", "midiStreamOpen", "midiStreamOut", "midiStreamPause",
        "midiStreamPosition", "midiStreamProperty", "midiStreamRestart", "midiStreamStop",

        // AUX interface
        "auxGetDevCapsA", "auxGetDevCapsW", "auxGetNumDevs", "auxGetVolume", "auxOutMessage", "auxSetVolume",

        // WAVE out interface
        "waveOutBreakLoop", "waveOutClose", "waveOutGetDevCapsA", "waveOutGetDevCapsW",
        "waveOutGetErrorTextA", "waveOutGetErrorTextW", "waveOutGetID", "waveOutGetNumDevs", "waveOutGetPitch",
        "waveOutGetPlaybackRate", "waveOutGetPosition", "waveOutGetVolume", "waveOutMessage",
        "waveOutOpen", "waveOutPause", "waveOutPrepareHeader", "waveOutReset", "waveOutRestart",
        "waveOutSetPitch", "waveOutSetPlaybackRate", "waveOutSetVolume", "waveOutUnprepareHeader", "waveOutWrite",

        // WAVE in interface
        "waveInAddBuffer", "waveInClose", "waveInGetDevCapsA", "waveInGetDevCapsW", "waveInGetErrorTextA",
        "waveInGetErrorTextW", "waveInGetID", "waveInGetNumDevs", "waveInGetPosition", "waveInMessage",
        "waveInOpen", "waveInPrepareHeader", "waveInReset", "waveInStart", "waveInStop", "waveInUnprepareHeader",

        // TIME interface
        "timeBeginPeriod", "timeEndPeriod", "timeGetDevCaps", "timeGetSystemTime",
        "timeGetTime", "timeKillEvent", "timeSetEvent",

        // MIXER interface
        "mixerClose", "mixerGetControlDetailsA", "mixerGetControlDetailsW", "mixerGetDevCapsA", "mixerGetDevCapsW",
        "mixerGetID", "mixerGetLineControlsA", "mixerGetLineControlsW", "mixerGetLineInfoA", "mixerGetLineInfoW",
        "mixerGetNumDevs", "mixerMessage", "mixerOpen", "mixerSetControlDetails",

        // TASK
        "WOWAppExit", "mmGetCurrentTask", "mmTaskBlock", "mmTaskCreate", "mmTaskSigna", "mmTaskYield",

        // INSTALLABLE DRIVER
        "CloseDriver", "DefDriverProc", "DrvClose", "DrvGetModuleHandle", "DrvOpen", "DrvSendMessage",
        "GetDriverModuleHandle", "OpenDriver", "SendDriverMessage",

        // WOW Thunks
        // "aux32Message", "joy32Message", "mci32Message", "mid32Message", "mod32Message",
        // "mxd32Message", "tid32Message", "wid32Message", "wod32Message",
        "NotifyCallbackData", "WOW32DriverCallback", "WOW32ResolveMultiMediaHandle",

        // Win NT Specific Registry
        "MigrateAllDrivers", "MigrateSoundEvents", "WinmmLogoff", "WinmmLogon",

        // Audio GFX support
        "gfxAddGfx", "gfxBatchChange", "gfxCreateGfxFactoriesList", "gfxCreateZoneFactoriesList",
        "gfxDestroyDeviceInterfaceList", "gfxEnumerateGfxs", "gfxModifyGfx", "gfxOpenGfx", "gfxRemoveGfx"
    };

    static const wchar_t gMessgageTitle[] = L"Direct-WinMM EXE Patcher for Wine Compatibility";
    static constexpr int gFuncSize = (sizeof(gFunctions) / sizeof(gFunctions[0]));

    static const wchar_t* getNowFileTime() {
        static wchar_t time[17]{ 0, };
        FILETIME ftNow; GetSystemTimeAsFileTime(&ftNow);
        wsprintfW(time, L"%08x%08x", ftNow.dwHighDateTime, ftNow.dwLowDateTime);
        return time;
    }

    // Reads FileVersion and OriginalFilename from version resource.
    // Returns TRUE on success. originalIsWinmm is TRUE when OriginalFilename == "winmm.dll".
    static BOOL GetFileVersionAndOriginalName(const std::wstring& path, ULARGE_INTEGER& fileVersion, BOOL& originalIsWinmm) {
        fileVersion.QuadPart = 0;
        originalIsWinmm = FALSE;

        DWORD handle = 0;
        DWORD size = GetFileVersionInfoSizeW(path.c_str(), &handle);
        if (size == 0) {
            return FALSE;
        }

        std::vector<BYTE> buffer(size);
        if (!GetFileVersionInfoW(path.c_str(), 0, size, buffer.data())) {
            return FALSE;
        }

        VS_FIXEDFILEINFO* pffi = NULL;
        UINT ffiLen = 0;
        if (!VerQueryValueW(buffer.data(), L"\\", reinterpret_cast<LPVOID*>(&pffi), &ffiLen)) {
            return FALSE;
        }
        if (!pffi) {
            return FALSE;
        }

        fileVersion.HighPart = pffi->dwFileVersionMS;
        fileVersion.LowPart = pffi->dwFileVersionLS;

        struct LANGANDCODEPAGE {
            WORD wLanguage;
            WORD wCodePage;
        };

        LANGANDCODEPAGE* lpTranslate = NULL;
        UINT cbTranslate = 0;

        if (VerQueryValueW(buffer.data(),
            L"\\VarFileInfo\\Translation",
            reinterpret_cast<LPVOID*>(&lpTranslate),
            &cbTranslate) &&
            lpTranslate && cbTranslate >= sizeof(LANGANDCODEPAGE)) {

            wchar_t subBlock[64];
            wsprintfW(subBlock,
                L"\\StringFileInfo\\%04x%04x\\OriginalFilename",
                lpTranslate[0].wLanguage,
                lpTranslate[0].wCodePage);

            LPVOID lpBuffer = NULL;
            UINT cchBuffer = 0;
            if (VerQueryValueW(buffer.data(), subBlock, &lpBuffer, &cchBuffer) &&
                lpBuffer && cchBuffer > 0) {
                const wchar_t* originalName = static_cast<const wchar_t*>(lpBuffer);
                if (_wcsicmp(originalName, L"winmm.dll") == 0) {
                    originalIsWinmm = TRUE;
                }
            }
        }

        return TRUE;
    }

    static BOOL ErrorMessageHandle(PE_ERROR errNo) {
        const static UINT errType = MB_ICONERROR | MB_OK;
        const static UINT infoType = MB_ICONINFORMATION | MB_OK;
        const static UINT tryType = MB_ICONINFORMATION | MB_YESNO;

        if (errNo == PE_NOERROR) {
            return TRUE;
        }
        else if (errNo < PE_ERR_BASE || errNo > PE_ERR_UNKNOWN) {
            return FALSE;
        }

        std::wstring tmp;
        UINT uType;
        const wchar_t* pMsg;

        // Internal error of PE Convert
        if (errNo <= PE_IMPORT_TRUNKS) {
#ifdef _DEBUG
            switch (errNo) {
            case PE_GET_SIZE:
                pMsg = L"Failed to get file size.";
            case PE_LARGE_MAP:
                pMsg = L"File is too large to map.";
            case PE_CREATE_MAP:
                pMsg = L"Failed to create file mapping.";
            case PE_FAIL_MAP:
                pMsg = L"Failed to map file.";
            case PE_SMALL_HEADER:
                pMsg = L"File is too small to contain a valid PE header.";
            case PE_MISSING_MZ:
                pMsg = L"Selected file is not a valid PE (missing MZ header).";
            case PE_INVALID_OFFSET:
                pMsg = L"Invalid PE header offset.";
            case PE_MISSING_PE:
                pMsg = L"Selected file is not a valid PE (missing PE signature).";
            case PE_INVALID_OPTION:
                pMsg = L"PE optional header is truncated or invalid.";
            case PE_UNKNOWN_OPTION:
                pMsg = L"Unknown PE optional header type.";
            case PE_NO_SECTIONS:
                pMsg = L"PE file has no sections.";
            case PE_OUT_OF_FILE:
                pMsg = L"PE section headers are out of file bounds.";
            case PE_INVALID_TBL_PTR:
                pMsg = L"Import table pointer is invalid.";
            case PE_MODULE_PTR:
                pMsg = L"Failed to resolve module name pointer.";
            case PE_MODULE_NAME:
                pMsg = L"Invalid module name.";
            case PE_IMPORT_TRUNKS:
                pMsg = L"Failed to resolve import thunks.";
            default:
                pMsg = UILang::Get(L"ERR_UNKNOWN", tmp);
            }
            uType = errType;
#else
            wchar_t msg[256]{ 0, };
            wsprintfW(msg, UILang::Get(L"ERR_PE_INTERNAL", tmp), errNo);
            pMsg = msg;
            uType = errType;
#endif
        }
        else {
            wchar_t* msg;
            switch (errNo) {
            case PE_NO_HAS_TARGET:
            case PE_NO_HAS_FUNC:
                msg = L"INFO_PE_INTERNAL";
                uType = infoType;
                break;
            case INVALID_PATH:
                msg = L"ERR_INVALID_PATH";
                uType = errType;
                break;
            case FAILED_OPEN:
                msg = L"ERR_FAIL_OPEN";
                uType = errType;
                break;
            case PE_ALREADY_IMPORT:
                msg = L"INFO_PE_ALREADY_IMPORT";
                uType = tryType;
                break;
            case FAILED_BACKUP:
                msg = L"INFO_FAIL_BACKUP";
                uType = tryType;
                break;
            case DLL_NOT_FOUND:
                msg = L"INFO_DLL_NOT_FOUND";
                uType = tryType;
                break;
            case SUCCESS_CONVERT:
                msg = L"INFO_SUCCESS_MODIFY";
                uType = infoType;
                break;
            case SUCCESS_RECOVER:
                msg = L"INFO_SUCCESS_RECOVERY";
                uType = infoType;
                break;
            default:
                msg = L"ERR_UNKNOWN";
                uType = errType;
            }
            pMsg = UILang::Get(msg, tmp);
        }
        return (MessageBoxW(NULL, pMsg, gMessgageTitle, uType) == IDYES) ? TRUE : FALSE;
    }

    static BOOL ConvertFunctionName(char* funcName, BOOL isUnpatch, BOOL checkMode) {
        if (!funcName || funcName[1] == 0) return FALSE;
        char* funcPos = &funcName[1];

        for (int i = 0; i < gFuncSize; i++) {
            if (!_stricmp(&gFunctions[i][1], funcPos)) {
                if (!checkMode)
                     funcName[0] = isUnpatch ? gFunctions[i][0] : '_';
                return TRUE;
            }
        }
        return FALSE;
    }

    static BOOL CompareFileHandle(HANDLE hFile1, HANDLE hFile2) {
        if (hFile1 == INVALID_HANDLE_VALUE || hFile2 == INVALID_HANDLE_VALUE) {
            return FALSE;
        }

        LARGE_INTEGER fileSize1, fileSize2;
        if (!GetFileSizeEx(hFile1, &fileSize1) || !GetFileSizeEx(hFile2, &fileSize2)) {
            return FALSE;
        }

        if (fileSize1.QuadPart != fileSize2.QuadPart) {
            return FALSE;
        }

        const DWORD BUFFER_SIZE = 4096;
        BYTE buffer1[BUFFER_SIZE];
        BYTE buffer2[BUFFER_SIZE];
        DWORD bytesRead1, bytesRead2;
        BOOL result = TRUE;

        SetFilePointer(hFile1, 0, NULL, FILE_BEGIN);
        SetFilePointer(hFile2, 0, NULL, FILE_BEGIN);

        while (TRUE) {
            if ((!ReadFile(hFile1, buffer1, BUFFER_SIZE, &bytesRead1, NULL)) ||
                (!ReadFile(hFile2, buffer2, BUFFER_SIZE, &bytesRead2, NULL)) ||
                (bytesRead1 != bytesRead2)) {
                result = FALSE;
                break;
            }
            else if (bytesRead1 == 0) break;
            if (memcmp(buffer1, buffer2, bytesRead1) != 0) {
                result = FALSE;
                break;
            }
        }
        return result;
    }

    static wchar_t* GetFilterMessage() {
        // static buffer for OPENFILENAME filter string
        static wchar_t buffer[256];
        static BOOL initialized = FALSE;
        
        if (initialized) {
            return buffer;
        }

        SHFILEINFOW sfiExe;
        SHFILEINFOW sfiDll;
        ZeroMemory(&sfiExe, sizeof(sfiExe));
        ZeroMemory(&sfiDll, sizeof(sfiDll));

        // localized type names for .exe, .dll, .* (fallbacks provided)
        if (!SHGetFileInfoW(L".exe", FILE_ATTRIBUTE_NORMAL, &sfiExe, sizeof(sfiExe), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)) {
            sfiExe.szTypeName[0] = L'\0';
        }

        if (!SHGetFileInfoW(L".dll", FILE_ATTRIBUTE_NORMAL, &sfiDll, sizeof(sfiDll), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)) {
            sfiDll.szTypeName[0] = L'\0';
        }

        const wchar_t* exeName = (sfiExe.szTypeName[0] != L'\0') ? sfiExe.szTypeName : L"Executable Files";
        const wchar_t* dllName = (sfiDll.szTypeName[0] != L'\0') ? sfiDll.szTypeName : L"DLL Files";

        wchar_t* p = buffer;
        size_t remain = sizeof(buffer) / sizeof(buffer[0]);

        // simple helper macro to append a string without overrunning buffer
#define APPEND_STR_W(src)                                      \
        do {                                                   \
            const wchar_t* __s = (src);                        \
            size_t __len = wcslen(__s);                        \
            if (__len >= remain) {                             \
                buffer[0] = L'\0';                             \
                initialized = TRUE;                            \
                return buffer;                                 \
            }                                                  \
            memcpy(p, __s, __len * sizeof(wchar_t));           \
            p += __len;                                        \
            remain -= __len;                                   \
        } while (0)

        // All Files (*.*)\0*.*\0
        std::wstring allFlies;
        if (UILang::Get(L"UI_ALL_FILE", allFlies)[0]) {
            APPEND_STR_W(allFlies.c_str());
            APPEND_STR_W(L" (*.*)");
        }
        else {
            APPEND_STR_W(L"All Files (*.*)");
        }
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.*");
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        // <exeName> (*.exe)\0*.exe\0
        APPEND_STR_W(exeName);
        APPEND_STR_W(L" (*.exe)");
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.exe");
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        // <dllName> (*.dll)\0*.dll\0
        APPEND_STR_W(dllName);
        APPEND_STR_W(L" (*.dll)");
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.dll");
        if (remain == 0) { buffer[0] = L'\0'; initialized = TRUE; return buffer; }
        *p++ = L'\0';
        --remain;

        // final double-null terminator
        if (remain == 0) {
            buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = L'\0';
        }
        else {
            *p++ = L'\0';
        }

        initialized = TRUE;
        return buffer;

#undef APPEND_STR_W
    }

    static BOOL OpenExeOrDllDialog(std::wstring& outPath) {
        wchar_t fileBuffer[MAX_PATH] = { 0 };

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = fileBuffer;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrInitialDir = L".\\";

        ofn.lpstrFilter = GetFilterMessage();
        ofn.nFilterIndex = 2;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_DONTADDTORECENT;
        ofn.lpstrDefExt = L"exe";

        if (!GetOpenFileNameW(&ofn) || !fileBuffer[0]) {
            return FALSE; // User canceled
        }

        outPath.assign(fileBuffer);
        return TRUE;
    }

    static std::wstring GetDirectoryFromPath(const std::wstring& path) {
        std::wstring::size_type pos = path.find_last_of(L"\\/");
        if (pos == std::wstring::npos) {
            return L"";
        }
        return path.substr(0, pos + 1); // include separator
    }

    static std::wstring MakeOrgBackupPath(const std::wstring& originalPath) {
        std::wstring result = originalPath;
        std::wstring::size_type posSlash = result.find_last_of(L"\\/");
        std::wstring::size_type posDot = result.find_last_of(L'.');

        if (posDot == std::wstring::npos || (posSlash != std::wstring::npos && posDot < posSlash)) {
            // No extension; append
            result.append(L".org");
        }
        else {
            result.erase(posDot);
            result.append(L".org");
        }
        return result;
    }

    static BYTE* RvaToPtr(DWORD rva, BYTE* base, IMAGE_SECTION_HEADER* sections, WORD numberOfSections, DWORD fileSize) {
        for (WORD i = 0; i < numberOfSections; ++i) {
            IMAGE_SECTION_HEADER& sec = sections[i];
            DWORD secVA = sec.VirtualAddress;
            DWORD secSize = sec.Misc.VirtualSize;
            DWORD rawSize = sec.SizeOfRawData;
            if (secSize == 0) {
                secSize = rawSize;
            }

            if (rva >= secVA && rva < secVA + secSize) {
                DWORD delta = rva - secVA;
                if (delta >= rawSize) {
                    return NULL;
                }

                DWORD fileOffset = sec.PointerToRawData + delta;
                if (fileOffset >= fileSize) {
                    return NULL;
                }

                return base + fileOffset;
            }
        }
        return NULL;
    }

    static PE_ERROR PatchWinmmImports(HANDLE hFile, BOOL isUnpatch, BOOL checkMode = FALSE) {
        LARGE_INTEGER liSize;
        if (!GetFileSizeEx(hFile, &liSize) || liSize.QuadPart <= 0) {
            return PE_GET_SIZE;
        }

        if (liSize.QuadPart > 0xFFFFFFFFULL) {
            return PE_LARGE_MAP;
        }
        DWORD fileSize = static_cast<DWORD>(liSize.QuadPart);

        HANDLE hMap = CreateFileMappingW(
            hFile,
            NULL,
            PAGE_READWRITE,
            0,
            0,
            NULL);
        if (!hMap) {
            return PE_CREATE_MAP;
        }

        BYTE* base = static_cast<BYTE*>(
            MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
        if (!base) {
            CloseHandle(hMap);
            return PE_FAIL_MAP;
        }

        PE_ERROR result = PE_NOERROR;

        do {
            if (fileSize < sizeof(IMAGE_DOS_HEADER)) {
                result = PE_SMALL_HEADER;
                break;
            }

            IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
                result = PE_MISSING_MZ;
                break;
            }

            if (dos->e_lfanew <= 0 ||
                static_cast<DWORD>(dos->e_lfanew) > fileSize - (sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER))) {
                result = PE_INVALID_OFFSET;
                break;
            }

            BYTE* ntBase = base + dos->e_lfanew;
            DWORD peSignature = *reinterpret_cast<DWORD*>(ntBase);
            if (peSignature != IMAGE_NT_SIGNATURE) {
                result = PE_MISSING_PE;
                break;
            }

            IMAGE_FILE_HEADER* fileHeader =
                reinterpret_cast<IMAGE_FILE_HEADER*>(ntBase + sizeof(DWORD));
            WORD sizeOfOptionalHeader = fileHeader->SizeOfOptionalHeader;

            if (dos->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + sizeOfOptionalHeader > fileSize) {
                result = PE_INVALID_OPTION;
                break;
            }

            BYTE* optBase = ntBase + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
            WORD magic = *reinterpret_cast<WORD*>(optBase);

            BOOL is64 = FALSE;
            IMAGE_OPTIONAL_HEADER32* opt32 = NULL;
            IMAGE_OPTIONAL_HEADER64* opt64 = NULL;

            if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                is64 = FALSE;
                opt32 = reinterpret_cast<IMAGE_OPTIONAL_HEADER32*>(optBase);
            }
            else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                is64 = TRUE;
                opt64 = reinterpret_cast<IMAGE_OPTIONAL_HEADER64*>(optBase);
            }
            else {
                result = PE_UNKNOWN_OPTION;
                break;
            }

            // Section headers
            IMAGE_SECTION_HEADER* sections =
                reinterpret_cast<IMAGE_SECTION_HEADER*>(optBase + sizeOfOptionalHeader);
            if (fileHeader->NumberOfSections == 0) {
                result = PE_NO_SECTIONS;
                break;
            }

            BYTE* sectionsEnd = reinterpret_cast<BYTE*>(&sections[fileHeader->NumberOfSections]);
            if (sectionsEnd > base + fileSize) {
                result = PE_OUT_OF_FILE;
                break;
            }

            // Import directory (rdata)
            DWORD importRva = 0;
            DWORD importSize = 0;

            if (!is64) {
                importRva = opt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
                importSize = opt32->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
            }
            else {
                importRva = opt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
                importSize = opt64->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
            }

            if (importRva == 0 || importSize == 0) {
                result = PE_NO_HAS_TARGET;
                break;
            }

            IMAGE_IMPORT_DESCRIPTOR* importDesc =
                reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
                    RvaToPtr(importRva, base, sections, fileHeader->NumberOfSections, fileSize));
            if (!importDesc) {
                result = PE_INVALID_TBL_PTR;
                break;
            }

            // Find WINMM.DLL (case-insensitive)
            IMAGE_IMPORT_DESCRIPTOR* winmmDesc = NULL;
            BOOL alreadyPatched = FALSE;
            char* baseModuleName = isUnpatch ? "_inmm.dll" : "winmm.dll";
            char* targetModuleName = isUnpatch ? "winmm.dll" : "_inmm.dll";

            for (IMAGE_IMPORT_DESCRIPTOR* desc = importDesc; desc->Name != 0; ++desc) {
                char* moduleName =
                    reinterpret_cast<char*>(
                        RvaToPtr(desc->Name, base, sections, fileHeader->NumberOfSections, fileSize));
                if (!moduleName) {
                    continue;
                }

                if (_stricmp(moduleName, baseModuleName) == 0) {
                    winmmDesc = desc;
                    break;
                }
                else if (_stricmp(moduleName, targetModuleName) == 0) {
                    alreadyPatched = TRUE;
                    break;
                }
            }

            if (!winmmDesc) {
                result = alreadyPatched ? PE_ALREADY_IMPORT : PE_NO_HAS_TARGET;
                break;
            }

            // Patch module name: WINMM.DLL -> _INMM.DLL
            {
                char* moduleName =
                    reinterpret_cast<char*>(
                        RvaToPtr(winmmDesc->Name, base, sections, fileHeader->NumberOfSections, fileSize));
                if (!moduleName) {
                    result = PE_MODULE_PTR;
                    break;
                }
                // only work not check mode
                if (!checkMode) {
                    moduleName[0] = isUnpatch ? 'W' : '_';
                }
            }

            // Patch imported function names from WINMM.DLL
            BOOL anyFuncPatched = FALSE;

            if (!is64) {
                DWORD thunkRva = winmmDesc->OriginalFirstThunk ? winmmDesc->OriginalFirstThunk : winmmDesc->FirstThunk;
                IMAGE_THUNK_DATA32* thunk =
                    reinterpret_cast<IMAGE_THUNK_DATA32*>(
                        RvaToPtr(thunkRva, base, sections, fileHeader->NumberOfSections, fileSize));

                if (!thunk) {
                    result = PE_IMPORT_TRUNKS;
                    break;
                }

                for (; thunk->u1.AddressOfData != 0; ++thunk) {
                    if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG32) {
                        // Import by ordinal, skip
                        continue;
                    }

                    IMAGE_IMPORT_BY_NAME* importByName =
                        reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                            RvaToPtr(thunk->u1.AddressOfData, base, sections, fileHeader->NumberOfSections, fileSize));
                    if (!importByName) {
                        continue;
                    }

                    char* funcName = reinterpret_cast<char*>(importByName->Name);
                    if (funcName && funcName[0] != '\0' && (isUnpatch ? funcName[0] == '_' : funcName[0] != '_')) {
                        // only work not check mode
                        if (ConvertFunctionName(funcName, isUnpatch, checkMode)) {
                            anyFuncPatched = TRUE;
                        }
#ifdef _DEBUG
                        else {
                            anyFuncPatched = FALSE;
                            result = PE_MODULE_NAME;
                            break;
                        }
#endif
                    }
                }
            }
            else {
                DWORD thunkRva = winmmDesc->OriginalFirstThunk ? winmmDesc->OriginalFirstThunk : winmmDesc->FirstThunk;
                IMAGE_THUNK_DATA64* thunk =
                    reinterpret_cast<IMAGE_THUNK_DATA64*>(
                        RvaToPtr(thunkRva, base, sections, fileHeader->NumberOfSections, fileSize));

                if (!thunk) {
                    result = PE_IMPORT_TRUNKS;
                    break;
                }

                for (; thunk->u1.AddressOfData != 0; ++thunk) {
                    if (thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64) {
                        // Import by ordinal, skip
                        continue;
                    }

                    IMAGE_IMPORT_BY_NAME* importByName =
                        reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(
                            RvaToPtr(static_cast<DWORD>(thunk->u1.AddressOfData),
                                base, sections, fileHeader->NumberOfSections, fileSize));
                    if (!importByName) {
                        continue;
                    }

                    char* funcName = reinterpret_cast<char*>(importByName->Name);
                    if (funcName && funcName[0] != '\0' && (isUnpatch ? funcName[0] == '_' : funcName[0] != '_')) {
                        // only work not check mode
                        if (ConvertFunctionName(funcName, isUnpatch, checkMode)) {
                            anyFuncPatched = TRUE;
                        }
#ifdef _DEBUG
                        else {
                            anyFuncPatched = FALSE;
                            result = PE_MODULE_NAME;
                            break;
                        }
#endif
                    }
                }
            }
#ifdef _DEBUG
            if (!anyFuncPatched) {
                result = PE_NO_HAS_FUNC;
                break;
            }
#endif
        } while (FALSE);

        // Unmap and close mapping
        UnmapViewOfFile(base);
        CloseHandle(hMap);

        return result;
    }

    static void HandleDllCopyLogic(const std::wstring& dirPath) {
        std::wstring inmmPath = dirPath + L"_inmm.dll";
        std::wstring winmmPath = dirPath + L"winmm.dll";

        DWORD attrInmm = GetFileAttributesW(inmmPath.c_str());
        DWORD attrWinmm = GetFileAttributesW(winmmPath.c_str());

        BOOL hasInmm = (attrInmm != INVALID_FILE_ATTRIBUTES);
        BOOL hasWinmm = (attrWinmm != INVALID_FILE_ATTRIBUTES);

        if (!hasInmm && !hasWinmm) {
            // Already checked earlier, but do nothing here
            return;
        }

        if (!hasInmm && hasWinmm) {
            // Only winmm.dll exists: copy to _inmm.dll
            CopyFileW(winmmPath.c_str(), inmmPath.c_str(), FALSE);
            return;
        }

        if (hasInmm && !hasWinmm) {
            // Only _inmm.dll exists: nothing to do
            return;
        }

        // Both winmm.dll and _inmm.dll exist:
        // If _inmm.dll is an original winmm.dll with higher version, bypass copy/rename.
        if (hasInmm && hasWinmm) {
            ULARGE_INTEGER verInmm;
            ULARGE_INTEGER verWinmm;
            BOOL origInmmIsWinmm = FALSE;
            BOOL origDummy = FALSE;

            if (GetFileVersionAndOriginalName(inmmPath, verInmm, origInmmIsWinmm) &&
                GetFileVersionAndOriginalName(winmmPath, verWinmm, origDummy)) {
                if (origInmmIsWinmm && verInmm.QuadPart > verWinmm.QuadPart) {
                    // Bypass when _inmm.dll is newer winmm.dll
                    return;
                }
            }
        }

        // check winmm.dll readability
        HANDLE hWin = CreateFileW(
            winmmPath.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (hWin == INVALID_HANDLE_VALUE) {
            // Failure: do not continue
            return;
        }
        CloseHandle(hWin);

        // try to delete _inmm.dll
        if (!DeleteFileW(inmmPath.c_str())) {
            // Failure: do not continue
            return;
        }

        // try to copy winmm.dll to _inmm.dll
        if (CopyFileW(winmmPath.c_str(), inmmPath.c_str(), FALSE)) {
            return;
        }

        // fallback: rename winmm.dll to _inmm.dll
        MoveFileW(winmmPath.c_str(), inmmPath.c_str());
    }
}

namespace WineCompat {
    void SelectFile() {
        // File selection dialog
        std::wstring path;
        if (!OpenExeOrDllDialog(path)) {
            // User canceled
            return;
        }

        std::wstring dir = GetDirectoryFromPath(path);
        if (dir.empty()) {
            ErrorMessageHandle(INVALID_PATH);
            return;
        }

        // Check read/write access and PE Header
        BOOL isUnpatch = FALSE;
        {
            HANDLE hFile = CreateFileW(
                path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                ErrorMessageHandle(FAILED_OPEN);
                return;
            }

            PE_ERROR ret = PatchWinmmImports(hFile, FALSE, TRUE);

            if (ret != PE_NOERROR) {
                if (!ErrorMessageHandle(ret)) {
                    CloseHandle(hFile);
                    return;
                }
                ret = PatchWinmmImports(hFile, TRUE, TRUE);
                if (ret != PE_NOERROR) {
                    ErrorMessageHandle((ret <= PE_IMPORT_TRUNKS) ? ret : PE_ERR_UNKNOWN);
                    CloseHandle(hFile);
                    return;
                }
                isUnpatch = TRUE;
            }

            CloseHandle(hFile);
        }

        // Check _inmm.dll or winmm.dll existence when patch mode
        if (!isUnpatch) {
            std::wstring inmmPath = dir + L"_inmm.dll";
            std::wstring winmmPath = dir + L"winmm.dll";

            DWORD attrInmm = GetFileAttributesW(inmmPath.c_str());
            DWORD attrWinmm = GetFileAttributesW(winmmPath.c_str());

            BOOL hasInmm = (attrInmm != INVALID_FILE_ATTRIBUTES);
            BOOL hasWinmm = (attrWinmm != INVALID_FILE_ATTRIBUTES);

            if (!hasInmm && !hasWinmm && !ErrorMessageHandle(DLL_NOT_FOUND)) {
                return;
            }
            else {
                // Copy or rename winmm.dll to _inmm.dll according to rules
                HandleDllCopyLogic(dir);
            }

            // Create .org backup
            std::wstring backupPath = MakeOrgBackupPath(path);
            BOOL backupFile = TRUE;
            if (GetFileAttributesW(backupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                HANDLE hFile1 = CreateFileW(
                    path.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (hFile1 == INVALID_HANDLE_VALUE) {
                    goto APPEND_TIME;
                }
                HANDLE hFile2 = CreateFileW(
                    backupPath.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (hFile2 == INVALID_HANDLE_VALUE) {
                    CloseHandle(hFile1);
                    goto APPEND_TIME;
                }
                backupFile = !CompareFileHandle(hFile1, hFile2);
                CloseHandle(hFile1);
                CloseHandle(hFile2);

            APPEND_TIME:
                if (backupFile) {
                    backupPath.append(L"~");
                    backupPath.append(getNowFileTime());
                }
            }

            if (backupFile && !CopyFileW(path.c_str(), backupPath.c_str(), TRUE) && !ErrorMessageHandle(FAILED_BACKUP)) {
                return;
            }
        }

        // Open file for read/write
        HANDLE hFile = CreateFileW(
            path.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);
        if (hFile == INVALID_HANDLE_VALUE) {
            ErrorMessageHandle(FAILED_OPEN);
            return;
        }

        // Parse PE and patch WINMM.DLL imports
        PE_ERROR ret = PatchWinmmImports(hFile, isUnpatch);
        CloseHandle(hFile);

        // Success or fail message
        if (ret == PE_NOERROR) {
            ErrorMessageHandle(isUnpatch ? SUCCESS_RECOVER : SUCCESS_CONVERT);
        }
        else {
            ErrorMessageHandle((ret <= PE_IMPORT_TRUNKS) ? ret : PE_ERR_UNKNOWN);
        }
    }
}
