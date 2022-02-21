/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Buffer Class
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "buff.h"
Buffer::Buffer(void *buffer, long size) : m_bufferPtr(buffer), m_size(size), m_isAllocated(false) {}

Buffer::Buffer(char *buffer, long size) : m_bufferPtr(buffer), m_size(size), m_isAllocated(false) {}

Buffer::Buffer(long size) : m_bufferPtr(NULL), m_size(size), m_isAllocated(false)
{
    if (size > 0) {
        m_bufferPtr = new char[size];
        m_isAllocated = true;
    }
}

Buffer::Buffer(Buffer const &buffer) : m_isAllocated(false)
{
    m_bufferPtr = buffer.m_bufferPtr;
    m_size = buffer.m_size;
}

Buffer &Buffer::operator=(Buffer const &buffer)
{
    if (buffer != this) {
        if (m_isAllocated) {
            delete[](char *) m_bufferPtr;
        }

        m_isAllocated = false;
        m_bufferPtr = buffer.m_bufferPtr;
        m_size = buffer.m_size;
    }

    return *this;
}

Buffer::~Buffer()
{
    Reset();
}

void Buffer::Reset()
{
    if (m_isAllocated) {
        delete[](char *) m_bufferPtr;
    }

    m_bufferPtr = NULL;
    m_size = 0;
    m_isAllocated = false;
}
