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
#include "audiofilecache.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "filesystem.h"

#include <captainslog.h>

using namespace Thyme;

/**
 * Opens an audio file. Reads from the cache if available or loads from file if not.
 */
AudioDataHandle AudioFileCache::Open_File(const Utf8String &filename, const AudioEventInfo *event_info)
{
    ScopedMutexClass lock(&m_mutex);

    captainslog_trace("AudioFileCache: opening file %s", filename.Str());

    // Try to find existing data for this file to avoid loading it if unneeded.
    auto it = m_cacheMap.find(filename);

    if (it != m_cacheMap.end()) {
        ++(it->second.ref_count);

        return static_cast<AudioDataHandle>(it->second.wave_data);
    }

    // Load the file from disk
    File *file = g_theFileSystem->Open_File(filename.Str(), File::READ | File::BINARY | File::BUFFERED);

    if (file == nullptr) {
        if (filename.Is_Not_Empty()) {
            captainslog_warn("Missing audio file '%s', could not cache.", filename.Str());
        }

        return nullptr;
    }

    OpenAudioFile open_audio;
    if (!Load_File(file, open_audio)) {
        captainslog_warn("Failed to load audio file '%s', could not cache.", filename.Str());
        return nullptr;
    }

    file->Close();

    open_audio.audio_event_info = event_info;
    open_audio.ref_count = 1;
    m_currentSize += open_audio.data_size;

    // m_maxSize prevents using overly large amounts of memory, so if we are over it, unload some other samples.
    if (m_currentSize > m_maxSize && !Free_Space_For_Sample(open_audio)) {
        captainslog_warn("Cannot play audio file since cache is full: %s", filename.Str());
        m_currentSize -= open_audio.data_size;
        Release_Open_Audio(&open_audio);

        return nullptr;
    }

    m_cacheMap[filename] = open_audio;

    return static_cast<AudioDataHandle>(open_audio.wave_data);
}

/**
 * Opens an audio file for an event. Reads from the cache if available or loads from file if not.
 */
AudioDataHandle AudioFileCache::Open_File(AudioEventRTS *audio_event)
{
    Utf8String filename;

    // What part of an event are we playing?
    switch (audio_event->Get_Next_Play_Portion()) {
        case 0:
            filename = audio_event->Get_Attack_Name();
            break;
        case 1:
            filename = audio_event->Get_File_Name();
            break;
        case 2:
            filename = audio_event->Get_Decay_Name();
            break;
        case 3:
        default:
            return nullptr;
    }

    return Open_File(filename, audio_event->Get_Event_Info());
}

/**
 * Closes a file, reducing the references to it. Does not actually free the cache.
 */
void AudioFileCache::Close_File(AudioDataHandle file)
{
    if (file == nullptr) {
        return;
    }

    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        if (static_cast<AudioDataHandle>(it->second.wave_data) == file) {
            --(it->second.ref_count);

            break;
        }
    }
}

/**
 * Sets the maximum amount of memory in bytes that the cache should use.
 */
void AudioFileCache::Set_Max_Size(unsigned size)
{
    ScopedMutexClass lock(&m_mutex);
    m_maxSize = size;
}

/**
 * Attempts to free space by releasing files with no references
 */
unsigned AudioFileCache::Free_Space(unsigned required)
{
    std::list<Utf8String> to_free;
    unsigned freed = 0;

    // First check for samples that don't have any references.
    for (const auto &cached : m_cacheMap) {
        if (cached.second.ref_count == 0) {
            to_free.push_back(cached.first);
            freed += cached.second.data_size;

            // If required is "0" we free as much as possible
            if (required && freed >= required) {
                break;
            }
        }
    }

    for (const auto &file : to_free) {
        auto to_remove = m_cacheMap.find(file);

        if (to_remove != m_cacheMap.end()) {
            Release_Open_Audio(&to_remove->second);
            m_currentSize -= to_remove->second.data_size;
            m_cacheMap.erase(to_remove);
        }
    }

    return freed;
}

/**
 * Attempts to free space for a file by releasing files with no references and lower priority sounds.
 */
bool AudioFileCache::Free_Space_For_Sample(const OpenAudioFile &file)
{
    captainslog_assert(m_currentSize >= m_maxSize); // Assumed to be called only when we need more than allowed.
    std::list<Utf8String> to_free;
    unsigned required = m_currentSize - m_maxSize;
    unsigned freed = 0;

    // First check for samples that don't have any references.
    freed = Free_Space(required);

    // If we still don't have enough potential space freed up, look for lower priority sounds to remove.
    if (freed < required) {
        for (const auto &cached : m_cacheMap) {
            if (cached.second.ref_count != 0
                && cached.second.audio_event_info->Get_Priority() < file.audio_event_info->Get_Priority()) {
                to_free.push_back(cached.first);
                freed += cached.second.data_size;

                if (freed >= required) {
                    break;
                }
            }
        }
    }

    // If we have enough space to free, do the actual freeing, otherwise we didn't succeed, no point bothering.
    if (freed < required) {
        return false;
    }

    for (const auto &file : to_free) {
        auto to_remove = m_cacheMap.find(file);

        if (to_remove != m_cacheMap.end()) {
            Release_Open_Audio(&to_remove->second);
            m_currentSize -= to_remove->second.data_size;
            m_cacheMap.erase(to_remove);
        }
    }

    return true;
}
