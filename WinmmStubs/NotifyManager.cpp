#include "NotifyManager.h"
#include "DeviceInfo.h"
#include <chrono>

// External dependency: Audio Engine status
namespace AudioEngine {
    BOOL IsPlaying();
    BOOL HasReachedEnd();
}

namespace {
    // --- Internal State ---
    enum JobType {
        JOB_NONE,
        JOB_PLAY_POLL,   // For RegisterPlaybackNotify
        JOB_LAZY_NOTIFY  // For LazyNotify
    };

    enum PollState {
        POLL_STATE_NONE,
        POLL_STATE_WAITING_100MS, // Initial 100ms delay
        POLL_STATE_POLLING        // Actively polling AudioEngine
    };

    // State for the *single* asynchronous notification job
    struct NotifyState {
        BOOL        isActive;
        MCIDEVICEID deviceId;
        HWND        hwnd;
        DWORD       openingThreadId; // Thread ID from the DeviceContext
        JobType     jobType;
        PollState   pollState;
        DWORD       lazyNotifyCode; // Code for LAZY_NOTIFY
        std::chrono::steady_clock::time_point nextCheckTime; // When to next wake up
    };

    static CRITICAL_SECTION gCs;
    static BOOL gInited = FALSE;
    static NotifyState gNotifyState; // The single active job

    // Worker thread for asynchronous notifications
    static HANDLE gWorkerThread = NULL;
    static HANDLE gQuitEvent = NULL;
    static HANDLE gWakeEvent = NULL; // Event to wake the worker immediately

    // --- Internal Helpers ---

    /**
     * @brief Posts the MM_MCINOTIFY message to the correct thread and HWND.
     * This function retrieves the original request ID and thread ID from DeviceInfo.
     */
    static void PostNotifyMessage(HWND hwnd, MCIDEVICEID devId, DWORD code) {
        DeviceContext* table = DeviceInfo::FindByDeviceID(devId);
        if (!table) return;

        MCIDEVICEID requestId = table->requestId; // Use the ID from the original request

        dprintf("MCI_NOTIFY_%s, hwnd=0x%08X, Devid=0x%04X", (code == 1) ? "SUCCESSFUL" : ((code == 2) ? "SUPERSEDED" : ((code == 4) ? "ABORTED" : "FAILURE")), hwnd, requestId);

        // Post to the thread that made the opening call
        // PostThreadMessageW(threadID, MM_MCINOTIFY, code, (LPARAM)requestId);

        // Also post to the specific HWND provided in the notify request
        if (hwnd) {
            PostMessageW(hwnd, MM_MCINOTIFY, code, (LPARAM)requestId);
        }
    }

    /**
     * @brief Clears the active notification state. MUST be called inside gCs.
     */
    static void ClearNotifyState() {
        gNotifyState.isActive = FALSE;
        gNotifyState.jobType = JOB_NONE;
        gNotifyState.pollState = POLL_STATE_NONE;
        gNotifyState.hwnd = NULL;
        gNotifyState.deviceId = 0;
    }

    /**
     * @brief Aborts the currently active async job, optionally sending a notification.
     * MUST be called inside gCs.
     * @param sendAborted If TRUE, sends MCI_NOTIFY_ABORTED.
     */
    static void SupersedeActiveJob(BOOL sendAborted) {
        if (!gNotifyState.isActive) return;

        LeaveCriticalSection(&gCs);
        NotifyState oldState = gNotifyState; // Copy state for posting
        ClearNotifyState();

        if (sendAborted && oldState.hwnd) {
            // Post message *outside* the lock
            PostNotifyMessage(oldState.hwnd, oldState.deviceId, MCI_NOTIFY_ABORTED);
        }
        EnterCriticalSection(&gCs);
    }

    /**
     * @brief Worker thread's main job handler. MUST be called inside gCs.
     */
    static void HandleJob(const std::chrono::steady_clock::time_point& now) {
        if (!gNotifyState.isActive) return;

        NotifyState stateToNotify = gNotifyState; // Copy for safe posting
        BOOL doPost = FALSE;
        DWORD postCode = 0;

        switch (gNotifyState.jobType) {
        case JOB_PLAY_POLL:
            if (gNotifyState.pollState == POLL_STATE_WAITING_100MS) {
                // This is the first check after 100ms
                if (!AudioEngine::IsPlaying()) {
                    // Playback is not active. Check if it finished or failed.
                    if (AudioEngine::HasReachedEnd()) {
                        // It's a short sound that finished *before* 100ms.
                        doPost = TRUE;
                        postCode = MCI_NOTIFY_SUCCESSFUL;
                    }
                    else {
                        // It never started (or stopped abnormally).
                        doPost = TRUE;
                        postCode = MCI_NOTIFY_FAILURE;
                    }
                    ClearNotifyState();
                }
                else if (!PostThreadMessageW(gNotifyState.openingThreadId, WM_USER, NULL, NULL)) {
                    dprintf("Thread Terminated.");
                    ClearNotifyState();
                }
                else {
                    // Playback started successfully, move to polling state
                    gNotifyState.pollState = POLL_STATE_POLLING;
                    gNotifyState.nextCheckTime = now + std::chrono::milliseconds(100);
                }
            }
            else if (gNotifyState.pollState == POLL_STATE_POLLING) {
                // We are polling for completion
                if (!AudioEngine::IsPlaying()) {
                    // Playback finished
                    doPost = TRUE;
                    postCode = MCI_NOTIFY_SUCCESSFUL;
                    ClearNotifyState();
                }
                else if (!PostThreadMessageW(gNotifyState.openingThreadId, WM_USER, NULL, NULL)) {
                    dprintf("Thread Terminated.");
                    ClearNotifyState();
                }
                else {
                    // Not finished, check again in 100ms
                    gNotifyState.nextCheckTime = now + std::chrono::milliseconds(100);
                }
            }
            break;

        case JOB_LAZY_NOTIFY:
            // Time is up, post the delayed message
            doPost = TRUE;
            postCode = gNotifyState.lazyNotifyCode;
            ClearNotifyState();
            break;
        }

        if (doPost) {
            // Post the message *outside* the lock
            LeaveCriticalSection(&gCs);
            PostNotifyMessage(stateToNotify.hwnd, stateToNotify.deviceId, postCode);
            EnterCriticalSection(&gCs);
        }
    }

    /**
     * @brief Main worker thread procedure for handling async notifications.
     */
    static DWORD WINAPI WorkerProc(LPVOID) {
        HANDLE hEvents[2] = { gQuitEvent, gWakeEvent };

        while (TRUE) {
            DWORD timeout_ms = INFINITE;

            EnterCriticalSection(&gCs);

            if (gNotifyState.isActive) {
                auto now = std::chrono::steady_clock::now();
                if (gNotifyState.nextCheckTime <= now) {
                    // Job is ready, handle it
                    HandleJob(now);
                }

                // If job is *still* active, calculate next timeout
                if (gNotifyState.isActive) {
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        gNotifyState.nextCheckTime - now
                    );
                    timeout_ms = (0 < duration.count()) ? duration.count() : 0;
                }
            }

            LeaveCriticalSection(&gCs);

            // Wait for quit, wake, or timeout
            DWORD wr = WaitForMultipleObjects(2, hEvents, FALSE, timeout_ms);

            if (wr == WAIT_OBJECT_0) {
                break; // QuitEvent
            }
            if (wr == WAIT_OBJECT_0 + 1) {
                continue; // WakeEvent (state changed)
            }
            if (wr == WAIT_TIMEOUT) {
                continue; // Job is ready
            }
        }
        return 0;
    }
} // namespace

namespace NotifyManager {
    BOOL Initialize(void) {
        if (gInited) return TRUE;
        InitializeCriticalSection(&gCs);
        ZeroMemory(&gNotifyState, sizeof(gNotifyState));
        gQuitEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        gWakeEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
        gWorkerThread = CreateThread(NULL, 0, WorkerProc, NULL, 0, NULL);
        gInited = TRUE;
        return TRUE;
    }

    void Shutdown(void) {
        if (!gInited) return;

        SetEvent(gQuitEvent);
        if (gWorkerThread) {
            WaitForSingleObject(gWorkerThread, 3000);
            CloseHandle(gWorkerThread);
            gWorkerThread = NULL;
        }

        if (gQuitEvent) { CloseHandle(gQuitEvent); gQuitEvent = NULL; }
        if (gWakeEvent) { CloseHandle(gWakeEvent); gWakeEvent = NULL; }

        DeleteCriticalSection(&gCs);
        gInited = FALSE;
    }

    void RegisterPlaybackNotify(MCIDEVICEID devId, HWND hwndCallback) {
        if (!gInited) Initialize();

        EnterCriticalSection(&gCs);

        // Supersede any existing job
        SupersedeActiveJob(TRUE);

        // Register the new job
        gNotifyState.isActive = TRUE;
        gNotifyState.deviceId = devId;
        gNotifyState.hwnd = hwndCallback;
        gNotifyState.jobType = JOB_PLAY_POLL;
        gNotifyState.pollState = POLL_STATE_WAITING_100MS; // Start in 100ms wait state
        gNotifyState.nextCheckTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);

        DeviceContext* ctx = DeviceInfo::FindByDeviceID(devId);
        gNotifyState.openingThreadId = ctx ? ctx->hOpeningThreadId : 0;

        LeaveCriticalSection(&gCs);

        // Wake the worker to acknowledge the new job
        SetEvent(gWakeEvent);
    }

    MMRESULT WaitPlayback(MCIDEVICEID devId, HWND hwndCallback, BOOL hasNotify) {
        if (!gInited) Initialize();

        EnterCriticalSection(&gCs);
        SupersedeActiveJob(TRUE);
        LeaveCriticalSection(&gCs);

        // Failed to start, return failure (no notify)
        if (!AudioEngine::IsPlaying()) {
            return MCIERR_INTERNAL;
        }

        // Blocking poll loop
        while (AudioEngine::IsPlaying()) {
            Sleep(50);
        }

        // Playback finished
        // MCI_NOTIFY_SUCCESSFUL message post after 100ms.
        if (hasNotify) {
            LazyNotify(devId, hwndCallback, MCI_NOTIFY_SUCCESSFUL);
        }

        return MMSYSERR_NOERROR; // Return success
    }

    void UnregisterPlaybackNotify(MCIDEVICEID devId) {
        if (!gInited) return;

        // MCI_NOTIFY_ABORTED message post if previously registered PlaybackNotify exists (immediately)
        EnterCriticalSection(&gCs);
        SupersedeActiveJob(TRUE);
        LeaveCriticalSection(&gCs);

        // Wake worker so it sees the job is gone
        SetEvent(gWakeEvent);
    }

    void LazyNotify(MCIDEVICEID devId, HWND hwndCallback, DWORD notifyCode) {
        if (!gInited) Initialize();

        EnterCriticalSection(&gCs);

        // Supersede any existing job
        NotifyState oldState = gNotifyState; // Copy state for posting
        ClearNotifyState();

        if (oldState.hwnd) {
            // Post message *outside* the lock
            LeaveCriticalSection(&gCs);
            dprintf("MCI_NOTIFY_SUPERSEDED, hwnd=0x%08X, Devid=0x%04X", oldState.hwnd, oldState.deviceId);
            PostNotifyMessage(oldState.hwnd, oldState.deviceId, MCI_NOTIFY_SUPERSEDED);
            EnterCriticalSection(&gCs);
        }

        // Register the new lazy notify job
        gNotifyState.isActive = TRUE;
        gNotifyState.deviceId = devId;
        gNotifyState.hwnd = hwndCallback;
        gNotifyState.jobType = JOB_LAZY_NOTIFY;
        gNotifyState.pollState = POLL_STATE_NONE;
        gNotifyState.lazyNotifyCode = notifyCode;
        gNotifyState.nextCheckTime = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);

        DeviceContext* ctx = DeviceInfo::FindByDeviceID(devId);
        gNotifyState.openingThreadId = ctx ? ctx->hOpeningThreadId : 0;

        LeaveCriticalSection(&gCs);

        // Wake the worker
        SetEvent(gWakeEvent);
    }

} // namespace NotifyManager
