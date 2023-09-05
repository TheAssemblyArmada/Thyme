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

FFmpegVideoStream::FFmpegVideoStream(FFmpegFile *file) : m_ffmpegFile(file)
{
    captainslog_assert(m_ffmpegFile != nullptr);
    file->Set_Frame_Callback(On_Frame);
    file->Set_User_Data(this);
    // Decode until we have our first video frame
    while (m_good && m_got_frame == false)
        m_good = m_ffmpegFile->Decode_Packet();
}

FFmpegVideoStream::~FFmpegVideoStream()
{
    av_frame_free(&m_frame);
    sws_freeContext(m_swsContext);
    delete m_ffmpegFile;
}

void FFmpegVideoStream::On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data)
{
    FFmpegVideoStream *stream = static_cast<FFmpegVideoStream *>(user_data);
    if (stream_type == AVMEDIA_TYPE_VIDEO) {
        av_frame_free(&stream->m_frame);
        stream->m_frame = av_frame_clone(frame);
        stream->m_got_frame = true;
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
    return m_frame != nullptr;
}

/**
 * Decompress a frame ready to render to a video buffer.
 */
void FFmpegVideoStream::Decompress_Frame()
{
    // Our frames are always decoded / decompressed. We might want to change at some point
}

/**
 * Render a frame to a video buffer.
 */
void FFmpegVideoStream::Render_Frame(VideoBuffer *buffer)
{
    if (buffer == nullptr) {
        return;
    }

    if (m_frame == nullptr) {
        return;
    }

    if (m_frame->data == nullptr) {
        return;
    }

    AVPixelFormat dst_pix_fmt;

    switch (buffer->Get_Format()) {
        case VideoBuffer::TYPE_R8G8B8:
            dst_pix_fmt = AV_PIX_FMT_RGB24;
            break;
        case VideoBuffer::TYPE_X8R8G8B8:
            dst_pix_fmt = AV_PIX_FMT_BGR0;
            break;
        case VideoBuffer::TYPE_R5G6B5:
            dst_pix_fmt = AV_PIX_FMT_RGB565;
            break;
        case VideoBuffer::TYPE_X1R5G5B5:
            dst_pix_fmt = AV_PIX_FMT_RGB555;
            return;
        default:
            return;
    }

    m_swsContext = sws_getCachedContext(m_swsContext,
        Width(),
        Height(),
        static_cast<AVPixelFormat>(m_frame->format),
        buffer->Get_Width(),
        buffer->Get_Height(),
        dst_pix_fmt,
        SWS_BICUBIC,
        NULL,
        NULL,
        NULL);

    uint8_t *buffer_data = static_cast<uint8_t *>(buffer->Lock());
    if (buffer_data == nullptr) {
        captainslog_error("Failed to lock videobuffer");
        return;
    }

    int dst_strides[] = { (int)buffer->Get_Pitch() };
    uint8_t *dst_data[] = { buffer_data };
    int result = sws_scale(m_swsContext, m_frame->data, m_frame->linesize, 0, Height(), dst_data, dst_strides);
    if (result < 0) {
        captainslog_error("Failed to write into videobuffer");
    }
    buffer->Unlock();
}

/**
 * Move to next frame.
 */
void FFmpegVideoStream::Next_Frame()
{
    m_got_frame = false;
    // Decode until we have our next video frame
    while (m_good && m_got_frame == false)
        m_good = m_ffmpegFile->Decode_Packet();
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
    // The index starts at 0, while the number starts at 1
    return m_ffmpegFile->Get_Current_Frame() - 1;
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
    m_ffmpegFile->Seek_Frame(frame);
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
