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

    size_t avio_ctx_buffer_size = 0x10000;
    uint8_t *buffer = static_cast<uint8_t *>(av_malloc(avio_ctx_buffer_size));
    if (!buffer) {
        captainslog_error("Failed to alloc AVIOContextBuffer");
        Close();
        return false;
    }

    m_avio_ctx = avio_alloc_context(buffer, avio_ctx_buffer_size, 0, file, &Read_Packet, nullptr, nullptr);
    if (!m_avio_ctx) {
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

    if (m_fmt_ctx->nb_streams != 1) {
        captainslog_error("Expected exactly one stream per file");
        Close();
        return false;
    }

    const AVCodec *input_codec = avcodec_find_decoder(m_fmt_ctx->streams[0]->codecpar->codec_id);
    if (!input_codec) {
        captainslog_error("Codec not supported: '%u'", m_fmt_ctx->streams[0]->codecpar->codec_tag);
        Close();
        return false;
    }

    m_codec_ctx = avcodec_alloc_context3(input_codec);
    if (!m_codec_ctx) {
        captainslog_error("Could not allocate codec context");
        Close();
        return false;
    }

    result = avcodec_parameters_to_context(m_codec_ctx, m_fmt_ctx->streams[0]->codecpar);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_parameters_to_context': %s", error_buffer);
        Close();
        return false;
    }

    result = avcodec_open2(m_codec_ctx, input_codec, NULL);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_open2': %s", error_buffer);
        Close();
        return false;
    }

    m_packet = av_packet_alloc();
    m_frame = av_frame_alloc();

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
    if (m_fmt_ctx) {
        avformat_close_input(&m_fmt_ctx);
    }

    if (m_codec_ctx) {
        avcodec_free_context(&m_codec_ctx);
    }

    if (m_avio_ctx && m_avio_ctx->buffer) {
        av_freep(&m_avio_ctx->buffer);
    }

    if (m_avio_ctx) {
        avio_context_free(&m_avio_ctx);
    }

    if (m_packet) {
        av_packet_free(&m_packet);
    }

    if (m_frame) {
        av_frame_free(&m_frame);
    }

    if (m_file) {
        m_file->Close();
        m_file = nullptr;
    }
}

bool FFmpegFile::Read_Packet()
{
    int result = av_read_frame(m_fmt_ctx, m_packet);
    if (result < 0) // EOF
        return false;

    result = avcodec_send_packet(m_codec_ctx, m_packet);
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_send_packet': %s", error_buffer);
        return false;
    }
    av_packet_unref(m_packet);
    return true;
}

bool FFmpegFile::Decode_Frame(AVFrame *&out)
{
    int result = 0;
    // Check if we need more data
    while ((result = avcodec_receive_frame(m_codec_ctx, m_frame)) == AVERROR(EAGAIN)) {
        // This is EOF
        if (!Read_Packet())
            return false;
    }

    // Handle any other errors
    if (result < 0) {
        char error_buffer[1024];
        av_strerror(result, error_buffer, sizeof(error_buffer));
        captainslog_error("Failed 'avcodec_receive_frame': %s", error_buffer);
        return false;
    }

    out = m_frame;
    return true;
}

int FFmpegFile::Get_Num_Channels()
{
    if (!m_codec_ctx)
        return 0;

    return m_codec_ctx->channels;
}

int FFmpegFile::Get_Sample_Rate()
{
    if (!m_codec_ctx)
        return 0;

    return m_codec_ctx->sample_rate;
}

int FFmpegFile::Get_Bytes_Per_Sample()
{
    if (!m_codec_ctx)
        return 0;

    return av_get_bytes_per_sample(m_codec_ctx->sample_fmt);
}

int FFmpegFile::Get_Size_For_Samples(int numSamples)
{
    if (!m_codec_ctx)
        return 0;

    return av_samples_get_buffer_size(NULL, m_codec_ctx->channels, numSamples, m_codec_ctx->sample_fmt, 1);
}

int FFmpegFile::Get_Height()
{
    if (!m_codec_ctx)
        return 0;

    return m_codec_ctx->height;
}

int FFmpegFile::Get_Width()
{
    if (!m_codec_ctx)
        return 0;

    return m_codec_ctx->width;
}

int FFmpegFile::Get_Num_Frames()
{
    if (!m_fmt_ctx || !m_fmt_ctx->streams[0])
        return 0;

    return (m_fmt_ctx->duration / (double)AV_TIME_BASE) * av_q2d(m_fmt_ctx->streams[0]->avg_frame_rate);
}

int FFmpegFile::Get_Current_Frame()
{
    if (!m_fmt_ctx || !m_fmt_ctx->streams[0] || !m_frame || (m_frame->format == -1))
        return 0;
    double timeBase = av_q2d(m_fmt_ctx->streams[0]->time_base);
    return (m_frame->best_effort_timestamp / timeBase) * av_q2d(m_fmt_ctx->streams[0]->avg_frame_rate);
}

int FFmpegFile::Get_Pixel_Format()
{
    if (!m_codec_ctx)
        return AV_PIX_FMT_NONE;

    return m_codec_ctx->pix_fmt;
}

} // namespace Thyme