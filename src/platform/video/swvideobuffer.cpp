/**
 * @file
 *
 * @author feliwir
 *
 * @brief A software implementation for video buffers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "swvideobuffer.h"
#include <cstring>

namespace Thyme
{

SWVideoBuffer::SWVideoBuffer(Type type) : VideoBuffer(type), m_locked(false), m_data(nullptr) {}

SWVideoBuffer::~SWVideoBuffer()
{
    Free();
}

bool SWVideoBuffer::Allocate(unsigned width, unsigned height)
{
    if (m_format != VideoBuffer::TYPE_X8R8G8B8 && m_format != VideoBuffer::TYPE_R8G8B8) {
        return false;
    }

    Free();
    m_width = width;
    m_height = height;
    m_textureWidth = width;
    m_textureHeight = height;

    int bytes_per_pixel = (m_format == VideoBuffer::TYPE_X8R8G8B8) ? 4 : 3;
    m_pitch = bytes_per_pixel * m_width;
    m_data = new uint8_t[m_pitch * m_height]();
    return true;
}

void SWVideoBuffer::Free()
{
    delete[] m_data;
    m_data = nullptr;
    VideoBuffer::Free();
}

void *SWVideoBuffer::Lock()
{
    m_locked = true;
    return m_data;
}

void SWVideoBuffer::Unlock()
{
    m_locked = false;
}

bool SWVideoBuffer::Valid()
{
    return m_data != nullptr;
}

} // namespace Thyme
