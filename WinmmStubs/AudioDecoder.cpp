#include "AudioDecoder.h"
#include <string.h>

static inline DWORD MinDW(DWORD a, DWORD b) { return a < b ? a : b; }
static inline WORD  ReadLE16(const BYTE* p) { return (WORD)(p[0] | (p[1] << 8)); }
static inline DWORD ReadLE32(const BYTE* p) { return (DWORD)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24)); }

#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION
#include <minimp3_ex.h>

AudioDecoder::AudioDecoder() {
    ZeroMemory(&fmt, sizeof(fmt));
    type = AD_File_None;
    ZeroMemory(&wav, sizeof(wav));
    ZeroMemory(&ogg, sizeof(ogg));
    ZeroMemory(&mp3, sizeof(mp3));
    ZeroMemory(&flac, sizeof(flac));
}
AudioDecoder::~AudioDecoder() { Close(); }

// Automatically branches the audio decoder based on the file extension
BOOL AudioDecoder::OpenAuto(const wchar_t* path) {
    if (!path) return FALSE;
    const wchar_t* ext = wcsrchr(path, L'.');
    if (ext) {
        if (_wcsicmp(ext, L".wav") == 0) return OpenWav(path);
        if (_wcsicmp(ext, L".ogg") == 0) return OpenOgg(path);
        if (_wcsicmp(ext, L".mp3") == 0) return OpenMp3(path);
        if (_wcsicmp(ext, L".flac") == 0) return OpenFlac(path);
    }
    // Default to WAV if extension is unknown or missing
    return OpenWav(path);
}

// Releases resources specific to each file type
void AudioDecoder::Close() {
    if (type == AD_File_Wav) {
        if (wav.fp) fclose(wav.fp);
        ZeroMemory(&wav, sizeof(wav));
    }
    if (type == AD_File_Ogg) {
        if (ogg.opened) { ov_clear(&ogg.vf); }
        ZeroMemory(&ogg, sizeof(ogg));
    }
    if (type == AD_File_Mp3) {
        if (mp3.opened) { mp3dec_ex_close(&mp3.ex); }
        ZeroMemory(&mp3, sizeof(mp3));
    }
    if (type == AD_File_Flac) {
        if (flac.dec) { FLAC__stream_decoder_delete(flac.dec); flac.dec = nullptr; }
        if (flac.fp) { fclose(flac.fp); flac.fp = nullptr; }
        if (flac.fifo) { free(flac.fifo); flac.fifo = nullptr; }
        ZeroMemory(&flac, sizeof(flac));
    }

    type = AD_File_None;
    ZeroMemory(&fmt, sizeof(fmt));
}

BOOL AudioDecoder::GetFormat(AD_Format* out) const {
    if (!out) return FALSE;
    if (type == AD_File_None) return FALSE;
    *out = fmt;
    return TRUE;
}

DWORD AudioDecoder::TotalFrames() const {
    switch (type) {
    case AD_File_Wav:
        if (fmt.channels == 0 || wav.originalBps == 0) return 0;
        // Calculate based on *original* BPS
        return (wav.dataBytes / (fmt.channels * (wav.originalBps / 8)));
    case AD_File_Ogg:
        return ogg.totalFrames;
    case AD_File_Mp3:
        return mp3.totalFrames;
    case AD_File_Flac:
        return flac.totalFrames;
    default: return 0;
    }
}

DWORD AudioDecoder::TellFrames() const {
    switch (type) {
    case AD_File_Wav:
        if (fmt.channels == 0 || wav.originalBps == 0) return 0;
        // Calculate based on *original* BPS
        return (wav.dataReadBytes / (fmt.channels * (wav.originalBps / 8)));
    case AD_File_Ogg:
        return ogg.tellFrames;
    case AD_File_Mp3:
        return mp3.tellFrames;
    case AD_File_Flac:
        return flac.tellFrames;
    default: return 0;
    }
}

DWORD AudioDecoder::ReadFrames(short* outPCM, DWORD frames) {
    if (type == AD_File_Wav)  return ReadFramesWav(outPCM, frames);
    if (type == AD_File_Ogg)  return ReadFramesOgg(outPCM, frames);
    if (type == AD_File_Mp3)  return ReadFramesMp3(outPCM, frames);
    if (type == AD_File_Flac) return ReadFramesFlac(outPCM, frames);
    return 0;
}

BOOL AudioDecoder::SeekFrames(DWORD f) {
    if (type == AD_File_Wav)  return SeekFramesWav(f);
    if (type == AD_File_Ogg)  return SeekFramesOgg(f);
    if (type == AD_File_Mp3)  return SeekFramesMp3(f);
    if (type == AD_File_Flac) return SeekFramesFlac(f);
    return FALSE;
}

BOOL AudioDecoder::OpenWav(const wchar_t* path) {
    Close();
    FILE* fp = NULL;
#if defined(_MSC_VER)
    if (_wfopen_s(&fp, path, L"rb") != 0) return FALSE;
#else
    fp = _wfopen(path, L"rb");
    if (!fp) return FALSE;
#endif
    if (!OpenWavInternal(fp)) {
        fclose(fp);
        return FALSE;
    }
    type = AD_File_Wav;
    return TRUE;
}

// Basic PCM WAV (PCM/8-bit or 16-bit) parsing
BOOL AudioDecoder::OpenWavInternal(FILE* fp) {
    BYTE hdr[12];
    if (fread(hdr, 1, 12, fp) != 12) return FALSE;
    if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) return FALSE;

    WORD audioFormat = 1, numChannels = 0, bitsPerSample = 0;
    DWORD sampleRate = 0, byteRate = 0, blockAlign = 0;
    DWORD dataOffset = 0, dataBytes = 0;

    for (;;) {
        BYTE ck[8];
        if (fread(ck, 1, 8, fp) != 8) break;
        DWORD ckSize = ReadLE32(ck + 4);
        if (memcmp(ck, "fmt ", 4) == 0) {
            BYTE* tmp = (BYTE*)malloc(ckSize);
            if (!tmp) return FALSE;
            if (fread(tmp, 1, ckSize, fp) != ckSize) { free(tmp); return FALSE; }
            audioFormat = ReadLE16(tmp + 0);
            numChannels = ReadLE16(tmp + 2);
            sampleRate = ReadLE32(tmp + 4);
            byteRate = ReadLE32(tmp + 8);
            blockAlign = ReadLE16(tmp + 12);
            bitsPerSample = ReadLE16(tmp + 14); // Read the *original* BPS
            free(tmp);
            
            // Only PCM is allowed
            if (audioFormat != 1) return FALSE;
            // Only 1 or 2 channels
            if (numChannels != 1 && numChannels != 2) return FALSE;
            // Only 8-bit or 16-bit
            if (bitsPerSample != 16 && bitsPerSample != 8) return FALSE;
        }
        else if (memcmp(ck, "data", 4) == 0) {
            dataOffset = (DWORD)ftell(fp);
            dataBytes = ckSize;
            fseek(fp, ckSize, SEEK_CUR);
        }
        else {
            fseek(fp, ckSize, SEEK_CUR);
        }
    }
    if (dataOffset == 0 || dataBytes == 0) return FALSE;

    // Fill in the *output* format struct
    fmt.sampleRate = sampleRate;
    fmt.channels = numChannels;
    fmt.bitsPerSample = 16; // Internal/output format is fixed to 16bit

    // Store WAV-specific info
    wav.fp = fp;
    wav.dataOffset = dataOffset;
    wav.dataBytes = dataBytes;
    wav.dataReadBytes = 0;
    wav.originalBps = bitsPerSample; // Store the *original* BPS

    // Seek to the data position
    fseek(wav.fp, wav.dataOffset, SEEK_SET);
    return TRUE;
}

DWORD AudioDecoder::ReadFramesWav(short* outPCM, DWORD frames) {
    if (!wav.fp) return 0;
    const UINT ch = fmt.channels;
    
    // Use the *original* BPS for source byte calculation
    const UINT srcBps = wav.originalBps;
    const UINT srcBpsBytes = srcBps / 8;
    DWORD bytesPerFrameSrc = ch * srcBpsBytes;
    DWORD bytesNeed = frames * bytesPerFrameSrc;

    // Remaining bytes
    DWORD remainBytes = 0;
    if (wav.dataBytes >= wav.dataReadBytes)
        remainBytes = wav.dataBytes - wav.dataReadBytes;
    
    DWORD toRead = MinDW(bytesNeed, remainBytes);
    
    // If 8-bit, read directly into the *end* of the buffer
    // If 16-bit, read directly into the *start*
    void* readBuffer = outPCM;
    if (srcBps == 8) {
        // read into the end of the buffer to convert in-place backwards
        readBuffer = (BYTE*)outPCM + (frames * ch * 2) - toRead;
    }

    DWORD read = (DWORD)fread(readBuffer, 1, toRead, wav.fp);
    wav.dataReadBytes += read;

    if (srcBps == 16) {
        // Already S16, so: read bytes -> number of frames
        return (read / (ch * 2));
    }
    else {
        // Convert 8-bit unsigned -> 16-bit signed
        // read into the end, so now convert backwards
        BYTE* pSrc = (BYTE*)readBuffer;
        short* pDest = outPCM;
        DWORD samples = read; // For 8-bit, bytes == samples
        
        for (DWORD i = 0; i < samples; i++) {
            unsigned char u = pSrc[i];
            short s = (short)((int)u - 128) << 8;
            pDest[i] = s;
        }
        return (read / ch);
    }
}

BOOL AudioDecoder::SeekFramesWav(DWORD f) {
    if (!wav.fp) return FALSE;
    // Use *original* BPS for offset calculation
    DWORD bytesPerFrameSrc = fmt.channels * (wav.originalBps / 8);
    DWORD off = f * bytesPerFrameSrc;
    if (off > wav.dataBytes) off = wav.dataBytes;
    if (fseek(wav.fp, wav.dataOffset + off, SEEK_SET) != 0) return FALSE;
    wav.dataReadBytes = off;
    return TRUE;
}

// --- OGG Decoder Start ---
static size_t ov_cb_read(void* ptr, size_t size, size_t nmemb, void* datasource) {
    return fread(ptr, size, nmemb, (FILE*)datasource);
}
static int ov_cb_seek(void* datasource, ogg_int64_t offset, int whence) {
    return _fseeki64((FILE*)datasource, offset, whence);
}
static int ov_cb_close(void* datasource) {
    return fclose((FILE*)datasource);
}
static long ov_cb_tell(void* datasource) {
    return (long)_ftelli64((FILE*)datasource);
}

BOOL AudioDecoder::OpenOgg(const wchar_t* path) {
    Close();
    FILE* fp = NULL;
#if defined(_MSC_VER)
    if (_wfopen_s(&fp, path, L"rb") != 0) return FALSE;
#else
    fp = _wfopen(path, L"rb");
    if (!fp) return FALSE;
#endif

    ov_callbacks cbs;
    cbs.read_func = ov_cb_read;
    cbs.seek_func = ov_cb_seek;
    cbs.close_func = ov_cb_close;
    cbs.tell_func = ov_cb_tell;

    if (ov_open_callbacks(fp, &ogg.vf, NULL, 0, cbs) < 0) {
        fclose(fp);
        return FALSE;
    }
    vorbis_info* vi = ov_info(&ogg.vf, -1);
    if (!vi) { ov_clear(&ogg.vf); return FALSE; }

    fmt.sampleRate = (UINT)vi->rate;
    fmt.channels = (vi->channels == 1 || vi->channels == 2) ? (UINT)vi->channels : 2; // Limit to 1 or 2 ch
    fmt.bitsPerSample = 16;

    // Get total frames (ov_pcm_total returns total PCM samples *per channel*)
    ogg_int64_t totalPcmSamples = ov_pcm_total(&ogg.vf, -1);
    if (totalPcmSamples > 0) {
        ogg.totalFrames = (DWORD)totalPcmSamples;
    }
    else {
        ogg.totalFrames = 0;
    }
    ogg.tellFrames = 0;
    ogg.opened = TRUE;
    ogg.curSection = 0;
    type = AD_File_Ogg;
    return TRUE;
}

DWORD AudioDecoder::ReadFramesOgg(short* outPCM, DWORD frames) {
    if (!ogg.opened) return 0;
    DWORD wantBytes = frames * fmt.channels * 2;
    DWORD have = 0;
    while (have < wantBytes) {
        // 0 = Little Endian, 2 = 16-bit, 1 = signed
        long ret = ov_read(&ogg.vf, (char*)outPCM + have, (int)(wantBytes - have), 0, 2, 1, &ogg.curSection);
        if (ret == 0) break; // EOF
        if (ret < 0)  break; // error -> stop
        have += (DWORD)ret;
    }
    DWORD framesOut = have / (fmt.channels * 2);
    ogg.tellFrames += framesOut;
    return framesOut;
}

BOOL AudioDecoder::SeekFramesOgg(DWORD f) {
    if (!ogg.opened) return FALSE;
    // libvorbisfile seeks by PCM sample offset (per channel), which is our "frame"
    ogg_int64_t pcm_offset = (ogg_int64_t)f;
    if (ov_pcm_seek(&ogg.vf, pcm_offset) != 0) return FALSE;
    ogg.tellFrames = f;
    return TRUE;
}

// --- MP3 Decoder Start ---
BOOL AudioDecoder::OpenMp3(const wchar_t* path) {
    Close();

    // MP3D_SEEK_TO_SAMPLE allows frame-accurate seeking
    if (mp3dec_ex_open_w(&mp3.ex, path, MP3D_SEEK_TO_SAMPLE) != 0) return FALSE;

    fmt.sampleRate = mp3.ex.info.hz;
    fmt.channels = (mp3.ex.info.channels == 1 || mp3.ex.info.channels == 2) ? mp3.ex.info.channels : 2;
    fmt.bitsPerSample = 16;

    // Total frames (total interleaved samples / channels)
    if (mp3.ex.samples > 0) {
        mp3.totalFrames = (DWORD)(mp3.ex.samples / fmt.channels);
    }
    else {
        mp3.totalFrames = 0;
    }
    mp3.tellFrames = 0;
    mp3.opened = TRUE;
    type = AD_File_Mp3;
    return TRUE;
}

DWORD AudioDecoder::ReadFramesMp3(short* outPCM, DWORD frames) {
    if (!mp3.opened) return 0;
    // wantSamples is total interleaved samples (frames * channels)
    size_t wantSamples = (size_t)frames * fmt.channels;
    size_t gotSamples = mp3dec_ex_read(&mp3.ex, outPCM, wantSamples);
    DWORD framesOut = (DWORD)(gotSamples / fmt.channels);
    mp3.tellFrames += framesOut;
    return framesOut;
}

BOOL AudioDecoder::SeekFramesMp3(DWORD f) {
    if (!mp3.opened) return FALSE;
    // sample offset (total interleaved samples)
    size_t sample_offset = (size_t)f * fmt.channels;
    if (mp3dec_ex_seek(&mp3.ex, sample_offset) < 0) return FALSE;
    mp3.tellFrames = f;
    return TRUE;
}

// --- FLAC Decoder Start ---
// FIFO helpers
void AudioDecoder::FlacFifoClear() {
    if (flac.fifo) {
        flac.fifoLen = 0;
    }
}
bool AudioDecoder::FlacFifoEnsure(size_t needMoreSamples) {
    // Ensure capacity to append 'needMoreSamples'
    size_t need = flac.fifoLen + needMoreSamples;
    if (need <= flac.fifoCap) return true;
    size_t newCap = (flac.fifoCap ? flac.fifoCap : 8192);
    while (newCap < need) newCap *= 2;
    short* p = (short*)realloc(flac.fifo, newCap * sizeof(short));
    if (!p) return false;
    flac.fifo = p;
    flac.fifoCap = newCap;
    return true;
}
void AudioDecoder::FlacFifoPushInterleaved(const short* data, size_t samples) {
    if (samples == 0) return;
    if (!FlacFifoEnsure(samples)) return; // drop on OOM
    memcpy(flac.fifo + flac.fifoLen, data, samples * sizeof(short));
    flac.fifoLen += samples;
}

// Callbacks
FLAC__StreamDecoderWriteStatus AudioDecoder::FlacWriteCB(
    const FLAC__StreamDecoder*,
    const FLAC__Frame* frame,
    const FLAC__int32* const buffer[],
    void* client_data)
{
    AudioDecoder* self = (AudioDecoder*)client_data;
    
    const UINT ch = self->fmt.channels;       // 1 or 2 (output)
    const UINT inCh = self->flac.srcChannels; // actual channels in stream
    const UINT bps = self->flac.srcBitsPerSample;

    const unsigned blocksize = frame->header.blocksize;
    if (blocksize == 0) return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;

    // Prepare temp interleaved S16 buffer
    // samples = blocksize * ch
    size_t totalSamples = (size_t)blocksize * ch;
    if (!self->FlacFifoEnsure(totalSamples)) {
        return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
    }

    // Convert and (if needed) downmix to 1/2 ch
    // Strategy:
    // - If inCh == 1 and out == 2: duplicate to stereo
    // - If inCh >= 2 and out == 2: take first two channels (L,R)
    // - If out == 1: average all input channels to mono
    for (unsigned i = 0; i < blocksize; ++i) {
        // Read one sample for each input channel
        // Convert to S16
        auto to_s16 = [bps](FLAC__int32 v)->short {
            if (bps <= 16) {
                // align to 16bit
                int shift = 16 - (int)bps;
                int s = (int)(v << shift);
                return (short)(s);
            } else {
                // 24 or 32 -> shift down
                int shift = (int)bps - 16;
                int s = (int)(v >> shift);
                return (short)(s);
            }
        };

        if (ch == 2) {
            short L = 0, R = 0;
            if (inCh == 1) {
                short mono = to_s16(buffer[0][i]);
                L = mono; R = mono;
            } else {
                // At least 2 input channels: use first two
                L = to_s16(buffer[0][i]);
                R = to_s16(buffer[1][i]);
            }
            self->flac.fifo[self->flac.fifoLen++] = L;
            self->flac.fifo[self->flac.fifoLen++] = R;
        } else { // ch == 1
            // Average all input channels to mono
            int acc = 0;
            for (UINT c = 0; c < inCh; ++c) {
                acc += (int)to_s16(buffer[c][i]);
            }
            acc /= (int)inCh;
            self->flac.fifo[self->flac.fifoLen++] = (short)acc;
        }
    }

    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void AudioDecoder::FlacMetadataCB(const FLAC__StreamDecoder*, const FLAC__StreamMetadata* md, void* client_data) {
    if (md->type != FLAC__METADATA_TYPE_STREAMINFO) return;
    AudioDecoder* self = (AudioDecoder*)client_data;
    const FLAC__StreamMetadata_StreamInfo& si = md->data.stream_info;
    
    self->flac.srcSampleRate    = si.sample_rate;
    self->flac.srcChannels      = si.channels;
    self->flac.srcBitsPerSample = si.bits_per_sample;

    if (si.total_samples > 0) {
        // total_samples is per-channel sample count
        self->flac.totalFrames = (DWORD)si.total_samples;
    } else {
        self->flac.totalFrames = 0;
    }
}

void AudioDecoder::FlacErrorCB(const FLAC__StreamDecoder*, FLAC__StreamDecoderErrorStatus, void*) {
    // No-op: caller will stop on read failure
}

BOOL AudioDecoder::OpenFlac(const wchar_t* path) {
    Close();

    // Open FILE* with wide path and init FILE-based decoder
    FILE* fp = nullptr;
#if defined(_MSC_VER)
    if (_wfopen_s(&fp, path, L"rb") != 0) return FALSE;
#else
    fp = _wfopen(path, L"rb");
    if (!fp) return FALSE;
#endif

    flac.dec = FLAC__stream_decoder_new();
    if (!flac.dec) { fclose(fp); return FALSE; }

    FLAC__StreamDecoderInitStatus ist =
        FLAC__stream_decoder_init_FILE(flac.dec, fp, &AudioDecoder::FlacWriteCB, &AudioDecoder::FlacMetadataCB, &AudioDecoder::FlacErrorCB, this);
    if (ist != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        FLAC__stream_decoder_delete(flac.dec); flac.dec = nullptr;
        fclose(fp);
        return FALSE;
    }

    // Read and process STREAMINFO
    if (!FLAC__stream_decoder_process_until_end_of_metadata(flac.dec)) {
        FLAC__stream_decoder_delete(flac.dec); flac.dec = nullptr;
        fclose(fp);
        return FALSE;
    }

    // Validate channels: only 1 or 2 supported for output
    UINT outCh = (flac.srcChannels == 1) ? 1 : 2;
    fmt.sampleRate    = (flac.srcSampleRate ? flac.srcSampleRate : 44100);
    fmt.channels      = outCh;
    fmt.bitsPerSample = 16;

    // Reject exotic channel layout if src has 0 channel
    if (flac.srcChannels == 0) {
        FLAC__stream_decoder_delete(flac.dec); flac.dec = nullptr; fclose(fp); return FALSE;
    }

    flac.fp      = fp;
    flac.opened  = TRUE;
    flac.eof     = FALSE;
    flac.tellFrames = 0;

    flac.fifo    = nullptr;
    flac.fifoLen = 0;
    flac.fifoCap = 0;

    type = AD_File_Flac;
    return TRUE;
}

DWORD AudioDecoder::ReadFramesFlac(short* outPCM, DWORD frames) {
    if (!flac.opened) return 0;
    const size_t wantSamples = (size_t)frames * fmt.channels;

    // While FIFO has less than needed samples, ask decoder to produce more
    while (flac.fifoLen < wantSamples && !flac.eof) {
        if (!FLAC__stream_decoder_process_single(flac.dec)) {
            // decoding error; stop further decoding
            flac.eof = TRUE;
            break;
        }
        if (FLAC__stream_decoder_get_state(flac.dec) == FLAC__STREAM_DECODER_END_OF_STREAM) {
            flac.eof = TRUE;
            break;
        }
        // Write callback already pushed samples into FIFO
    }

    size_t give = (flac.fifoLen < wantSamples) ? flac.fifoLen : wantSamples;
    if (give > 0) {
        memcpy(outPCM, flac.fifo, give * sizeof(short));
        // shift FIFO
        memmove(flac.fifo, flac.fifo + give, (flac.fifoLen - give) * sizeof(short));
        flac.fifoLen -= give;
    }

    DWORD framesOut = (DWORD)(give / fmt.channels);
    flac.tellFrames += framesOut;
    return framesOut;
}

BOOL AudioDecoder::SeekFramesFlac(DWORD f) {
    if (!flac.opened) return FALSE;
    // libFLAC seeks by absolute sample index (per channel)
    // Clear FIFO because positions change
    FlacFifoClear();

    if (!FLAC__stream_decoder_seek_absolute(flac.dec, (FLAC__uint64)f)) {
        return FALSE; // not seekable or error
    }
    flac.eof = FALSE;
    flac.tellFrames = f;
    return TRUE;
}