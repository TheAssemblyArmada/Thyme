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

struct FFmpegOpenAudioFile
{
    // FFmpeg handles
    AVFormatContext *fmt_ctx = nullptr;
    AVIOContext *avio_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
    uint8_t *wave_data;
    int ref_count;
    int data_size;
    bool ffmpeg_allocated;
    const AudioEventInfo *audio_event_info;
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
    void *Open_File(AudioEventRTS *file);
    void Close_File(void *file);
    void Set_Max_Size(unsigned size);

private:
    bool Free_Space_For_Sample(const FFmpegOpenAudioFile &file);
    void Release_Open_Audio(FFmpegOpenAudioFile *file);
    bool Open_FFmpeg_Contexts(FFmpegOpenAudioFile *file, unsigned char *file_data, uint32_t file_size);

private:
    ffmpegaudiocachemap_t m_cacheMap;
    unsigned m_currentSize;
    unsigned m_maxSize;
    SimpleMutexClass m_mutex;
};
