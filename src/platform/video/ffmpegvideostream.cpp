#include "ffmpegvideostream.h"
#include "ffmpegfile.h"
#include "videobuffer.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include <captainslog.h>

namespace Thyme
{

FFmpegVideoStream::~FFmpegVideoStream()
{
    if (m_ffmpegFile != nullptr) {
        delete m_ffmpegFile;
        m_ffmpegFile = nullptr;
    }
}

/**
 * Update the stream state.
 */
void FFmpegVideoStream::Update()
{
    // BinkWait(m_binkHandle);
}

/**
 * Check if a frame is ready to be drawn.
 */
bool FFmpegVideoStream::Is_Frame_Ready()
{
    return m_ready;
}

/**
 * Decompress a frame ready to render to a video buffer.
 */
void FFmpegVideoStream::Decompress_Frame()
{
    m_ready = m_ffmpegFile->Decode_Frame(m_frame);
}

/**
 * Render a frame to a video buffer.
 */
void FFmpegVideoStream::Render_Frame(VideoBuffer *buffer)
{
    if (buffer == nullptr) {
        return;
    }

    if (m_frame = nullptr) {
        return;
    }

    AVPixelFormat dst_pix_fmt;

    switch (buffer->Get_Format()) {
        case VideoBuffer::TYPE_R8G8B8:
            dst_pix_fmt = AV_PIX_FMT_RGB24;
            break;
        case VideoBuffer::TYPE_X8R8G8B8:
            dst_pix_fmt = AV_PIX_FMT_ARGB;
            break;
        case VideoBuffer::TYPE_R5G6B5:
            dst_pix_fmt = AV_PIX_FMT_RGB565;
            break;
        case VideoBuffer::TYPE_X1R5G5B5:
            captainslog_error("X1R5G5B5 is not supported by FFMPEG");
            return;
        default:
            return;
    }

    if (m_swsContext == nullptr) {
        m_swsContext = sws_getContext(Width(),
            Height(),
            static_cast<AVPixelFormat>(m_ffmpegFile->Get_Pixel_Format()),
            buffer->Get_Width(),
            buffer->Get_Height(),
            dst_pix_fmt,
            SWS_BICUBIC,
            NULL,
            NULL,
            NULL);
    }

    uint8_t *dst_data = static_cast<uint8_t *>(buffer->Lock());
    if (dst_data == nullptr) {
        return;
    }

    int dst_strides[] = { (int)buffer->Get_Pitch() };
    sws_scale(m_swsContext, m_frame->data, m_frame->linesize, 0, Height(), &dst_data, dst_strides);

    buffer->Unlock();
}

/**
 * Move to next frame.
 */
void FFmpegVideoStream::Next_Frame()
{
    // TODO: ?
}

/*
bool FFmpegVideoStream::Is_Done()
{
    // TODO: ?
}
*/

/**
 * Get the current frame index.
 */
int FFmpegVideoStream::Frame_Index()
{
    return m_ffmpegFile->Get_Current_Frame();
}

/**
 * Get the number of frames in the video.
 */
int FFmpegVideoStream::Frame_Count()
{
    return m_ffmpegFile->Get_Num_Frames();
}

/**
 * Goto a specific frame.
 */
void FFmpegVideoStream::Goto_Frame(int frame)
{
    // TODO: seeking
}

/**
 * Get the video height.
 */
int FFmpegVideoStream::Height()
{
    return m_ffmpegFile->Get_Height();
}

/**
 * Get the video width.
 */
int FFmpegVideoStream::Width()
{
    return m_ffmpegFile->Get_Width();
}

} // namespace Thyme