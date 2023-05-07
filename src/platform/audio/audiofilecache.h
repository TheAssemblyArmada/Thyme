/**
 * @file
 *
 * @author feliwir
 *
 * @brief Base class for caching loaded audio samples to reduce file IO.
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
#include "file.h"
#include "mutex.h"
#include "rtsutils.h"

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class AudioEventInfo;
class AudioEventRTS;

struct OpenAudioFile
{
    AudioDataHandle wave_data = nullptr;
    int ref_count = 0;
    int data_size = 0;
    const AudioEventInfo *audio_event_info = nullptr;
    void *opaque = nullptr;
};

#ifdef THYME_USE_STLPORT
typedef std::hash_map<const Utf8String, OpenAudioFile, rts::hash<Utf8String>, std::equal_to<Utf8String>> audiocachemap_t;
#else
typedef std::unordered_map<const Utf8String, OpenAudioFile, rts::hash<Utf8String>, std::equal_to<Utf8String>>
    audiocachemap_t;
#endif

namespace Thyme
{

class AudioFileCache
{
public:
    AudioFileCache() : m_maxSize(0), m_currentSize(0), m_mutex("AudioFileCacheMutex") {}
    AudioDataHandle Open_File(AudioEventRTS *file);
    AudioDataHandle Open_File(const Utf8String &filename, const AudioEventInfo *event_info = nullptr);

    void Close_File(AudioDataHandle file);
    void Set_Max_Size(unsigned size);
    inline unsigned Get_Max_Size() const { return m_maxSize; }
    inline unsigned Get_Current_Size() const { return m_currentSize; }

    // #FEATURE: We can maybe call this during loading to free any old sounds we won't need ingame and decrease computation
    // ingame
    unsigned Free_Space(unsigned required = 0);

protected:
    bool Free_Space_For_Sample(const OpenAudioFile &open_audio);

    virtual bool Load_File(File *file, OpenAudioFile &audio_file) = 0;
    virtual void Release_Open_Audio(OpenAudioFile *open_audio) = 0;

protected:
    audiocachemap_t m_cacheMap;
    unsigned m_currentSize;
    unsigned m_maxSize;
    SimpleMutexClass m_mutex;
};
} // namespace Thyme
