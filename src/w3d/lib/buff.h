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
    Buffer(char *ptr, long size = 0);
    Buffer(void *ptr = 0, long size = 0);
    Buffer(void const *ptr, long size = 0);
    Buffer(long size);
    Buffer(Buffer const &buffer);
    ~Buffer(void);

    Buffer &operator=(Buffer const &buffer);
    operator void *(void) const { return m_bufferPtr; }
    operator char *(void) const { return (char *)m_bufferPtr; }

    void Reset(void);
    void *Get_Buffer(void) const { return m_bufferPtr; }
    long Get_Size(void) const { return m_size; }
    bool Is_Valid(void) const { return m_bufferPtr != 0; }

protected:
    void *m_bufferPtr;
    long m_size;
    bool m_isAllocated;
};
