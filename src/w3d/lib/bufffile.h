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
#pragma once

#include "always.h"
#include "rawfile.h"

class BufferedFileClass : public RawFileClass
{
public:
    BufferedFileClass();
    BufferedFileClass(const char *filename);
    virtual ~BufferedFileClass();

    virtual int Write(void const *buffer, int size) override;
    virtual int Read(void *buffer, int size) override;
    virtual off_t Seek(off_t offset, int whence = FS_SEEK_CURRENT) override;
    virtual void Close() override;

    void Reset_Buffer();

    static void Set_Desired_Buffer_Size(unsigned size) { m_desiredBufferSize = size; }

protected:
    uint8_t *m_buffer;
    int m_bufferSize;
    int m_bufferAvailable;
    int m_bufferOffset;

private:
    static unsigned m_desiredBufferSize;
};
