#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <commctrl.h> // For InitCommonControlsEx

#include "resource.h"
#include "RegistryManager.h"
#include "TrayIcon.h"
#include "VolumeSlider.h"
#include "AboutDialog.h"
#include "UiLang.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Shell32.lib")

// --- Configuration (from DLL context) ---
const wchar_t* const MUTEX_NAME = L"WinMM-Stubs Volume Control";
const wchar_t* const MAIN_WINDOW_CLASS = L"WinMMStubsMainMsgWindowClass";

// Custom message from DLL
constexpr UINT WM_EXIT_APP = WM_APP + 1;
constexpr UINT WM_UPDATE_APP = WM_APP + 2;
constexpr UINT WM_SHOW_SLIDER = WM_APP + 3;
constexpr UINT WM_SET_LIVEAPP = WM_APP + 4;

// Global variables
static HINSTANCE g_hInstance = NULL;
static HWND g_hMainWnd = NULL;
static HWND g_hSliderWnd = NULL;
static UINT g_shellCreated = 0;
static BOOL g_connected = FALSE;
static BOOL g_runFirst = FALSE;

// Main Message Window Procedure
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        // Load icon and create tray icon
    {
        // Try to load a specific small icon size for the tray
        HICON hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDI_TRAYICON_LVL3), IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
        // Fallback to default size if specific size loading fails
        if (!hIcon) hIcon = LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_TRAYICON_LVL3));

        if (hIcon) {
            TrayIcon::Create(hwnd, WM_TRAYICON, hIcon, L"WinMM Stubs Volume");
        }

        // Create the slider window (hidden initially)
        g_hSliderWnd = VolumeSlider::Create(g_hInstance, hwnd);
        g_shellCreated = RegisterWindowMessageW(L"TaskbarCreated");

        SetTimer(hwnd, 1, 100, NULL);
        break;
    }

    case WM_SET_LIVEAPP:
        if (wParam) {
            std::wstring guid;
            if (RegistryManager::GetGuidByPid((DWORD)wParam, guid)) {
                VolumeSlider::SetCurrentGuid(guid);
            }
        }
    case WM_UPDATE_APP:
    case WM_TIMER:
        if ((message != WM_TIMER) || (wParam == 1)) {
            RegistryManager::SweepAndInvalidateDeadPids();
            const BOOL hasLive = RegistryManager::HasAnyLiveApp();

            if (!g_connected) {
                if (hasLive) {
                    g_connected = TRUE;
                    TrayIcon::RefreshForTarget(VolumeSlider::GetCurrentGuid());
                }
                else if (!g_runFirst) {
                    g_runFirst = TRUE;
                    SetTimer(hwnd, 1, 3000, NULL);
                    goto SHOW_SLIDER;
                }
            }
            else if (!hasLive) {
                DestroyWindow(hwnd);
                break;
            }
            if (!g_runFirst){
                g_runFirst = TRUE;
                SetTimer(hwnd, 1, 3000, NULL);
            }
            VolumeSlider::EnsureSelectionLiveOrGlobal();
        }
        break;

    case WM_SHOW_SLIDER:
        TrayIcon::SetShowAllApps(FALSE);
        goto SHOW_SLIDER;

    case WM_TRAYICON: // Message from the tray icon
        switch (LOWORD(lParam)) { // Notification event
        case WM_LBUTTONUP:
        case NIN_SELECT: // Select event (e.g., Enter key)
        SHOW_SLIDER:
        {
            // Show volume slider
            POINT pt;
            GetCursorPos(&pt);
            VolumeSlider::Show(g_hSliderWnd, pt);
        }
        break;
        case WM_RBUTTONUP:
            // Show context menu
            TrayIcon::ShowContextMenu(hwnd);
            break;
        case WM_CONTEXTMENU: // Context menu event
            break;
        }
        break;

    case WM_COMMAND: // Menu item selected
        switch (LOWORD(wParam)) {
        case IDM_INFO_USAGE:
            AboutDialog::ShowUsage(g_hInstance, hwnd);
            break;
        case IDM_INFO_LICENSE:
            AboutDialog::ShowLicense(g_hInstance, hwnd);
            break;
        case IDM_MODE_CLEAR:
            RegistryManager::ResetAllSettings();
            VolumeSlider::SelectGlobal();
            VolumeSlider::UpdateDisplay(g_hSliderWnd);
            break;
        case IDM_SHOW_ALL_APPS:
            TrayIcon::SetShowAllApps(!TrayIcon::GetShowAllApps());
            VolumeSlider::UpdateDisplay(g_hSliderWnd);
            break;
        case IDM_EXIT:
            DestroyWindow(hwnd); // Trigger WM_DESTROY and exit message loop
            break;
        }
        break;

    case WM_EXIT_APP: // Custom message from DLL
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        TrayIcon::Destroy(hwnd);
        // Do not call RegistryManager::Close here, it's called by the main entry point
        PostQuitMessage(0); // Terminate the message loop
        break;

    default:
        if (message == g_shellCreated) {
            HICON hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCEW(IDI_TRAYICON_LVL3), IMAGE_ICON,
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
            if (!hIcon) hIcon = LoadIconW(g_hInstance, MAKEINTRESOURCEW(IDI_TRAYICON_LVL3));

            if (hIcon) {
                TrayIcon::Create(hwnd, WM_TRAYICON, hIcon, L"WinMM Stubs Volume");
            }
        }
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

// WinMain Entry Point
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    g_hInstance = hInstance;
    UILang::Init(hInstance);

    // Check for previous instance using Mutex
    HANDLE hMutex = CreateMutexW(NULL, TRUE, MUTEX_NAME);
    if (hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex) CloseHandle(hMutex);
#ifdef _DEBUG
        MessageBoxW(NULL, L"WinMM Stubs Volume Control is already running.", L"Information", MB_OK | MB_ICONINFORMATION);
#endif
        HWND hwnd = FindWindowW(MAIN_WINDOW_CLASS, NULL);
        if (hwnd) PostMessageW(hwnd, WM_SHOW_SLIDER, 0, 0);
        return 0;
    }

    // Initialize Common Controls (for Trackbar)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_LINK_CLASS; // Trackbar control
    InitCommonControlsEx(&icex);

    // Initialize Registry Manager
    if (!RegistryManager::Initialize()) {
#ifdef _DEBUG
        MessageBoxW(NULL, L"Failed to initialize registry access.", L"Error", MB_OK | MB_ICONERROR);
#endif
        CloseHandle(hMutex);
        return 1;
    }

    // Register Window Classes
    WNDCLASSEXW wcex = { sizeof(wcex) };
    wcex.style = 0;
    wcex.lpfnWndProc = MainWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    // Load main window icon using LoadIcon for default size (e.g., taskbar)
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = MAIN_WINDOW_CLASS;
    // Small icon for title bar
    wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));

    if (!RegisterClassExW(&wcex)) {
#ifdef _DEBUG
        MessageBoxW(NULL, L"Failed to register main window class.", L"Error", MB_OK | MB_ICONERROR);
#endif
        RegistryManager::Close();
        CloseHandle(hMutex);
        return 1;
    }

    if (!VolumeSlider::RegisterWindowClass(hInstance)) {
#ifdef _DEBUG
        MessageBoxW(NULL, L"Failed to register slider window class.", L"Error", MB_OK | MB_ICONERROR);
#endif
        RegistryManager::Close();
        CloseHandle(hMutex);
        return 1;
    }

    // Create the Main Hidden Window
    g_hMainWnd = CreateWindowExW(0, MAIN_WINDOW_CLASS, L"WinMM Stubs Hidden Main", 0,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
        HWND_MESSAGE, // Message-only window
        NULL, hInstance, NULL);

    if (!g_hMainWnd) {
#ifdef _DEBUG
        MessageBoxW(NULL, L"Failed to create main window.", L"Error", MB_OK | MB_ICONERROR);
#endif
        RegistryManager::Close();
        CloseHandle(hMutex);
        return 1;
    }

    // Message Loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    RegistryManager::Close(); // Ensure registry key is closed
    CloseHandle(hMutex); // Release the mutex

    return (int)msg.wParam;
}
