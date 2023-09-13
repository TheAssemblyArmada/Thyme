/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief FileClass for reading files with buffered calls.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "bufffile.h"
#include <algorithm>
#include <cstring>

using std::memcpy;

unsigned BufferedFileClass::m_desiredBufferSize = 0x4000;

BufferedFileClass::BufferedFileClass() : m_buffer(nullptr), m_bufferSize(0), m_bufferAvailable(0), m_bufferOffset(0) {}

BufferedFileClass::BufferedFileClass(const char *filename) :
    RawFileClass(filename), m_buffer(nullptr), m_bufferSize(0), m_bufferAvailable(0), m_bufferOffset(0)
{
}

BufferedFileClass::~BufferedFileClass()
{
    Reset_Buffer();
}

int BufferedFileClass::Write(void const *buffer, int size)
{
    // DEBUG_ASSERT(m_bufferSize > 0);

    return RawFileClass::Write(buffer, size);
}

int BufferedFileClass::Read(void *buffer, int size)
{
    int read = 0;

    if (m_bufferAvailable > 0) {
        int nsize = std::min(m_bufferAvailable, size);

        memmove(buffer, m_buffer + m_bufferOffset, nsize);
        m_bufferAvailable -= nsize;
        m_bufferOffset += nsize;
        size -= nsize;
        buffer = static_cast<uint8_t *>(buffer) + nsize;
        read = nsize;
    }

    if (size > 0) {
        int bsize = m_bufferSize;

        if (bsize == 0) {
            bsize = m_desiredBufferSize;
        }

        if (size > bsize) {
            return read + RawFileClass::Read(buffer, size);
        }

        if (m_bufferSize == 0) {
            m_bufferSize = m_desiredBufferSize;
            m_buffer = new uint8_t[m_bufferSize];
            m_bufferAvailable = 0;
            m_bufferOffset = 0;
        }

        if (m_bufferAvailable == 0) {
            m_bufferAvailable = RawFileClass::Read(m_buffer, m_bufferSize);
            m_bufferOffset = 0;
        }

        if (m_bufferAvailable > 0) {
            int nsize = std::min(m_bufferAvailable, size);
            memcpy(buffer, m_buffer + m_bufferOffset, nsize);
            m_bufferAvailable -= nsize;
            m_bufferOffset += nsize;
            read += nsize;
        }
    }

    return read;
}

off_t BufferedFileClass::Seek(off_t offset, int whence)
{
    if (whence != FS_SEEK_CURRENT || offset < 0) {
        Reset_Buffer();
    }

    if (m_bufferAvailable != 0) {
        int left = m_bufferAvailable;

        if (left > offset) {
            left = offset;
        }

        m_bufferAvailable -= left;
        m_bufferOffset += left;

        return RawFileClass::Seek(offset - left, whence) - m_bufferAvailable;
    }

    return RawFileClass::Seek(offset, whence);
}

void BufferedFileClass::Close()
{
    RawFileClass::Close();
    Reset_Buffer();
}

void BufferedFileClass::Reset_Buffer()
{
    if (m_buffer != nullptr) {
        delete[] m_buffer;
        m_buffer = nullptr;
        m_bufferSize = 0;
        m_bufferAvailable = 0;
        m_bufferOffset = 0;
    }
}
