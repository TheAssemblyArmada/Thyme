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
#include "milesaudiofilecache.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "filesystem.h"
#include <captainslog.h>
#include <list>

/**
 * 0x00780D30
 */
MilesAudioFileCache::~MilesAudioFileCache()
{
    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        Release_Open_Audio(&it->second);
    }
}

/**
 * Opens an audio file for an event. Reads from the cache if available or loads from file if not.
 *
 * 0x00780F80
 */
AudioDataHandle MilesAudioFileCache::Open_File(const AudioEventRTS *audio_event)
{
    ScopedMutexClass lock(&m_mutex);
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
            return nullptr;
        default:
            break;
    }

    // Try to find existing data for this file to avoid loading it if unneeded.
    auto it = m_cacheMap.find(filename);

    if (it != m_cacheMap.end()) {
        ++(it->second.ref_count);

        return it->second.wave_data;
    }

    // Load the file from disk
    File *file = g_theFileSystem->Open_File(filename.Str(), File::READ | File::BINARY);

    if (file == nullptr) {
        if (!filename.Is_Empty()) {
            captainslog_warn("Missing audio file '%s', could not cache.", filename.Str());
        }

        return nullptr;
    }

    uint32_t file_size = file->Size();
    AudioDataHandle file_data = static_cast<AudioDataHandle>(file->Read_Entire_And_Close());

    OpenAudioFile open_audio;
    open_audio.audio_event_info = audio_event->Get_Event_Info();

    AILSOUNDINFO sound_info;
    AIL_WAV_info(file_data, &sound_info);

    if (audio_event->Is_Positional_Audio() && sound_info.channels > 1) {
        captainslog_error("Audio marked as positional audio cannot have more than one channel.");
        delete[] file_data;

        return nullptr;
    }

    if (sound_info.format == 17) { // ADPCM, need to decompress.
        AudioDataHandle decomp_data;
        uint32_t decomp_size;
        AIL_decompress_ADPCM(&sound_info, &decomp_data, &decomp_size);
        file_size = decomp_size;
        open_audio.miles_allocated = true;
        open_audio.wave_data = decomp_data;
        delete[] file_data;
    } else {
        if (sound_info.format != 1) { // Must be PCM otherwise.
            captainslog_error(
                "Audio file '%s' is not PCM or ADPCM and is unsupported by the MSS based audio engine.", filename.Str());
            delete[] file_data;

            return nullptr;
        }

        open_audio.miles_allocated = false;
        open_audio.wave_data = file_data;
    }

    static_assert(sizeof(open_audio.info) == sizeof(sound_info), "Expected equal");
    memcpy(&open_audio.info, &sound_info, sizeof(sound_info));
    open_audio.data_size = file_size;
    open_audio.ref_count = 1;
    m_currentSize += open_audio.data_size;

    // m_maxSize prevents using overly large amounts of memory, so if we are over it, unload some other samples.
    if (m_currentSize > m_maxSize && !Free_Space_For_Sample(open_audio)) {
        m_currentSize -= open_audio.data_size;
        Release_Open_Audio(&open_audio);

        return nullptr;
    }

    m_cacheMap[filename] = open_audio;

    return open_audio.wave_data;
}

/**
 * Closes a file, reducing the references to it. Does not actually free the cache.
 *
 * 0x007813D0
 */
void MilesAudioFileCache::Close_File(AudioDataHandle file)
{
    if (file == nullptr) {
        return;
    }

    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        if (it->second.wave_data == file) {
            --it->second.ref_count;

            break;
        }
    }
}

/**
 * Sets the maximum amount of memory in bytes that the cache should use.
 */
void MilesAudioFileCache::Set_Max_Size(unsigned size)
{
    ScopedMutexClass lock(&m_mutex);
    m_maxSize = size;
}

/**
 * Attempts to free space for a file by releasing files with no references and lower priority sounds.
 *
 * 0x007814D0
 */
bool MilesAudioFileCache::Free_Space_For_Sample(const OpenAudioFile &file)
{
    captainslog_assert(m_currentSize >= m_maxSize); // Assumed to be called only when we need more than allowed.
    std::list<Utf8String> to_free;
    unsigned required = m_currentSize - m_maxSize;
    unsigned freed = 0;

    // First check for samples that don't have any references.
    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        if (it->second.ref_count == 0) {
            to_free.push_back(it->first);
            freed += it->second.data_size;

            if (freed >= required) {
                break;
            }
        }
    }

    // If we still don't have enough potential space freed up, look for lower priority sounds to remove.
    if (freed < required) {
        for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
            if (it->second.ref_count != 0
                && it->second.audio_event_info->Get_Priority() < file.audio_event_info->Get_Priority()) {
                to_free.push_back(it->first);
                freed += it->second.data_size;

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

    for (auto it = to_free.begin(); it != to_free.end(); ++it) {
        auto to_remove = m_cacheMap.find(*it);

        if (to_remove != m_cacheMap.end()) {
            Release_Open_Audio(&to_remove->second);
            m_currentSize -= to_remove->second.data_size;
            m_cacheMap.erase(to_remove);
        }
    }

    return true;
}

/**
 * Closes any playing instances of an audio file and then frees the memory for it.
 */
void MilesAudioFileCache::Release_Open_Audio(OpenAudioFile *file)
{
    // Close any playing samples that use this data.
    if (file->ref_count != 0) {
        g_theAudio->Close_Any_Sample_Using_File(file->wave_data);
    }

    // Deallocate the data buffer depending on how it was allocated.
    if (file->wave_data != nullptr) {
        if (file->miles_allocated) {
            AIL_mem_free_lock(file->wave_data);
        } else {
            delete[] file->wave_data;
        }

        file->wave_data = nullptr;
        file->audio_event_info = nullptr;
    }
}
