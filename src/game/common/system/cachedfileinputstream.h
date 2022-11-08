/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stream object providing a memory backed seekable data stream.
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
#include "asciistring.h"
#include "chunkinputstream.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class CachedFileInputStream : public ChunkInputStream
{
public:
    CachedFileInputStream() : m_cachedSize(0), m_cachedData(nullptr), m_cachePos(0) {}
    ~CachedFileInputStream();

    virtual int Read(void *dst, int size) override;
    virtual unsigned Tell() override;
    virtual bool Absolute_Seek(unsigned int pos) override;
    virtual bool Eof() override;

    bool Open(Utf8String filename);
    void Close();

private:
    unsigned int m_cachedSize;
    uint8_t *m_cachedData;
    unsigned int m_cachePos;
};
