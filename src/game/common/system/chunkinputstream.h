/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stream interface providing a seekable data stream.
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
#include "inputstream.h"

class ChunkInputStream : public InputStream
{
public:
    virtual unsigned int Tell() = 0;
    virtual bool Absolute_Seek(unsigned int pos) = 0;
    virtual bool Eof() = 0;
};