/**
 * @file
 *
 * @author feliwir
 *
 * @brief Class for caching loaded audio samples to reduce file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "asciistring.h"
#include "audiomanager.h"
#include "mutex.h"
#include "rtsutils.h"

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class AudioEventInfo;
class AudioEventRTS;

struct AVFormatContext;
struct AVIOContext;
struct AVCodecContext;

namespace Thyme
{
struct FFmpegOpenAudioFile
{
    // FFmpeg handles
    AVFormatContext *fmt_ctx = nullptr;
    AVIOContext *avio_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    uint8_t *wave_data = nullptr;
    float duration = 0.0f;
    int ref_count = 0;
    int data_size = 0;
    const AudioEventInfo *audio_event_info = nullptr;
};

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, FFmpegOpenAudioFile, rts::hash<Utf8String>, std::equal_to<Utf8String>>
    ffmpegaudiocachemap_t;
#else
typedef std::unordered_map<const Utf8String, FFmpegOpenAudioFile, rts::hash<Utf8String>, std::equal_to<Utf8String>>
    ffmpegaudiocachemap_t;
#endif

class FFmpegAudioFileCache
{
public:
    FFmpegAudioFileCache() : m_maxSize(0), m_currentSize(0), m_mutex("AudioFileCacheMutex") {}
    virtual ~FFmpegAudioFileCache();
    AudioDataHandle Open_File(AudioEventRTS *file);
    AudioDataHandle Open_File(const Utf8String &filename);

    void Close_File(AudioDataHandle file);
    void Set_Max_Size(unsigned size);
    inline unsigned Get_Max_Size() const { return m_maxSize; }
    inline unsigned Get_Current_Size() const { return m_currentSize; }

    float Get_File_Length_MS(AudioDataHandle file);

    // #FEATURE: We can maybe call this during loading to free any old sounds we won't need ingame and decrease computation
    // ingame
    unsigned Free_Space(unsigned required = 0);

private:
    bool Free_Space_For_Sample(const FFmpegOpenAudioFile &open_audio);
    void Release_Open_Audio(FFmpegOpenAudioFile *open_audio);

    bool Open_FFmpeg_Contexts(FFmpegOpenAudioFile *open_audio, File *file);
    bool Decode_FFmpeg(FFmpegOpenAudioFile *open_audio);
    void Close_FFmpeg_Contexts(FFmpegOpenAudioFile *open_audio);
    static int Read_FFmpeg_Packet(void *opaque, uint8_t *buf, int buf_size);

    void Fill_Wave_Data(FFmpegOpenAudioFile *open_audio);

private:
    ffmpegaudiocachemap_t m_cacheMap;
    unsigned m_currentSize;
    unsigned m_maxSize;
    SimpleMutexClass m_mutex;
};
} // namespace Thyme
