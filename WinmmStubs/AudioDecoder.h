#pragma once
#include "GlobalDefinitions.h"

// Audio Decoder (WAV/OGG/MP3 -> 16-bit PCM, interleaved)
// Sample Format: S16 (Signed 16-bit)
// Channels: 1 or 2 (Others will be downmixed or fail)
// ReadFrames() operates based on "frame count" (1 frame = a group of 'channels' samples)

#define MINIMP3_ALLOW_MONO_STEREO_TRANSITION
#include <minimp3_ex.h>
#include <vorbis/vorbisfile.h>

// Enum identifying the loaded audio file type
enum AD_FileType {
    AD_File_None = 0,
    AD_File_Wav,
    AD_File_Ogg,
    AD_File_Mp3
};

// Struct holding the *decoded output* PCM format
struct AD_Format {
    UINT  sampleRate;   // Hz
    UINT  channels;     // 1 or 2
    UINT  bitsPerSample;// Fixed to 16 (internal conversion)
};

class AudioDecoder {
public:
    AudioDecoder();
    ~AudioDecoder();

    // --- Open/Close Methods ---

    /**
     * @brief Opens a WAV file for decoding.
     * @param path File path.
     * @return TRUE on success, FALSE on failure.
     */
    BOOL OpenWav(const wchar_t* path);

    /**
     * @brief Opens an OGG Vorbis file for decoding.
     * @param path File path.
     * @return TRUE on success, FALSE on failure.
     */
    BOOL OpenOgg(const wchar_t* path);

    /**
     * @brief Opens an MP3 file for decoding.
     * @param path File path.
     * @return TRUE on success, FALSE on failure.
     */
    BOOL OpenMp3(const wchar_t* path);

    /**
     * @brief Opens a file, auto-detecting the type (WAV, OGG, MP3) by its extension.
     * @param path File path.
     * @return TRUE on success, FALSE on failure.
     */
    BOOL OpenAuto(const wchar_t* path); // Auto-select based on file extension

    /**
     * @brief Closes the currently open file and releases all resources.
     */
    void Close();

    // --- Format/Length ---

    /**
     * @brief Gets the audio format of the opened file.
     * @param out Pointer to an AD_Format struct to receive the format info.
     * @return TRUE if a file is open and format is retrieved, FALSE otherwise.
     */
    BOOL GetFormat(AD_Format* out) const;

    /**
     * @brief Gets the total number of PCM frames in the file.
     * @return Total frames. Returns 0 if unknown.
     */
    DWORD TotalFrames() const;

    /**
     * @brief Gets the current read position in PCM frames.
     * @return Current frame position.
     */
    DWORD TellFrames() const;

    // --- Read/Seek ---

    /**
     * @brief Reads a specified number of frames into the output buffer.
     * @param outPCM Pointer to the output buffer (must be S16, interleaved).
     * @param frames Number of frames to read.
     * @return The number of frames actually read (can be less than requested at EOF).
     */
    DWORD ReadFrames(short* outPCM, DWORD frames); // outPCM: interleaved S16

    /**
     * @brief Seeks to a specific frame position (absolute).
     * @param targetFrame The absolute frame index to seek to.
     * @return TRUE on success, FALSE on failure.
     */
    BOOL SeekFrames(DWORD targetFrame); // Based on absolute position

private:
    // --- Internal Implementations ---
    BOOL  OpenWavInternal(FILE* fp);
    DWORD ReadFramesWav(short* outPCM, DWORD frames);
    BOOL  SeekFramesWav(DWORD f);

    DWORD ReadFramesOgg(short* outPCM, DWORD frames);
    BOOL  SeekFramesOgg(DWORD f);

    DWORD ReadFramesMp3(short* outPCM, DWORD frames);
    BOOL  SeekFramesMp3(DWORD f);

private:
    AD_FileType type;
    AD_Format   fmt; // Holds the *output* format (always 16-bit)

    // WAV
    struct {
        FILE* fp;
        DWORD dataOffset;
        DWORD dataBytes;
        DWORD dataReadBytes;
        UINT  originalBps; // Stores the *source* BPS (8 or 16)
    } wav;

    // OGG
    struct {
        OggVorbis_File vf;
        BOOL opened;
        int curSection;
        DWORD totalFrames;
        DWORD tellFrames;
    } ogg;

    // MP3
    struct {
        mp3dec_ex_t ex;
        BOOL opened;
        DWORD totalFrames;
        DWORD tellFrames;
    } mp3;
};
