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
#pragma once
#include "always.h"
class Buffer
{
public:
    Buffer(void *ptr = nullptr, long size = 0);
    Buffer(char *ptr, long size = 0);
    Buffer(long size);
    Buffer(Buffer const &buffer);
    ~Buffer();

    Buffer &operator=(Buffer const &buffer);
    operator void *() const { return m_bufferPtr; }
    operator char *() const { return static_cast<char *>(m_bufferPtr); }

    void Reset();
    void *Get_Buffer() const { return m_bufferPtr; }
    long Get_Size() const { return m_size; }
    bool Is_Valid() const { return m_bufferPtr != 0; }

protected:
    void *m_bufferPtr;
    long m_size;
    bool m_isAllocated;
};
