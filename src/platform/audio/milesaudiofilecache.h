/**
 * @file
 *
 * @author OmniBlade
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
#ifdef BUILD_WITH_MILES
#include <miles.h>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class AudioEventInfo;
class AudioEventRTS;

struct OpenAudioFile
{
    AILSOUNDINFO info;
    void *wave_data;
    int ref_count;
    int data_size;
    bool miles_allocated;
    const AudioEventInfo *audio_event_info;
};

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, OpenAudioFile, rts::hash<Utf8String>, rts::equal_to<Utf8String>> audiocachemap_t;
#else
typedef std::unordered_map<const Utf8String, OpenAudioFile, rts::hash<Utf8String>, rts::equal_to<Utf8String>>
    audiocachemap_t;
#endif
#endif // BUILD_WITH_MILES

class MilesAudioFileCache
{
#ifdef BUILD_WITH_MILES
public:
    MilesAudioFileCache() : m_maxSize(0), m_currentSize(0), m_mutex("AudioFileCacheMutex") {}
    virtual ~MilesAudioFileCache();
    void *Open_File(AudioEventRTS *file);
    void Close_File(void *file);
    void Set_Max_Size(unsigned size);

private:
    bool Free_Space_For_Sample(const OpenAudioFile &file);
    void Release_Open_Audio(OpenAudioFile *file);

private:
    audiocachemap_t m_cacheMap;
    unsigned m_currentSize;
    unsigned m_maxSize;
    SimpleMutexClass m_mutex;
#endif
};
