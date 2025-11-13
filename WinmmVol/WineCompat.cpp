#include "WineCompat.h"
#include "UiLang.h"
#include <shlobj.h>
#include <shellapi.h>
#include <commdlg.h>
#include <winnt.h>
#include <string>

#pragma comment(lib, "shell32.lib")

// Simple message helpers
namespace {
    const wchar_t* getNowFileTime() {
        static wchar_t time[17]{ 0, };
        FILETIME ftNow; GetSystemTimeAsFileTime(&ftNow);
        wsprintfW(time, L"%08x%08x", ftNow.dwHighDateTime, ftNow.dwLowDateTime);
        return time;
    }

    void ShowPEError(int errNo) {
        std::wstring tmp, label = UILang::Get(L"ERR_PE_INTERNAL", tmp);
        wsprintfW(&label[0], tmp.c_str(), errNo);
        MessageBoxW(NULL, label.c_str(), L"WineCompat", MB_ICONERROR | MB_OK);
    }

    void ShowMessage(LPCWSTR text, BOOL isError) {
        std::wstring tmp;
        MessageBoxW(NULL, UILang::Get(text, tmp), L"WineCompat", (isError ? MB_ICONERROR : MB_ICONINFORMATION) | MB_OK);
    }

    wchar_t* GetFilterMessage() {
        // static buffer for OPENFILENAME filter string
        static wchar_t buffer[256];
        static bool initialized = false;

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
                initialized = true;                            \
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
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.*");
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        // <exeName> (*.exe)\0*.exe\0
        APPEND_STR_W(exeName);
        APPEND_STR_W(L" (*.exe)");
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.exe");
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        // <dllName> (*.dll)\0*.dll\0
        APPEND_STR_W(dllName);
        APPEND_STR_W(L" (*.dll)");
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        APPEND_STR_W(L"*.dll");
        if (remain == 0) { buffer[0] = L'\0'; initialized = true; return buffer; }
        *p++ = L'\0';
        --remain;

        // final double-null terminator
        if (remain == 0) {
            buffer[(sizeof(buffer) / sizeof(buffer[0])) - 1] = L'\0';
        }
        else {
            *p++ = L'\0';
        }

        initialized = true;
        return buffer;

#undef APPEND_STR_W
    }

    bool OpenExeOrDllDialog(std::wstring& outPath) {

        wchar_t fileBuffer[MAX_PATH] = { 0 };

        OPENFILENAMEW ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFile = fileBuffer;
        ofn.nMaxFile = MAX_PATH;

        ofn.lpstrFilter = GetFilterMessage();
        ofn.nFilterIndex = 2;
        ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
        ofn.lpstrDefExt = L"exe";

        if (!GetOpenFileNameW(&ofn)) {
            return false; // User canceled
        }

        outPath.assign(fileBuffer);
        return true;
    }

    std::wstring GetDirectoryFromPath(const std::wstring& path) {
        std::wstring::size_type pos = path.find_last_of(L"\\/");
        if (pos == std::wstring::npos) {
            return L"";
        }
        return path.substr(0, pos + 1); // include separator
    }

    std::wstring MakeOrgBackupPath(const std::wstring& originalPath) {
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

    BYTE* RvaToPtr(DWORD rva, BYTE* base, IMAGE_SECTION_HEADER* sections, WORD numberOfSections, DWORD fileSize) {
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

    bool PatchWinmmImports(HANDLE hFile) {
        LARGE_INTEGER liSize;
        if (!GetFileSizeEx(hFile, &liSize) || liSize.QuadPart <= 0) {
            // Failed to get file size.
            ShowPEError(1);
            return false;
        }

        if (liSize.QuadPart > 0xFFFFFFFFULL) {
            // File is too large to map.
            ShowPEError(2);
            return false;
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
            // Failed to create file mapping.
            ShowPEError(3);
            return false;
        }

        BYTE* base = static_cast<BYTE*>(
            MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
        if (!base) {
            CloseHandle(hMap);
            // Failed to map file.
            ShowPEError(4);
            return false;
        }

        bool result = false;

        do {
            if (fileSize < sizeof(IMAGE_DOS_HEADER)) {
                // File is too small to contain a valid PE header.
                ShowPEError(5);
                break;
            }

            IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
                // Selected file is not a valid PE (missing MZ header).
                ShowPEError(6);
                break;
            }

            if (dos->e_lfanew <= 0 ||
                static_cast<DWORD>(dos->e_lfanew) > fileSize - (sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER))) {
                // Invalid PE header offset.
                ShowPEError(7);
                break;
            }

            BYTE* ntBase = base + dos->e_lfanew;
            DWORD peSignature = *reinterpret_cast<DWORD*>(ntBase);
            if (peSignature != IMAGE_NT_SIGNATURE) {
                // Selected file is not a valid PE (missing PE signature).
                ShowPEError(8);
                break;
            }

            IMAGE_FILE_HEADER* fileHeader =
                reinterpret_cast<IMAGE_FILE_HEADER*>(ntBase + sizeof(DWORD));
            WORD sizeOfOptionalHeader = fileHeader->SizeOfOptionalHeader;

            if (dos->e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + sizeOfOptionalHeader > fileSize) {
                // PE optional header is truncated or invalid.
                ShowPEError(9);
                break;
            }

            BYTE* optBase = ntBase + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER);
            WORD magic = *reinterpret_cast<WORD*>(optBase);

            bool is64 = false;
            IMAGE_OPTIONAL_HEADER32* opt32 = NULL;
            IMAGE_OPTIONAL_HEADER64* opt64 = NULL;

            if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                is64 = false;
                opt32 = reinterpret_cast<IMAGE_OPTIONAL_HEADER32*>(optBase);
            }
            else if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                is64 = true;
                opt64 = reinterpret_cast<IMAGE_OPTIONAL_HEADER64*>(optBase);
            }
            else {
                // Unknown PE optional header type.
                ShowPEError(10);
                break;
            }

            // Section headers
            IMAGE_SECTION_HEADER* sections =
                reinterpret_cast<IMAGE_SECTION_HEADER*>(optBase + sizeOfOptionalHeader);
            if (fileHeader->NumberOfSections == 0) {
                // PE file has no sections.
                ShowPEError(11);
                break;
            }

            BYTE* sectionsEnd = reinterpret_cast<BYTE*>(&sections[fileHeader->NumberOfSections]);
            if (sectionsEnd > base + fileSize) {
                // PE section headers are out of file bounds.
                ShowPEError(12);
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
                // Import table (rdata) is not present. No WINMM.DLL imports found.
                ShowMessage(L"INFO_PE_INTERNAL", FALSE);
                break;
            }

            IMAGE_IMPORT_DESCRIPTOR* importDesc =
                reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
                    RvaToPtr(importRva, base, sections, fileHeader->NumberOfSections, fileSize));
            if (!importDesc) {
                // Import table pointer is invalid.
                ShowPEError(13);
                break;
            }

            // Find WINMM.DLL (case-insensitive)
            IMAGE_IMPORT_DESCRIPTOR* winmmDesc = NULL;
            BOOL alreadyPatched = FALSE;

            for (IMAGE_IMPORT_DESCRIPTOR* desc = importDesc; desc->Name != 0; ++desc) {
                char* moduleName =
                    reinterpret_cast<char*>(
                        RvaToPtr(desc->Name, base, sections, fileHeader->NumberOfSections, fileSize));
                if (!moduleName) {
                    continue;
                }

                if (_stricmp(moduleName, "winmm.dll") == 0) {
                    winmmDesc = desc;
                    break;
                }
                else if (_stricmp(moduleName, "_inmm.dll") == 0) {
                    alreadyPatched = TRUE;
                    break;
                }
            }

            if (!winmmDesc) {
                ShowMessage((alreadyPatched ? L"INFO_PE_ALREADY_IMPORT" : L"INFO_PE_INTERNAL"), FALSE);
                break;
            }

            // Patch module name: WINMM.DLL -> _INMM.DLL
            {
                char* moduleName =
                    reinterpret_cast<char*>(
                        RvaToPtr(winmmDesc->Name, base, sections, fileHeader->NumberOfSections, fileSize));
                if (!moduleName) {
                    // Failed to resolve module name pointer.
                    ShowPEError(14);
                    break;
                }
                moduleName[0] = '_';
            }

            // Patch imported function names from WINMM.DLL
            bool anyFuncPatched = false;

            if (!is64) {
                DWORD thunkRva = winmmDesc->OriginalFirstThunk ? winmmDesc->OriginalFirstThunk : winmmDesc->FirstThunk;
                IMAGE_THUNK_DATA32* thunk =
                    reinterpret_cast<IMAGE_THUNK_DATA32*>(
                        RvaToPtr(thunkRva, base, sections, fileHeader->NumberOfSections, fileSize));

                if (!thunk) {
                    // Failed to resolve import thunks.
                    ShowPEError(15);
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
                    if (funcName && funcName[0] != '\0' && funcName[0] != '_') {
                        funcName[0] = '_'; // mciSendCommandA -> _ciSendCommandA
                        anyFuncPatched = true;
                    }
                }
            }
            else {
                DWORD thunkRva = winmmDesc->OriginalFirstThunk ? winmmDesc->OriginalFirstThunk : winmmDesc->FirstThunk;
                IMAGE_THUNK_DATA64* thunk =
                    reinterpret_cast<IMAGE_THUNK_DATA64*>(
                        RvaToPtr(thunkRva, base, sections, fileHeader->NumberOfSections, fileSize));

                if (!thunk) {
                    // Failed to resolve import thunks.
                    ShowPEError(16);
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
                    if (funcName && funcName[0] != '\0' && funcName[0] != '_') {
                        funcName[0] = '_';
                        anyFuncPatched = true;
                    }
                }
            }

            if (!anyFuncPatched) {
                // Not a hard error, but inform user
                // WINMM.DLL is imported, but no function names were patched (imports may be by ordinal).
                ShowMessage(L"INFO_PE_INTERNAL", FALSE);
                break;
            }

            // If we reached here, patching is considered successful
            result = true;
        } while (false);

        // Unmap and close mapping
        UnmapViewOfFile(base);
        CloseHandle(hMap);

        return result;
    }

    void HandleDllCopyLogic(const std::wstring& dirPath) {
        std::wstring inmmPath = dirPath + L"_inmm.dll";
        std::wstring winmmPath = dirPath + L"winmm.dll";

        DWORD attrInmm = GetFileAttributesW(inmmPath.c_str());
        DWORD attrWinmm = GetFileAttributesW(winmmPath.c_str());

        bool hasInmm = (attrInmm != INVALID_FILE_ATTRIBUTES);
        bool hasWinmm = (attrWinmm != INVALID_FILE_ATTRIBUTES);

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
            ShowMessage(L"ERR_INVALID_PATH", TRUE);
            return;
        }

        std::wstring inmmPath = dir + L"_inmm.dll";
        std::wstring winmmPath = dir + L"winmm.dll";

        // Check _inmm.dll or winmm.dll existence
        DWORD attrInmm = GetFileAttributesW(inmmPath.c_str());
        DWORD attrWinmm = GetFileAttributesW(winmmPath.c_str());

        bool hasInmm = (attrInmm != INVALID_FILE_ATTRIBUTES);
        bool hasWinmm = (attrWinmm != INVALID_FILE_ATTRIBUTES);

        if (!hasInmm && !hasWinmm) {
            ShowMessage(L"ERR_DLL_NOT_FOUND", TRUE);
            return;
        }

        // Check read/write access
        {
            HANDLE hCheck = CreateFileW(
                path.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (hCheck == INVALID_HANDLE_VALUE) {
                // Selected file cannot be opened with read/write access.
                ShowMessage(L"ERR_FAIL_OPEN", TRUE);
                return;
            }
            CloseHandle(hCheck);
        }

        // Create .org backup
        std::wstring backupPath = MakeOrgBackupPath(path);
        if (GetFileAttributesW(backupPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            backupPath.append(L"~");
            backupPath.append(getNowFileTime());
        }

        if (!CopyFileW(path.c_str(), backupPath.c_str(), TRUE)) {
            ShowMessage(L"ERR_FAIL_BACKUP", TRUE);
            return;
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
            // Failed to reopen selected file for modification.
            ShowMessage(L"ERR_FAIL_OPEN", TRUE);
            DeleteFileW(backupPath.c_str());
            return;
        }

        // Parse PE and patch WINMM.DLL imports
        bool patched = PatchWinmmImports(hFile);
        CloseHandle(hFile);

        if (!patched) {
            // Error or info message already shown inside PatchWinmmImports
            DeleteFileW(backupPath.c_str());
            return;
        }

        // Copy or rename winmm.dll to _inmm.dll according to rules
        HandleDllCopyLogic(dir);

        // Success message
        ShowMessage(L"INFO_SUCCESS_MODIFY", FALSE);
    }
}
