extern "C" {
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

#include "ffmpegfile.h"
#include "ffmpegvideoplayer.h"
#include "ffmpegvideostream.h"
#include "videobuffer.h"

#ifdef BUILD_WITH_OPENAL
#include "alaudiomanager.h"
#include "alaudiostream.h"
#endif

#include <captainslog.h>

namespace Thyme
{

FFmpegVideoStream::FFmpegVideoStream(VideoPlayer *player, VideoStream *next, FFmpegFile *file) : m_ffmpegFile(file)
{
    m_player = player;
    captainslog_assert(m_player != nullptr);
#ifdef BUILD_WITH_OPENAL
    if (m_ffmpegFile->HasAudio())
        m_audioStream = new ALAudioStream();
#endif
    m_next = next;
    file->Set_Frame_Callback(On_Frame);
    file->Set_User_Data(this);
    // Decode until we have our first video frame
    while (m_good && m_gotFrame == false)
        m_good = m_ffmpegFile->Decode_Packet();

    m_startTime = rts::Get_Time();
#ifdef BUILD_WITH_OPENAL
    // Start audio playback, if there is any audio
    if (m_audioStream != nullptr)
        m_audioStream->Play();
#endif
}

FFmpegVideoStream::~FFmpegVideoStream()
{
    av_freep(&m_audio_buffer);
    av_frame_free(&m_frame);
    sws_freeContext(m_swsContext);
    delete m_ffmpegFile;
#ifdef BUILD_WITH_OPENAL
    delete m_audioStream;
#endif
}

void FFmpegVideoStream::On_Frame(AVFrame *frame, int stream_idx, int stream_type, void *user_data)
{
    FFmpegVideoStream *video_stream = static_cast<FFmpegVideoStream *>(user_data);
    if (stream_type == AVMEDIA_TYPE_VIDEO) {
        av_frame_free(&video_stream->m_frame);
        video_stream->m_frame = av_frame_clone(frame);
        video_stream->m_gotFrame = true;
    }
#ifdef BUILD_WITH_OPENAL
    else if (stream_type == AVMEDIA_TYPE_AUDIO) {
        video_stream->m_audioStream->Update();
        AVSampleFormat sample_fmt = static_cast<AVSampleFormat>(frame->format);
        const int bytes_per_sample = av_get_bytes_per_sample(sample_fmt);
        const int frame_size = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, sample_fmt, 1);
        uint8_t *frame_data = frame->data[0];
        // The format is planar - convert it to interleaved
        if (av_sample_fmt_is_planar(sample_fmt)) {
            video_stream->m_audio_buffer = static_cast<uint8_t *>(av_realloc(video_stream->m_audio_buffer, frame_size));
            if (video_stream->m_audio_buffer == nullptr) {
                captainslog_error("Failed to allocate audio buffer");
                return;
            }

            // Write the samples into our audio buffer
            for (int sample_idx = 0; sample_idx < frame->nb_samples; sample_idx++) {
                int byte_offset = sample_idx * bytes_per_sample;
                for (int channel_idx = 0; channel_idx < frame->channels; channel_idx++) {
                    uint8_t *dst =
                        &video_stream->m_audio_buffer[byte_offset * frame->channels + channel_idx * bytes_per_sample];
                    uint8_t *src = &frame->data[channel_idx][byte_offset];
                    memcpy(dst, src, bytes_per_sample);
                }
            }
            frame_data = video_stream->m_audio_buffer;
        }

        ALenum format = ALAudioManager::Get_AL_Format(frame->channels, bytes_per_sample * 8);
        video_stream->m_audioStream->BufferData(frame_data, frame_size, format, frame->sample_rate);
    }
#endif
}

/**
 * Update the stream state.
 */
void FFmpegVideoStream::Update()
{
#ifdef BUILD_WITH_OPENAL
    if (m_audioStream)
        m_audioStream->Update();
#endif
}

/**
 * Check if a frame is ready to be drawn.
 */
bool FFmpegVideoStream::Is_Frame_Ready()
{
    unsigned int time = rts::Get_Time();
    bool ready = (time - m_startTime) >= m_ffmpegFile->Get_Frame_Time() * Frame_Index();
    return ready;
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
            break;
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
    m_gotFrame = false;
    // Decode until we have our next video frame
    while (m_good && m_gotFrame == false)
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
