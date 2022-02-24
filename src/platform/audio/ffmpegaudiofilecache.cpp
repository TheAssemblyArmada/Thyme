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
#include <libavformat/avformat.h>
}

#include "audioeventrts.h"
#include "audiomanager.h"
#include "ffmpegaudiofilecache.h"
#include "filesystem.h"
#include <captainslog.h>
#include <list>

FFmpegAudioFileCache::~FFmpegAudioFileCache()
{
    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        Release_Open_Audio(&it->second);
    }
}

/**
 * Open all the required FFmpeg handles for a required file.
 */
bool FFmpegAudioFileCache::Open_FFmpeg_Contexts(FFmpegOpenAudioFile *file, unsigned char *file_data, uint32_t file_size)
{
    // FFmpeg setup
    int ret = 0;
    file->fmt_ctx = avformat_alloc_context();
    if (!file->fmt_ctx) {
        captainslog_error("Failed to alloc AVFormatContext");
        return false;
    }
    AVIOContext *avio_ctx = avio_alloc_context((unsigned char *)file_data, file_size, 0, nullptr, nullptr, nullptr, nullptr);
    if (!avio_ctx) {
        captainslog_error("Failed to alloc AVIOContext");
        avformat_close_input(&file->fmt_ctx);
        return false;
    }

    file->fmt_ctx->pb = avio_ctx;
    ret = avformat_open_input(&file->fmt_ctx, NULL, NULL, NULL);
    if (ret < 0) {
        captainslog_error("Failed to open audiofile with FFmpeg");
        avformat_close_input(&file->fmt_ctx);
        av_freep(&avio_ctx);
        return false;
    }

    ret = avformat_find_stream_info(file->fmt_ctx, NULL);
    if (ret < 0) {
        captainslog_error("Failed to find stream info");
        avformat_close_input(&file->fmt_ctx);
        av_freep(&avio_ctx);
        return false;
    }

    if (file->fmt_ctx->nb_streams != 1) {
        captainslog_error("Expected exactly one audio stream per file");
        avformat_close_input(&file->fmt_ctx);
        av_freep(&avio_ctx);
        return false;
    }

    AVCodec *input_codec = avcodec_find_decoder(file->fmt_ctx->streams[0]->codecpar->codec_id);
    if (!input_codec) {
        captainslog_error("Audio codec not supported: '%u'", file->fmt_ctx->streams[0]->codecpar->codec_tag);
        avformat_close_input(&file->fmt_ctx);
        av_freep(&avio_ctx);
        return false;
    }

    ret = avcodec_open2(file->fmt_ctx->streams[0]->codec, input_codec, NULL);
    if (ret < 0) {
        captainslog_error("Failed to open input codec");
        avformat_close_input(&file->fmt_ctx);
        av_freep(&avio_ctx);
        return false;
    }

    file->codec_ctx = file->fmt_ctx->streams[0]->codec;
    return true;
}

/**
 * Opens an audio file for an event. Reads from the cache if available or loads from file if not.
 */
void *FFmpegAudioFileCache::Open_File(AudioEventRTS *audio_event)
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
        ++it->second.ref_count;

        return it->second.wave_data;
    }

    // Load the file from disk
    File *file = g_theFileSystem->Open(filename, File::READ | File::BINARY);

    if (file == nullptr) {
        if (!filename.Is_Empty()) {
            captainslog_warn("Missing audio file '%s', could not cache.", filename.Str());
        }

        return nullptr;
    }

    uint32_t file_size = file->Size();
    uint8_t *file_data = static_cast<uint8_t *>(file->Read_All_And_Close());

    FFmpegOpenAudioFile open_audio;
    open_audio.data_size = 0;
    open_audio.audio_event_info = audio_event->Get_Event_Info();

    if (!Open_FFmpeg_Contexts(&open_audio, (unsigned char *)file_data, file_size)) {
        captainslog_warn("Failed to load audio file '%s', could not cache.", filename.Str());
        return nullptr;
    }

    if (audio_event->Is_Positional_Audio() && open_audio.codec_ctx->channels > 1) {
        captainslog_error("Audio marked as positional audio cannot have more than one channel.");
        delete[] file_data;

        return nullptr;
    }

    AVPacket packet;
    AVFrame frame;

    int got_frame = 0;
    while (av_read_frame(open_audio.fmt_ctx, &packet) >= 0) {
        int len = avcodec_decode_audio4(open_audio.codec_ctx, &frame, &got_frame, &packet);

        if (got_frame) {
            int frame_data_size = av_samples_get_buffer_size(
                NULL, open_audio.codec_ctx->channels, frame.nb_samples, open_audio.codec_ctx->sample_fmt, 1);
            av_realloc(open_audio.wave_data, open_audio.data_size + frame_data_size);
            memcpy(open_audio.wave_data + open_audio.data_size, frame.data[0], frame_data_size);
            open_audio.data_size += frame_data_size;
        }
    }

    av_free_packet(&packet);

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
 */
void FFmpegAudioFileCache::Close_File(void *file)
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
void FFmpegAudioFileCache::Set_Max_Size(unsigned size)
{
    ScopedMutexClass lock(&m_mutex);
    m_maxSize = size;
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
void FFmpegAudioFileCache::Release_Open_Audio(FFmpegOpenAudioFile *file)
{
    // Close any playing samples that use this data.
    if (file->ref_count) {
        g_theAudio->Close_Any_Sample_Using_File(file);
    }

    // Deallocate the data buffer depending on how it was allocated.
    if (file->wave_data) {
        av_free(file->wave_data);

        file->wave_data = nullptr;
        file->audio_event_info = nullptr;
    }
}
