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
struct AVCodecContext;
struct AVFrame;
struct AVPacket;

namespace Thyme
{
class FFmpegFile
{
public:
    FFmpegFile();
    FFmpegFile(File *file);
    ~FFmpegFile();

    bool Open(File *file);
    void Close();
    bool Decode_Frame(AVFrame *&out);

    // Audio specific
    int Get_Size_For_Samples(int numSamples);
    int Get_Num_Channels();
    int Get_Sample_Rate();
    int Get_Bytes_Per_Sample();

    // Video specific
    int Get_Width();
    int Get_Height();
    int Get_Num_Frames();
    int Get_Current_Frame();
    int Get_Pixel_Format();

private:
    static int Read_Packet(void *opaque, uint8_t *buf, int buf_size);
    bool Read_Packet();

    AVFormatContext *m_fmt_ctx = nullptr;
    AVIOContext *m_avio_ctx = nullptr;
    AVCodecContext *m_codec_ctx = nullptr;
    AVPacket *m_packet = nullptr;
    AVFrame *m_frame = nullptr;
    File *m_file = nullptr;
};
} // namespace Thyme
