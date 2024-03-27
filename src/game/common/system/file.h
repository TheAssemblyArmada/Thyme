/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Base class for file IO.
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
#include "gamememory.h"
#include "mempoolobj.h"

struct FileInfo
{
    int file_size_high;
    int file_size_low;
    int write_time_high;
    int write_time_low;
};

namespace Thyme
{
// Thyme specific

// Returns integer of given mode and BUFFERED flag and encoded buffer size.
// Buffer size min: 0, max: 4194240, in 64 byte increments.
int Encode_Buffered_File_Mode(int mode, int buffer_size);

// Decodes buffer size from given mode.
bool Decode_Buffered_File_Mode(int mode, int &buffer_size);
} // namespace Thyme

class File : public MemoryPoolObject
{
    IMPLEMENT_ABSTRACT_POOL(File);

public:
    enum SeekMode
    {
        START = 0,
        CURRENT = 1,
        END = 2,
    };

    enum FileMode
    {
        READ = 0x01,
        WRITE = 0x02,
        APPEND = 0x04,
        CREATE = 0x08,
        TRUNCATE = 0x10,
        TEXT = 0x20,
        BINARY = 0x40,
        STREAMING = 0x100,
        BUFFERED = 0x8000, // Thyme specific. When set, opens file with buffered read & write as opposed to instant file IO.
                           // Optionally, the high 16 bits of FileMode can be used to specify the buffer size, otherwise uses
                           // default buffer size.

        LASTMODE, // Just marks the enum end.
    };

protected:
    virtual ~File() override;

public:
    virtual bool Open(const char *filename, int mode);
    virtual void Close();
    virtual int Read(void *dst, int bytes) = 0;
    virtual int Write(void const *src, int bytes) = 0;
    virtual int Seek(int offset, SeekMode mode) = 0;
    virtual void Next_Line(char *dst, int bytes) = 0;
    virtual bool Scan_Int(int &integer) = 0;
    virtual bool Scan_Real(float &real) = 0;
    virtual bool Scan_String(Utf8String &string) = 0;
    virtual bool Print(const char *format, ...);
    virtual int Size();
    virtual int Position();

    virtual void *Read_Entire_And_Close() = 0;
    virtual File *Convert_To_RAM_File() = 0;

    bool Eof();

    const char *Get_Name() const { return m_name.Str(); }
    void Set_Name(const char *name) { m_name = name; }
    int Get_Access() const { return m_access; }
    void Delete_On_Close() { m_deleteOnClose = true; }

protected:
    File();

protected:
    Utf8String m_name;
    int m_access;
    bool m_open;
    bool m_deleteOnClose;
};
