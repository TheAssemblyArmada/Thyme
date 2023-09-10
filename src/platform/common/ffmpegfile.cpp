/**
 * @file
 *
 * @author feliwir
 *
 * @brief Class for opening FFmpeg contexts from a file.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ffmpegfile.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

#include <captainslog.h>

namespace Thyme
{
FFmpegFile::FFmpegFile() {}

FFmpegFile::FFmpegFile(File *file)
{
    Open(file);
}

FFmpegFile::~FFmpegFile()
{
    Close();
}

bool FFmpegFile::Open(File *file)
{
#if LOGGING_LEVEL != LOGLEVEL_NONE
    av_log_set_level(AV_LOG_INFO);
#endif

// This is required for FFmpeg older than 4.0 -> deprecated afterwards though
#if LIBAVFORMAT_VERSION_MAJOR < 58
    av_register_all();
#endif

    m_file = file;

    // FFmpeg setup
    m_fmt_ctx = avformat_alloc_context();
    if (!m_fmt_ctx) {
        captainslog_error("Failed to alloc AVFormatContext");
        return false;
    }

    constexpr size_t avio_ctx_buffer_size = 0x10000;
    uint8_t *buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (buffer == nullptr) {
        captainslog_error("Failed to alloc AVIOContextBuffer");
        Close();
        return false;
    }

    m_avio_ctx = avio_alloc_context(buffer, avio_ctx_buffer_size, 0, file, &Read_Packet, nullptr, nullptr);
    if (m_avio_ctx == nullptr) {
        captainslog_error("Failed to alloc AVIOContext");
        Close();
        return false;
    }

    m_fmt_ctx->pb = m_avio_ctx;
    m_fmt_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

    int result = 0;
    result = avformat_open_input(&m_fmt_ctx, nullptr, nullptr, nullptr);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avformat_open_input': %s", error_buffer);
        Close();
        return false;
    }

    result = avformat_find_stream_info(m_fmt_ctx, NULL);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avformat_find_stream_info': %s", error_buffer);
        Close();
        return false;
    }

    m_streams.resize(m_fmt_ctx->nb_streams);
    for (unsigned int stream_idx = 0; stream_idx < m_fmt_ctx->nb_streams; stream_idx++) {
        AVStream *av_stream = m_fmt_ctx->streams[stream_idx];
        const AVCodec *input_codec = avcodec_find_decoder(av_stream->codecpar->codec_id);
        if (input_codec == nullptr) {
            captainslog_error("Codec not supported: '%s'", avcodec_get_name(av_stream->codecpar->codec_id));
            Close();
            return false;
        }

        AVCodecContext *codec_ctx = avcodec_alloc_context3(input_codec);
        if (codec_ctx == nullptr) {
            captainslog_error("Could not allocate codec context");
            Close();
            return false;
        }

        result = avcodec_parameters_to_context(codec_ctx, av_stream->codecpar);
        if (result < 0) {
            char error_buffer[1024];
            av_strerror(result, error_buffer, sizeof(error_buffer));
            captainslog_error("Failed 'avcodec_parameters_to_context': %s", error_buffer);
            Close();
            return false;
        }

        result = avcodec_open2(codec_ctx, input_codec, NULL);
        if (result < 0) {
            char error_buffer[1024];
            av_strerror(result, error_buffer, sizeof(error_buffer));
            captainslog_error("Failed 'avcodec_open2': %s", error_buffer);
            Close();
            return false;
        }

        FFmpegStream &output_stream = m_streams[stream_idx];
        output_stream.codec_ctx = codec_ctx;
        output_stream.codec = input_codec;
        output_stream.stream_type = input_codec->type;
        output_stream.stream_idx = stream_idx;
        output_stream.frame = av_frame_alloc();
    }

    m_packet = av_packet_alloc();

    return true;
}

/**
 * Read an FFmpeg packet from file
 */
int FFmpegFile::Read_Packet(void *opaque, uint8_t *buf, int buf_size)
{
    File *file = static_cast<File *>(opaque);
    int read = file->Read(buf, buf_size);

    // Streaming protocol requires us to return real errors - when we read less equal 0 we're at EOF
    if (read <= 0)
        return AVERROR_EOF;

    return read;
}

/**
 * Close all the open FFmpeg handles for an open file.
 */
void FFmpegFile::Close()
{
    if (m_fmt_ctx != nullptr) {
        avformat_close_input(&m_fmt_ctx);
    }

    for (auto &stream : m_streams) {
        if (stream.codec_ctx != nullptr) {
            avcodec_free_context(&stream.codec_ctx);
            av_frame_free(&stream.frame);
        }
    }
    m_streams.clear();

    if (m_avio_ctx != nullptr && m_avio_ctx->buffer != nullptr) {
        av_freep(&m_avio_ctx->buffer);
    }

    if (m_avio_ctx != nullptr) {
        avio_context_free(&m_avio_ctx);
    }

    if (m_packet != nullptr) {
        av_packet_free(&m_packet);
    }

    if (m_file != nullptr) {
        m_file->Close();
        m_file = nullptr;
    }
}

bool FFmpegFile::Decode_Packet()
{
    captainslog_assert(m_fmt_ctx != nullptr);
    captainslog_assert(m_packet != nullptr);

    int result = av_read_frame(m_fmt_ctx, m_packet);
    if (result == AVERROR_EOF) // EOF
        return false;

    const int stream_idx = m_packet->stream_index;
    captainslog_assert(m_streams.size() > stream_idx);

    auto &stream = m_streams[stream_idx];
    AVCodecContext *codec_ctx = stream.codec_ctx;
    result = avcodec_send_packet(codec_ctx, m_packet);
    // Check if we need more data
    if (result == AVERROR(EAGAIN))
        return true;

    // Handle any other errors
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_send_packet': %s", error_buffer);
        return false;
    }
    av_packet_unref(m_packet);

    // Get all frames in this packet
    while (result >= 0) {
        result = avcodec_receive_frame(codec_ctx, stream.frame);

        // Check if we need more data
        if (result == AVERROR(EAGAIN))
            return true;

        // Handle any other errors
        if (result < 0) {
            char error_buffer[1024];
            av_strerror(result, error_buffer, sizeof(error_buffer));
            captainslog_error("Failed 'avcodec_receive_frame': %s", error_buffer);
            return false;
        }

        if (m_frame_callback != nullptr) {
            m_frame_callback(stream.frame, stream_idx, stream.stream_type, m_user_data);
        }
    }

    return true;
}

void FFmpegFile::Seek_Frame(int frame_idx)
{
    // Note: not tested, since not used ingame
    for (auto &stream : m_streams) {
        int64_t timestamp = av_q2d(m_fmt_ctx->streams[stream.stream_idx]->time_base) * frame_idx
            * av_q2d(m_fmt_ctx->streams[stream.stream_idx]->avg_frame_rate);
        int result = av_seek_frame(m_fmt_ctx, stream.stream_idx, timestamp, AVSEEK_FLAG_ANY);
        if (result < 0) {
            char error_buffer[1024];
            av_strerror(result, error_buffer, sizeof(error_buffer));
            captainslog_error("Failed 'av_seek_frame': %s", error_buffer);
        }
    }
}

bool FFmpegFile::HasAudio() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
    return stream != nullptr;
}

const FFmpegFile::FFmpegStream *FFmpegFile::Find_Match(int type) const
{
    for (auto &candidate : m_streams) {
        if (candidate.stream_type == type)
            return &candidate;
    }

    return nullptr;
}

int FFmpegFile::Get_Num_Channels() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
    if (stream == nullptr)
        return 0;

    return stream->codec_ctx->channels;
}

int FFmpegFile::Get_Sample_Rate() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
    if (stream == nullptr)
        return 0;

    return stream->codec_ctx->sample_rate;
}

int FFmpegFile::Get_Bytes_Per_Sample() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
    if (stream == nullptr)
        return 0;

    return av_get_bytes_per_sample(stream->codec_ctx->sample_fmt);
}

int FFmpegFile::Get_Size_For_Samples(int numSamples) const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_AUDIO);
    if (stream == nullptr)
        return 0;

    return av_samples_get_buffer_size(NULL, stream->codec_ctx->channels, numSamples, stream->codec_ctx->sample_fmt, 1);
}

int FFmpegFile::Get_Height() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (stream == nullptr)
        return 0;

    return stream->codec_ctx->height;
}

int FFmpegFile::Get_Width() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (stream == nullptr)
        return 0;

    return stream->codec_ctx->width;
}

int FFmpegFile::Get_Num_Frames() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (m_fmt_ctx == nullptr || stream == nullptr || m_fmt_ctx->streams[stream->stream_idx] == nullptr)
        return 0;

    return (m_fmt_ctx->duration / (double)AV_TIME_BASE) * av_q2d(m_fmt_ctx->streams[stream->stream_idx]->avg_frame_rate);
}

int FFmpegFile::Get_Current_Frame() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (m_fmt_ctx == nullptr || stream == nullptr)
        return 0;
    return stream->codec_ctx->frame_number;
}

int FFmpegFile::Get_Pixel_Format() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (stream == nullptr)
        return AV_PIX_FMT_NONE;

    return stream->codec_ctx->pix_fmt;
}

unsigned int FFmpegFile::Get_Frame_Time() const
{
    const FFmpegStream *stream = Find_Match(AVMEDIA_TYPE_VIDEO);
    if (stream == nullptr)
        return 0;
    return 1000u / av_q2d(m_fmt_ctx->streams[stream->stream_idx]->avg_frame_rate);
}

} // namespace Thyme
