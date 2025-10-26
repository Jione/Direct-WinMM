#pragma once
#include "GlobalDefinitions.h"

namespace PreferenceLoader {
    /**
     * @brief Initializes the preference loader.
     * Attempts to launch WinmmVol.exe if not running and starts monitoring
     * the registry for volume changes. Should be called when the audio device
     * is first accessed (e.g., inside DeviceInfo::Initialize or the first MCI_OPEN).
     * Safe to call multiple times.
     * @return TRUE if initialization was successful or already done.
     */
    BOOL Initialize();

    /**
     * @brief Shuts down the preference loader.
     * Stops the registry monitoring thread and attempts to signal WinmmVol.exe
     * to clean up (e.g., remove tray icon). Should be called when the DLL
     * is unloading or the last audio device is closed.
     */
    void Shutdown();

} // namespace PreferenceLoader
