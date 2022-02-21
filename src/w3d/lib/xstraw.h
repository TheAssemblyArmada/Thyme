/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Buffer Straw Class
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
#include "buff.h"
#include "straw.h"

class BufferStraw : public Straw
{
public:
    BufferStraw(Buffer const &buffer) : m_bufferPtr(buffer), m_index(0) {}
    BufferStraw(void *buffer, int length) : m_bufferPtr(buffer, length), m_index(0) {}
    virtual int Get(void *source, int slen);

private:
    Buffer m_bufferPtr;
    int m_index;

    bool Is_Valid() { return (m_bufferPtr.Is_Valid()); }

    BufferStraw(BufferStraw &) = delete;
    BufferStraw &operator=(BufferStraw const &) = delete;
};
