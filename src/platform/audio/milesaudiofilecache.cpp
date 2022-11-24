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
#include <captainslog.h>
#include <list>

#include <miles.h>

namespace Thyme
{
struct MilesContext
{
    bool miles_allocated;
};
} // namespace Thyme

using namespace Thyme;

/**
 * Clear all remaining open audio files
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
 */
bool MilesAudioFileCache::Load_File(File *file, OpenAudioFile &open_audio)
{
    Utf8String filename = file->Get_Name();
    uint32_t file_size = file->Size();
    AudioDataHandle file_data = static_cast<AudioDataHandle>(file->Read_Entire_And_Close());
    MilesContext *miles_ctx = static_cast<MilesContext *>(malloc(sizeof(MilesContext)));
    open_audio.opaque = miles_ctx;

    AILSOUNDINFO sound_info;
    AIL_WAV_info(file_data, &sound_info);

    if (sound_info.format == WAVE_FORMAT_IMA_ADPCM) { // ADPCM, need to decompress.
        AudioDataHandle decomp_data;
        uint32_t decomp_size;
        AIL_decompress_ADPCM(&sound_info, &decomp_data, &decomp_size);
        file_size = decomp_size;
        miles_ctx->miles_allocated = true;
        open_audio.wave_data = decomp_data;
        delete[] file_data;
    } else {
        if (sound_info.format != WAVE_FORMAT_PCM) { // Must be PCM otherwise.
            captainslog_error(
                "Audio file '%s' is not PCM or ADPCM and is unsupported by the MSS based audio engine.", filename.Str());
            delete[] file_data;

            return false;
        }

        miles_ctx->miles_allocated = false;
        open_audio.wave_data = file_data;
    }

    open_audio.data_size = file_size;
    open_audio.ref_count = 1;
    m_currentSize += open_audio.data_size;

    // m_maxSize prevents using overly large amounts of memory, so if we are over it, unload some other samples.
    if (m_currentSize > m_maxSize && !Free_Space_For_Sample(open_audio)) {
        m_currentSize -= open_audio.data_size;
        Release_Open_Audio(&open_audio);

        return false;
    }

    m_cacheMap[filename] = open_audio;

    return true;
}

/**
 * Closes any playing instances of an audio file and then frees the memory for it.
 */
void MilesAudioFileCache::Release_Open_Audio(OpenAudioFile *file)
{
    MilesContext *mss_ctx = static_cast<MilesContext *>(file->opaque);

    // Close any playing samples that use this data.
    if (file->ref_count != 0) {
        g_theAudio->Close_Any_Sample_Using_File(file->wave_data);
    }

    // Deallocate the data buffer depending on how it was allocated.
    if (file->wave_data != nullptr) {
        if (mss_ctx && mss_ctx->miles_allocated) {
            AIL_mem_free_lock(file->wave_data);
        } else {
            delete[] file->wave_data;
        }

        file->wave_data = nullptr;
        file->audio_event_info = nullptr;
    }

    if (file->opaque) {
        free(file->opaque);
        file->opaque = nullptr;
    }
}
