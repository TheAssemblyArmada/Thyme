/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Video stream implementation using Bink video.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "binkvideostream.h"
#include "videobuffer.h"
#include <bink.h>

/**
 * 0x007AA9F0
 */
BinkVideoStream::~BinkVideoStream()
{
    if (m_binkHandle != nullptr) {
        BinkClose(m_binkHandle);
        m_binkHandle = nullptr;
    }
}

/**
 * Update the stream state.
 *
 * 0x007AAA20
 */
void BinkVideoStream::Update()
{
    BinkWait(m_binkHandle);
}

/**
 * Check if a frame is ready to be drawn.
 *
 * 0x007AAA30
 */
bool BinkVideoStream::Is_Frame_Ready()
{
    return BinkWait(m_binkHandle) == 0;
}

/**
 * Decompress a frame ready to render to a video buffer.
 *
 * 0x007AAA40
 */
void BinkVideoStream::Decompress_Frame()
{
    BinkDoFrame(m_binkHandle);
}

/**
 * Render a frame to a video buffer.
 *
 * 0x007AAA50
 */
void BinkVideoStream::Render_Frame(VideoBuffer *buffer)
{
    if (buffer == nullptr) {
        return;
    }

    void *dst = buffer->Lock();
    int bink_format;

    switch (buffer->Get_Format()) {
        case VideoBuffer::TYPE_R8G8B8:
            bink_format = 1;
            break;
        case VideoBuffer::TYPE_X8R8G8B8:
            bink_format = 3;
            break;
        case VideoBuffer::TYPE_R5G6B5:
            bink_format = 10;
            break;
        case VideoBuffer::TYPE_X1R5G5B5:
            bink_format = 9;
            break;
        default:
            return;
    }

    if (dst != nullptr) {
        BinkCopyToBuffer(m_binkHandle,
            dst,
            buffer->Get_Pitch(),
            buffer->Get_Height(),
            buffer->Get_XPos(),
            buffer->Get_YPos(),
            bink_format);
        buffer->Unlock();
    }
}

/**
 * Move to next frame.
 *
 * 0x007AAAD0
 */
void BinkVideoStream::Next_Frame()
{
    return BinkNextFrame(m_binkHandle);
}

/*
bool BinkVideoStream::Is_Done()
{
    return BinkIsPlaying(m_binkHandle) == 0;
}
*/

/**
 * Get the current frame index.
 *
 * 0x007AAAE0
 */
int BinkVideoStream::Frame_Index()
{
    return m_binkHandle->frame_num - 1;
}

/**
 * Get the number of frames in the video.
 *
 * 0x007AAAF0
 */
int BinkVideoStream::Frame_Count()
{
    return m_binkHandle->frames;
}

/**
 * Goto a specific frame.
 *
 * 0x007AAB00
 */
void BinkVideoStream::Goto_Frame(int frame)
{
    BinkGoto(m_binkHandle, frame, 0);
}

/**
 * Get the video height.
 *
 * 0x007AAB20
 */
int BinkVideoStream::Height()
{
    return m_binkHandle->height;
}

/**
 * Get the video width.
 *
 * 0x007AAB30
 */
int BinkVideoStream::Width()
{
    return m_binkHandle->width;
}
