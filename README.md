![DirectWinMM Logo](DirectWinMM-Logo.png)

# Direct-WinMM: Play CD Audio via DirectSound / WASAPI

**Direct-WinMM** is a DirectSound/WASAPI-based wrapper for `winmm.dll`, used by many older games and programs to play CD-DA audio tracks.
This program emulates CD-DA playback by playing supported audio files (`.wav`, `.mp3`, `.ogg`, `.flac`) instead of actual audio tracks from a CD-ROM.

It fixes the common audio looping bug present in modern Windows (Vista and newer) and provides advanced configuration options for maximum compatibility and performance.

# How to Use

1.  Copy `winmm.dll` and `WinmmVol.exe` into the folder where the game's or program's executable is located.

2.  Rip your audio tracks as `.wav`, `.mp3`, `.ogg`, or `.flac` files.

3.  The program searches for audio files in a subfolder using the format: `[Subfolder]/[Prefix]NN.[wav|mp3|ogg|flac]`.
    * `NN` represents the two-digit track number (e.g., 02, 03, 04...).
    * Track 1 is typically data, so audio playback usually starts from Track 02.

4.  **Recommended Setup:** The program gives **highest priority** to the following path:
    `[GameFolder]/Music/TrackNN.[wav|mp3|ogg|flac]`
    * Example: `C:/Games/MyGame/Music/Track02.mp3`

5.  If audio is not playing, ensure your files are named and placed according to this priority structure (`Music/TrackNN...`).

6.  Enjoy your game!


## File Descriptions

* **`winmm.dll`**: The core wrapper file. It intercepts CD-DA commands and emulates them by playing audio files.
* **`WinmmVol.exe`**: The configuration and volume control utility.

The core CD audio emulation will function correctly with only `winmm.dll`. However, `WinmmVol.exe` is required to access any volume control or advanced configuration.


## Volume Control Utility (WinmmVol.exe)

`WinmmVol.exe` provides all configuration for the wrapper. It runs automatically when a game uses `winmm.dll` and registers itself as a **dynamic tray icon** (the icon's appearance changes with the volume level).

Clicking the tray icon opens the main control window. If you launch `WinmmVol.exe` again while it's running, the window will appear at your mouse cursor (this is useful for environments like Winlator where the tray is inaccessible).

![WinmmVol UI](DirectWinMM-VolumeControl.png)

### Key Features

* **Per-Application Settings:** The UI defaults to "Global Settings." You can use the **App Selection dropdown** to choose a specific game's .exe and create an override profile.
* **Multiplicative Volume:** The final volume is `(Global Volume * App Volume)`.
* **Integrated Settings:** The Audio Engine and Buffer Mode settings are now managed directly within this window.
* **Localization:** The UI supports 22 languages and will attempt to auto-detect your OS language.
* **Window Management:**
    * The window can be **moved** by dragging the top bar.
    * A **Minimize** button is included.
* **Controls:**
    * Use the **mouse wheel** or **keyboard arrow keys** to adjust the volume.
    * Click the **speaker icon** inside the window to toggle Mute.

### .exe Patch (for Wine)
The App Selection dropdown contains a special utility: **".exe Patch (for Wine)"**.

This is for environments like Wine/Proton that may fail to load the custom `winmm.dll`. This tool patches the game's `.exe` file to load `_inmm.dll` instead (and copies `winmm.dll` to `_inmm.dll`). It also provides an option to unpatch (restore) the executable.

---

## System Requirements

* Minimum OS: Windows XP or later
* [DirectX End-User Runtime](https://www.microsoft.com/en-us/download/details.aspx?id=8109)

---

# Known Issues

### Locale Emulator Compatibility Issue
On Windows 10 and newer, using **Locale Emulator** with a specific combination of settings can cause problems.

* **Trigger:** Enabling *both* "Fake language-related keys in Registry" **AND** "Fake system UI language" at the same time.
* **Problem:** This configuration prevents the wrapper from correctly forwarding calls to the original system `winmm.dll` (likely due to a `ucrt` library conflict).
* **Symptom:** Only the CD Audio emulation will function. Other `winmm` features (like standard wave audio or MIDI playback) will fail.
* **Solution:** To fix this, **only enable one** of those two options, not both.
* **Note:** This is a known issue that affects all `winmm` wrappers.

### Wine / Proton Compatibility Issue
Wine's default configuration will often load its own built-in `winmm.dll`, bypassing Direct-WinMM.

You have two options to fix this:

1.  **Solution 1 (Configuration):**
    * Open `winecfg` for your game prefix.
    * Go to the **Libraries** tab.
    * Type in `winmm` and click **Add**.
    * Select `winmm` in the list and set its override to **"Native, builtin"**.

2.  **Solution 2 (Patcher):**
    * Use the **".exe Patch (for Wine)"** utility.
    * This tool is located in the app selection dropdown menu in the `WinmmVol.exe` control window.

---

# Advanced Configuration

These settings are available in the `WinmmVol.exe` control window and can be set globally or per-application.

### Audio Engine
Controls the audio backend used for playback.

* **Auto (Default):** The best choice. Uses `DirectSound` on Windows XP and `WASAPI` on Windows Vista and newer.
* **DirectSound:** An older audio system.
* **WASAPI:** The modern audio system for Windows.
* **WaveOut:** A legacy fallback engine. Use this only if DirectSound is being used exclusively by the game itself.

### Buffer Mode
Controls how audio files are read from disk.

* **Streaming (Default):** **Recommended for 99% of users.** Uses a large staging buffer to provide low-memory, glitch-free playback.
* **Full (No Resampling):** Loads the *entire* audio file into RAM before playing. Uses more memory but can be an alternative for troubleshooting.
* **Full (Resampling):** Loads the entire file into RAM *and* resamples it to 48kHz using libsamplerate. This bypasses the low-quality `kmixer` in XP, fixing distortion.

---

# How the Program Works

Windows prioritizes loading DLL files located in the same folder as the executable. The target program loads our custom `winmm.dll` instead of the system's.

**Direct-WinMM** forwards all non-CD-DA commands to the system's original `winmm.dll`.
When a **CD-DA** command is detected, Direct-WinMM handles it internally using **WASAPI**, **DirectSound**, or **WaveOut**.

Unlike many existing wrappers, Direct-WinMM correctly implements **Notify** and **Wait** semantics and returns accurate status information (like playback position) consistent with the original `winmm.dll`, ensuring high compatibility.

---

# Build Guide

* Build using **Visual Studio 2022** on **Windows**
* Navigate to:
    `Menu > Tools (T) > Get Tools and Features (T)`
    Go to the **Individual components** tab, search for **v141**, and update **x86-related tools**
* Install **Windows SDK 7.1A** (for XP/legacy headers & libs if needed)
* Install **DirectX SDK** (for legacy DirectSound headers & libs)

## SDKs

* [Windows SDK 7.1A](https://www.microsoft.com/en-us/download/details.aspx?id=8442)
* [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)

## Dependent Libraries (via Git Submodule)

* [minimp3](https://github.com/lieff/minimp3.git)
* [libogg](https://github.com/gcp/libogg.git)
* [libvorbis](https://github.com/xiph/vorbis.git)
* [libflac](https://github.com/xiph/flac.git)
* [libsamplerate](https://github.com/libsndfile/libsamplerate.git)

---

# Referenced Projects

- [_inmm.dll (by irori)](https://cryo.jp/_inmm/)
- [AheadLib (by Xjun)](https://github.com/strivexjun/AheadLib-x86-x64.git)
- [ogg-winmm (by hifi)](https://github.com/hifi-unmaintained/ogg-winmm.git)
- [ogg-winmm (by AyuanX)](https://github.com/ayuanx/ogg-winmm.git)
- [cdaudio-winmm (by dippy-dipper)](https://github.com/dippy-dipper/cdaudio-winmm.git)
- [cdaudio-winmm (by YELLO-belly)](https://github.com/YELLO-belly/cdaudio-winmm.git)
