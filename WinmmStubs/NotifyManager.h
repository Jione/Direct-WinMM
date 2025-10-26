#pragma once
#include "GlobalDefinitions.h"

namespace NotifyManager {
    // Initialize/Shutdown (Called at DLL attach/detach or first OPEN/last CLOSE)
    BOOL Initialize(void);
    void Shutdown(void);

    /**
     * @brief Registers an asynchronous notification for playback completion (for MCI_PLAY | MCI_NOTIFY).
     * @param devId The device ID.
     * @param hwndCallback The window handle for the notification.
     */
    void RegisterPlaybackNotify(MCIDEVICEID devId, HWND hwndCallback);

    /**
     * @brief Waits (blocks) for playback to complete (for MCI_PLAY | MCI_WAIT).
     * @param devId The device ID.
     * @param hwndCallback The window handle if MCI_NOTIFY is also specified.
     * @param hasNotify TRUE if MCI_NOTIFY was also specified (posts a delayed SUCCESSFUL).
     * @return  if playback started and completed, FALSE if playback failed to start.
     */
    MMRESULT WaitPlayback(MCIDEVICEID devId, HWND hwndCallback, BOOL hasNotify);

    /**
     * @brief Cancels any active playback notification (for STOP, CLOSE, SEEK, etc.).
     * @param devId The device ID.
     */
    void UnregisterPlaybackNotify(MCIDEVICEID devId);

    /**
     * @brief Posts a notification message after a 100ms delay.
     * @param devId The device ID.
     * @param hwndCallback The window handle for the notification.
     * @param notifyCode The message to send (e.g., MCI_NOTIFY_SUCCESSFUL).
     */
    void LazyNotify(MCIDEVICEID devId, HWND hwndCallback, DWORD notifyCode);

}
