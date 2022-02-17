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
#include "surfaceclass.h"
#include "texture.h"
#include "textureloader.h"

/**
 * 0x0051AA80
 */
VideoBuffer::VideoBuffer(VideoBuffer::Type type) :
    m_xPos(0), m_yPos(0), m_width(0), m_height(0), m_textureWidth(0), m_textureHeight(0), m_pitch(0), m_format(type)
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
        return RectClass(width * x1, height * y1, width * x2, height * y2);
    }

    return RectClass(0.0f, 0.0f, 0.0f, 0.0f);
}

W3DVideoBuffer::W3DVideoBuffer(Type type) : VideoBuffer(type), m_texture(nullptr), m_surface(nullptr) {}

W3DVideoBuffer::~W3DVideoBuffer()
{
    Free();
}

bool W3DVideoBuffer::Allocate(unsigned width, unsigned height)
{
    Free();
    m_width = width;
    m_height = height;
    m_textureWidth = width;
    m_textureHeight = height;
    unsigned int volume = 1;
    TextureLoader::Validate_Texture_Size(m_textureWidth, m_textureHeight, volume);
    WW3DFormat format = Type_To_W3D_Format(m_format);

    if (!format) {
        return false;
    }

    m_texture = new TextureClass(m_textureWidth, m_textureHeight, format, MIP_LEVELS_1, POOL_MANAGED, false, true);

    if (m_texture == nullptr) {
        return false;
    }

    if (Lock() != nullptr) {
        Unlock();
        return true;
    } else {
        Unlock();
        return false;
    }
}

void W3DVideoBuffer::Free()
{
    Unlock();

    if (m_texture != nullptr) {
        Unlock();
        m_texture->Release_Ref();
        m_texture = nullptr;
    }

    m_surface = nullptr;
    VideoBuffer::Free();
}

void *W3DVideoBuffer::Lock()
{
    void *ptr = nullptr;

    if (m_surface != nullptr) {
        Unlock();
    }

    m_surface = m_texture->Get_Surface_Level(0);

    if (m_surface != nullptr) {
        return m_surface->Lock(reinterpret_cast<int *>(&m_pitch));
    }

    return ptr;
}

void W3DVideoBuffer::Unlock()
{
    if (m_surface != nullptr) {
        m_surface->Unlock();
        m_surface->Release_Ref();
        m_surface = nullptr;
    }
}

bool W3DVideoBuffer::Valid()
{
    return m_texture != nullptr;
}

W3DVideoBuffer::Type W3DVideoBuffer::W3D_Format_To_Type(WW3DFormat format)
{
    Type type = TYPE_UNKNOWN;

    switch (format) {
        case WW3D_FORMAT_R8G8B8:
            type = TYPE_R8G8B8;
            break;
        case WW3D_FORMAT_X8R8G8B8:
            type = TYPE_X8R8G8B8;
            break;
        case WW3D_FORMAT_R5G6B5:
            type = TYPE_R5G6B5;
            break;
        case WW3D_FORMAT_X1R5G5B5:
            type = TYPE_X1R5G5B5;
            break;
        default:
            return type;
    }

    return type;
}

WW3DFormat W3DVideoBuffer::Type_To_W3D_Format(Type type)
{
    WW3DFormat format = WW3D_FORMAT_UNKNOWN;

    switch (type) {
        case TYPE_R8G8B8:
            format = WW3D_FORMAT_R8G8B8;
            break;
        case TYPE_X8R8G8B8:
            format = WW3D_FORMAT_X8R8G8B8;
            break;
        case TYPE_R5G6B5:
            format = WW3D_FORMAT_R5G6B5;
            break;
        case TYPE_X1R5G5B5:
            format = WW3D_FORMAT_X1R5G5B5;
            break;
        default:
            return format;
    }

    return format;
}
