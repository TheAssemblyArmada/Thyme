/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class interface for video buffers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */ 
#include "videobuffer.h"

/**
 * 0x0051AA80
 */
VideoBuffer::VideoBuffer(VideoBuffer::Type type) :
    m_xPos(0),
    m_yPos(0),
    m_width(0),
    m_height(0),
    m_textureWidth(0),
    m_textureHeight(0),
    m_pitch(0),
    m_format(type)
{
    if (type >= TYPE_COUNT || type < TYPE_UNKNOWN) {
        m_format = TYPE_UNKNOWN;   
    }
}

/**
 * @brief Frees the buffer. 
 *
 * 0x0051ABA0
 */
void VideoBuffer::Free()
{
    m_width = 0;
    m_height = 0;
    m_textureWidth = 0;
    m_textureHeight = 0;
}

/**
 * @brief Generates the rectangle the video will occupy (I think).
 *
 * 0x0051AAE0
 */
RectClass VideoBuffer::Rect(float x1, float y1, float x2, float y2)
{
    if (Valid()) {
        float width = float(m_width) / float(m_textureWidth);
        float height = float(m_height) / float(m_textureHeight);
        return RectClass(width * x1, height *y1, width * x2, height * y2);
    }

    return RectClass(0.0f, 0.0f, 0.0f, 0.0f);
}
