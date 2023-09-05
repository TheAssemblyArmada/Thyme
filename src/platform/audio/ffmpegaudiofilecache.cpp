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

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "audioeventrts.h"
#include "audiomanager.h"
#include "ffmpegaudiofilecache.h"
#include "filesystem.h"
#include <captainslog.h>
#include <list>

namespace Thyme
{
struct WavHeader
{
    uint8_t riff_id[4] = { 'R', 'I', 'F', 'F' };
    uint32_t chunk_size;
    uint8_t wave_id[4] = { 'W', 'A', 'V', 'E' };
    /* "fmt" sub-chunk */
    uint8_t fmt_id[4] = { 'f', 'm', 't', ' ' }; // FMT header
    uint32_t subchunk1_size = 16; // Size of the fmt chunk
    uint16_t audio_format = 1; // Audio format 1=PCM
    uint16_t channels = 1; // Number of channels 1=Mono 2=Sterio
    uint32_t samples_per_sec = 16000; // Sampling Frequency in Hz
    uint32_t bytes_per_sec = 16000 * 2; // bytes per second
    uint16_t block_align = 2; // 2=16-bit mono, 4=16-bit stereo
    uint16_t bits_per_sample = 16; // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t subchunk2_id[4] = { 'd', 'a', 't', 'a' }; // "data"  string
    uint32_t subchunk2_size; // Sampled data length
};

FFmpegAudioFileCache::~FFmpegAudioFileCache()
{
    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        Release_Open_Audio(&it->second);
    }
}

/**
 * Decode the input data and append it to our wave data stream
 */
bool FFmpegAudioFileCache::Decode_FFmpeg(FFmpegOpenAudioFile *file)
{
    auto on_frame = [](AVFrame *frame, int stream_idx, int stream_type, void *user_data) {
        FFmpegOpenAudioFile *file = static_cast<FFmpegOpenAudioFile *>(user_data);
        if (stream_type != AVMEDIA_TYPE_AUDIO) {
            captainslog_warn(
                "Skipping non-audio data inside audioevent: %s", file->audio_event_info->Get_Event_Name().Str());
            return;
        }

        if (av_sample_fmt_is_planar(static_cast<AVSampleFormat>(frame->format))) {
            captainslog_error("Not supporting planar audio samples yet");
            return;
        }

        const int frame_data_size = file->ffmpeg_file->Get_Size_For_Samples(frame->nb_samples);
        file->wave_data = static_cast<uint8_t *>(av_realloc(file->wave_data, file->data_size + frame_data_size));
        memcpy(file->wave_data + file->data_size, frame->data[0], frame_data_size);
        file->data_size += frame_data_size;
        file->total_samples += frame->nb_samples;
    };

    file->ffmpeg_file->Set_Frame_Callback(on_frame);
    file->ffmpeg_file->Set_User_Data(file);

    // Read all packets inside the file
    while (file->ffmpeg_file->Decode_Packet()) {
    }

    // Calculate the duration in MS
    file->duration = (file->total_samples / (float)file->ffmpeg_file->Get_Sample_Rate()) * 1000.0f;

    return true;
}

/**
 * Write a wav header in front of our existing audio data.
 */
void FFmpegAudioFileCache::Fill_Wave_Data(FFmpegOpenAudioFile *open_audio)
{
    WavHeader wav;
    wav.chunk_size = open_audio->data_size - (offsetof(WavHeader, chunk_size) + sizeof(uint32_t));
    wav.subchunk2_size = open_audio->data_size - (offsetof(WavHeader, subchunk2_size) + sizeof(uint32_t));
    wav.channels = open_audio->ffmpeg_file->Get_Num_Channels();
    wav.bits_per_sample = open_audio->ffmpeg_file->Get_Bytes_Per_Sample() * 8;
    wav.samples_per_sec = open_audio->ffmpeg_file->Get_Sample_Rate();
    wav.bytes_per_sec = open_audio->ffmpeg_file->Get_Sample_Rate() * open_audio->ffmpeg_file->Get_Num_Channels()
        * open_audio->ffmpeg_file->Get_Bytes_Per_Sample();
    wav.block_align = open_audio->ffmpeg_file->Get_Num_Channels() * open_audio->ffmpeg_file->Get_Bytes_Per_Sample();
    memcpy(open_audio->wave_data, &wav, sizeof(WavHeader));
}

/**
 * Parse wave data and return the parameters
 */
void FFmpegAudioFileCache::Get_Wave_Data(
    AudioDataHandle wave_data, uint8_t *&data, uint32_t &size, uint32_t &freq, uint8_t &channels, uint8_t &bits_per_sample)
{
    WavHeader *header = reinterpret_cast<WavHeader *>(wave_data);
    data = static_cast<uint8_t *>(wave_data) + sizeof(WavHeader);

    size = header->subchunk2_size;
    freq = header->samples_per_sec;
    channels = header->channels;
    bits_per_sample = header->bits_per_sample;
}

/**
 * Opens an audio file. Reads from the cache if available or loads from file if not.
 */
AudioDataHandle FFmpegAudioFileCache::Open_File(const Utf8String &filename)
{
    ScopedMutexClass lock(&m_mutex);

    captainslog_trace("FFmpegAudioFileCache: opening file %s", filename.Str());

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

    FFmpegOpenAudioFile open_audio;
    open_audio.wave_data = static_cast<uint8_t *>(av_malloc(sizeof(WavHeader)));
    open_audio.data_size = sizeof(WavHeader);
    open_audio.ffmpeg_file = new FFmpegFile();

    // This transfer ownership of file
    if (!open_audio.ffmpeg_file->Open(file)) {
        captainslog_warn("Failed to load audio file '%s', could not cache.", filename.Str());
        Release_Open_Audio(&open_audio);
        return nullptr;
    }

    if (!Decode_FFmpeg(&open_audio)) {
        captainslog_warn("Failed to decode audio file '%s', could not cache.", filename.Str());
        Release_Open_Audio(&open_audio);
        return nullptr;
    }

    Fill_Wave_Data(&open_audio);
    open_audio.ffmpeg_file->Close();

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
AudioDataHandle FFmpegAudioFileCache::Open_File(AudioEventRTS *audio_event)
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

    captainslog_trace("FFmpegAudioFileCache: opening file %s", filename.Str());

    // Try to find existing data for this file to avoid loading it if unneeded.
    auto it = m_cacheMap.find(filename);

    if (it != m_cacheMap.end()) {
        ++(it->second.ref_count);

        return static_cast<AudioDataHandle>(it->second.wave_data);
    }

    // Load the file from disk
    File *file = g_theFileSystem->Open_File(filename.Str(), File::READ | File::BINARY | File::BUFFERED);

    if (file == nullptr) {
        if (!filename.Is_Empty()) {
            captainslog_warn("Missing audio file '%s', could not cache.", filename.Str());
        }

        return nullptr;
    }

    FFmpegOpenAudioFile open_audio;
    open_audio.wave_data = static_cast<uint8_t *>(av_malloc(sizeof(WavHeader)));
    open_audio.data_size = sizeof(WavHeader);
    open_audio.audio_event_info = audio_event->Get_Event_Info();
    open_audio.ffmpeg_file = new FFmpegFile();

    if (!open_audio.ffmpeg_file->Open(file)) {
        captainslog_warn("Failed to load audio file '%s', could not cache.", filename.Str());
        return nullptr;
    }

    if (audio_event->Is_Positional_Audio() && open_audio.ffmpeg_file->Get_Num_Channels() > 1) {
        captainslog_error("Audio marked as positional audio cannot have more than one channel.");
        return nullptr;
    }

    if (!Decode_FFmpeg(&open_audio)) {
        captainslog_warn("Failed to decode audio file '%s', could not cache.", filename.Str());
        open_audio.ffmpeg_file->Close();
        return nullptr;
    }

    Fill_Wave_Data(&open_audio);
    open_audio.ffmpeg_file->Close();

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
 * Closes a file, reducing the references to it. Does not actually free the cache.
 */
void FFmpegAudioFileCache::Close_File(AudioDataHandle file)
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
 * Get the length of the file in MS.
 */
float FFmpegAudioFileCache::Get_File_Length_MS(AudioDataHandle file) const
{
    if (file == nullptr) {
        return 0.0f;
    }

    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        if (static_cast<AudioDataHandle>(it->second.wave_data) == file) {
            return it->second.duration;
        }
    }

    return 0.0f;
}

/**
 * Sets the maximum amount of memory in bytes that the cache should use.
 */
void FFmpegAudioFileCache::Set_Max_Size(unsigned size)
{
    ScopedMutexClass lock(&m_mutex);
    m_maxSize = size;
}

/**
 * Attempts to free space by releasing files with no references
 */
unsigned FFmpegAudioFileCache::Free_Space(unsigned required)
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
bool FFmpegAudioFileCache::Free_Space_For_Sample(const FFmpegOpenAudioFile &file)
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

/**
 * Closes any playing instances of an audio file and then frees the memory for it.
 */
void FFmpegAudioFileCache::Release_Open_Audio(FFmpegOpenAudioFile *open_audio)
{
    // Close any playing samples that use this data.
    if (open_audio->ref_count != 0 && g_theAudio) {
        g_theAudio->Close_Any_Sample_Using_File(open_audio->wave_data);
    }

    // Close FFmpeg contexts
    if (open_audio->ffmpeg_file != nullptr) {
        delete open_audio->ffmpeg_file;
        open_audio->ffmpeg_file = nullptr;
    }

    // Deallocate the data buffer depending on how it was allocated.
    if (open_audio->wave_data != nullptr) {
        av_freep(&open_audio->wave_data);
        open_audio->audio_event_info = nullptr;
    }
}
} // namespace Thyme
