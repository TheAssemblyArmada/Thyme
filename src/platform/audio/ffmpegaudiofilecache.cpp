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

#include "ffmpegaudiofilecache.h"
#include <captainslog.h>
#include <list>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace Thyme
{
struct FFmpegContext
{
    // FFmpeg handles
    AVFormatContext *fmt_ctx = nullptr;
    AVIOContext *avio_ctx = nullptr;
    AVCodecContext *codec_ctx = nullptr;
};

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

} // namespace Thyme

using namespace Thyme;

/**
 * Clear all remaining open audio files
 */
FFmpegAudioFileCache::~FFmpegAudioFileCache()
{
    ScopedMutexClass lock(&m_mutex);

    for (auto it = m_cacheMap.begin(); it != m_cacheMap.end(); ++it) {
        Release_Open_Audio(&it->second);
    }
}

/**
 * Read an FFmpeg packet from file
 */
int FFmpegAudioFileCache::Read_FFmpeg_Packet(void *opaque, uint8_t *buf, int buf_size)
{
    File *file = static_cast<File *>(opaque);
    int read = file->Read(buf, buf_size);

    // Streaming protocol requires us to return real errors - when we read less equal 0 we're at EOF
    if (read <= 0)
        return AVERROR_EOF;

    return read;
}

/**
 * Open all the required FFmpeg handles for a required file.
 */
bool FFmpegAudioFileCache::Open_FFmpeg_Contexts(OpenAudioFile *open_audio, File *file)
{
#if LOGGING_LEVEL != LOGLEVEL_NONE
    av_log_set_level(AV_LOG_INFO);
#endif

// This is required for FFmpeg older than 4.0 -> deprecated afterwards though
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif

    FFmpegContext *ff_ctx = static_cast<FFmpegContext *>(open_audio->opaque);

    // FFmpeg setup
    ff_ctx->fmt_ctx = avformat_alloc_context();
    if (!ff_ctx->fmt_ctx) {
        captainslog_error("Failed to alloc AVFormatContext");
        return false;
    }

    size_t avio_ctx_buffer_size = 0x10000;
    uint8_t *buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (!buffer) {
        captainslog_error("Failed to alloc AVIOContextBuffer");
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    ff_ctx->avio_ctx = avio_alloc_context(buffer, avio_ctx_buffer_size, 0, file, &Read_FFmpeg_Packet, nullptr, nullptr);
    if (!ff_ctx->avio_ctx) {
        captainslog_error("Failed to alloc AVIOContext");
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    ff_ctx->fmt_ctx->pb = ff_ctx->avio_ctx;
    ff_ctx->fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

    int result = 0;
    result = avformat_open_input(&ff_ctx->fmt_ctx, nullptr, nullptr, nullptr);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avformat_open_input': %s", error_buffer);
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    result = avformat_find_stream_info(ff_ctx->fmt_ctx, NULL);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avformat_find_stream_info': %s", error_buffer);
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    if (ff_ctx->fmt_ctx->nb_streams != 1) {
        captainslog_error("Expected exactly one audio stream per file");
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    const AVCodec *input_codec = avcodec_find_decoder(ff_ctx->fmt_ctx->streams[0]->codecpar->codec_id);
    if (!input_codec) {
        captainslog_error("Audio codec not supported: '%u'", ff_ctx->fmt_ctx->streams[0]->codecpar->codec_tag);
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    ff_ctx->codec_ctx = avcodec_alloc_context3(input_codec);
    if (!ff_ctx->codec_ctx) {
        captainslog_error("Could not allocate codec context");
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    result = avcodec_parameters_to_context(ff_ctx->codec_ctx, ff_ctx->fmt_ctx->streams[0]->codecpar);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_parameters_to_context': %s", error_buffer);
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    result = avcodec_open2(ff_ctx->codec_ctx, input_codec, NULL);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_open2': %s", error_buffer);
        Close_FFmpeg_Contexts(open_audio);
        return false;
    }

    return true;
}

/**
 * Decode the input data and append it to our wave data stream
 */
bool FFmpegAudioFileCache::Decode_FFmpeg(OpenAudioFile *file)
{
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    FFmpegContext *ff_ctx = static_cast<FFmpegContext *>(file->opaque);

    int result = 0;

    // Read all packets inside the file
    while (av_read_frame(ff_ctx->fmt_ctx, packet) >= 0) {
        result = avcodec_send_packet(ff_ctx->codec_ctx, packet);
        if (result < 0) {
            char error_buffer[1024];
            av_strerror(result, error_buffer, sizeof(error_buffer));
            captainslog_error("Failed 'avcodec_send_packet': %s", error_buffer);
            return false;
        }
        // Decode all frames contained inside the packet
        while (result >= 0) {
            result = avcodec_receive_frame(ff_ctx->codec_ctx, frame);
            // Check if we need more data
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
                break;
            else if (result < 0) {
                char error_buffer[1024];
                av_strerror(result, error_buffer, sizeof(error_buffer));
                captainslog_error("Failed 'avcodec_receive_frame': %s", error_buffer);
                return false;
            }

            int frame_data_size = av_samples_get_buffer_size(
                NULL, ff_ctx->codec_ctx->channels, frame->nb_samples, ff_ctx->codec_ctx->sample_fmt, 1);
            file->wave_data = static_cast<uint8_t *>(av_realloc(file->wave_data, file->data_size + frame_data_size));
            memcpy(static_cast<uint8_t *>(file->wave_data) + file->data_size, frame->data[0], frame_data_size);
            file->data_size += frame_data_size;
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    av_frame_free(&frame);

    return true;
}

/**
 * Close all the open FFmpeg handles for an open file.
 */
void FFmpegAudioFileCache::Close_FFmpeg_Contexts(OpenAudioFile *open_audio)
{
    FFmpegContext *ff_ctx = static_cast<FFmpegContext *>(open_audio->opaque);
    if (ff_ctx->fmt_ctx) {
        avformat_close_input(&ff_ctx->fmt_ctx);
    }

    if (ff_ctx->codec_ctx) {
        avcodec_free_context(&ff_ctx->codec_ctx);
    }

    if (ff_ctx->avio_ctx && ff_ctx->avio_ctx->buffer) {
        av_freep(&ff_ctx->avio_ctx->buffer);
    }

    if (ff_ctx->avio_ctx) {
        avio_context_free(&ff_ctx->avio_ctx);
    }
}

void FFmpegAudioFileCache::Fill_Wave_Data(OpenAudioFile *open_audio)
{
    FFmpegContext *ff_ctx = static_cast<FFmpegContext *>(open_audio->opaque);
    WavHeader wav;
    wav.chunk_size = open_audio->data_size - (offsetof(WavHeader, chunk_size) + sizeof(uint32_t));
    wav.subchunk2_size = open_audio->data_size - (offsetof(WavHeader, subchunk2_size) + sizeof(uint32_t));
    wav.channels = ff_ctx->codec_ctx->channels;
    wav.bits_per_sample = av_get_bytes_per_sample(ff_ctx->codec_ctx->sample_fmt) * 8;
    wav.samples_per_sec = ff_ctx->codec_ctx->sample_rate;
    wav.bytes_per_sec = ff_ctx->codec_ctx->sample_rate * ff_ctx->codec_ctx->channels * wav.bits_per_sample / 8;
    wav.block_align = ff_ctx->codec_ctx->channels * wav.bits_per_sample / 8;
    memcpy(open_audio->wave_data, &wav, sizeof(WavHeader));
}

/**
 * Load audio data with FFmpeg
 */
bool FFmpegAudioFileCache::Load_File(File *file, OpenAudioFile &open_audio)
{
    Utf8String filename = file->Get_Name();
    open_audio.wave_data = static_cast<uint8_t *>(av_malloc(sizeof(WavHeader)));
    open_audio.data_size = sizeof(WavHeader);
    open_audio.opaque = av_malloc(sizeof(FFmpegContext));

    if (!Open_FFmpeg_Contexts(&open_audio, file)) {
        captainslog_warn("Failed to load audio file '%s', could not cache.", filename.Str());
        Release_Open_Audio(&open_audio);
        file->Close();
        return false;
    }

    if (!Decode_FFmpeg(&open_audio)) {
        captainslog_warn("Failed to decode audio file '%s', could not cache.", filename.Str());
        Close_FFmpeg_Contexts(&open_audio);
        Release_Open_Audio(&open_audio);
        file->Close();
        return false;
    }

    Fill_Wave_Data(&open_audio);
    Close_FFmpeg_Contexts(&open_audio);
    av_freep(&open_audio.opaque);

    return true;
}

/**
 * Closes any playing instances of an audio file and then frees the memory for it.
 */
void FFmpegAudioFileCache::Release_Open_Audio(OpenAudioFile *open_audio)
{
    if (open_audio->opaque) {
        Close_FFmpeg_Contexts(open_audio);
        av_freep(&open_audio->opaque);
    }

    // Close any playing samples that use this data.
    if (open_audio->ref_count != 0 && g_theAudio) {
        g_theAudio->Close_Any_Sample_Using_File(open_audio->wave_data);
    }

    // Deallocate the data buffer depending on how it was allocated.
    if (open_audio->wave_data != nullptr) {
        av_freep(&open_audio->wave_data);
        open_audio->audio_event_info = nullptr;
    }
}
