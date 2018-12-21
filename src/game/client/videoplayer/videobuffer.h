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
#pragma once

#include "always.h"
#include "rect.h"

class VideoBuffer
{
public:
    enum Type
    {
        TYPE_UNKNOWN,
        TYPE_R8G8B8,
        TYPE_X8R8G8B8,
        TYPE_R5G6B5,
        TYPE_X1R5G5B5,
        TYPE_COUNT,
    };

public:
    VideoBuffer(Type type);
    virtual ~VideoBuffer() {}
    virtual bool Allocate(unsigned width, unsigned height) = 0;
    virtual void Free();
    virtual void *Lock() = 0;
    virtual void Unlock() = 0;
    virtual bool Valid() = 0;

    RectClass Rect(float x1, float y1, float x2, float y2);
    unsigned Get_XPos() const { return m_xPos; }
    unsigned Get_YPos() const { return m_yPos; }
    unsigned Get_Width() const { return m_width; }
    unsigned Get_Height() const { return m_height; }
    unsigned Get_Pitch() const { return m_pitch; }
    Type Get_Format() const { return m_format; }

protected:
    unsigned m_xPos;
    unsigned m_yPos;
    unsigned m_width;
    unsigned m_height;
    unsigned m_textureWidth;
    unsigned m_textureHeight;
    unsigned m_pitch;
    Type m_format;
};
