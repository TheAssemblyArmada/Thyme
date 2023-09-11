/**
 * @file
 *
 * @author feliwir
 *
 * @brief Video stream implementation using FFmpeg.
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
#include "videostream.h"

struct AVFrame;
struct SwsContext;

namespace Thyme
{
class FFmpegFile;
class FFmpegVideoStream final : public VideoStream
{
    friend class FFmpegVideoPlayer;

public:
    FFmpegVideoStream(VideoPlayer *player, VideoStream *next, FFmpegFile *file);
    virtual ~FFmpegVideoStream();

    virtual void Update() override;
    virtual bool Is_Frame_Ready() override;
    virtual void Decompress_Frame() override;
    virtual void Render_Frame(VideoBuffer *buffer) override;
    virtual void Next_Frame() override;
    // bool Is_Done();
    virtual int Frame_Index() override;
    virtual int Frame_Count() override;
    virtual void Goto_Frame(int frame) override;
    virtual int Height() override;
    virtual int Width() override;

private:
    static void On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data);
    AVFrame *m_frame = nullptr;
    SwsContext *m_swsContext = nullptr;
    FFmpegFile *m_ffmpegFile = nullptr;
    bool m_good = true;
    bool m_gotFrame = false;
    unsigned int m_startTime = 0;
    uint8_t *m_audio_buffer = nullptr;
#ifdef BUILD_WITH_OPENAL
    ALAudioStream *m_audioStream = nullptr;
#endif
};
} // namespace Thyme
