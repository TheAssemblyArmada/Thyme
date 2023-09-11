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
#pragma once

#include "always.h"
#include "filesystem.h"

struct AVFormatContext;
struct AVIOContext;
struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

namespace Thyme
{
typedef void (*FFmpegFrameCallback)(AVFrame *frame, int stream_idx, int stream_type, void *user_data);

class FFmpegFile
{
public:
    FFmpegFile();
    FFmpegFile(File *file);
    ~FFmpegFile();

    bool Open(File *file);
    void Close();
    void Set_Frame_Callback(FFmpegFrameCallback callback) { m_frame_callback = callback; }
    void Set_User_Data(void *user_data) { m_user_data = user_data; }
    // Read & decode a packet from the container. Note that we could/should split this step
    bool Decode_Packet();
    void Seek_Frame(int frame_idx);
    bool HasAudio() const;

    // Audio specific
    int Get_Size_For_Samples(int numSamples) const;
    int Get_Num_Channels() const;
    int Get_Sample_Rate() const;
    int Get_Bytes_Per_Sample() const;

    // Video specific
    int Get_Width() const;
    int Get_Height() const;
    int Get_Num_Frames() const;
    int Get_Current_Frame() const;
    int Get_Pixel_Format() const;
    unsigned int Get_Frame_Time() const;

private:
    struct FFmpegStream
    {
        AVCodecContext *codec_ctx = nullptr;
        const AVCodec *codec = nullptr;
        int stream_idx = -1;
        int stream_type = -1;
        AVFrame *frame = nullptr;
    };

    static int Read_Packet(void *opaque, uint8_t *buf, int buf_size);
    const FFmpegStream *Find_Match(int type) const;

    FFmpegFrameCallback m_frame_callback = nullptr;
    AVFormatContext *m_fmt_ctx = nullptr;
    AVIOContext *m_avio_ctx = nullptr;
    AVPacket *m_packet = nullptr;
    std::vector<FFmpegStream> m_streams;
    File *m_file = nullptr;
    void *m_user_data = nullptr;
};
} // namespace Thyme
