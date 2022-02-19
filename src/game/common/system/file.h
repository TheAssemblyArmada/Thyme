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

#include "asciistring.h"
#include "gamememory.h"
#include "mempoolobj.h"

struct FileInfo
{
    int32_t file_size_high;
    int32_t file_size_low;
    int32_t write_time_high;
    int32_t write_time_low;
};

namespace Thyme
{
// Thyme specific

// Returns integer of given mode and BUFFERED flag and encoded buffer size.
// Buffer size min: 0, max: 4194240, in 64 byte increments.
int32_t Encode_Buffered_File_Mode(int32_t mode, int32_t buffer_size);

// Decodes buffer size from given mode.
bool Decode_Buffered_File_Mode(int32_t mode, int32_t &buffer_size);
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
    virtual bool Open(const char *filename, int32_t mode);
    virtual void Close();
    virtual int32_t Read(void *dst, int32_t bytes) = 0;
    virtual int32_t Write(void const *src, int32_t bytes) = 0;
    virtual int32_t Seek(int32_t offset, File::SeekMode mode) = 0;
    virtual void Next_Line(char *dst, int32_t bytes) = 0;
    virtual bool Scan_Int(int32_t &integer) = 0;
    virtual bool Scan_Real(float &real) = 0;
    virtual bool Scan_String(Utf8String &string) = 0;
    virtual bool Print(const char *format, ...);
    virtual int32_t Size();
    virtual int32_t Position();

    virtual void *Read_All_And_Close() = 0;
    virtual File *Convert_To_RAM() = 0;

    Utf8String &Get_File_Name() { return m_filename; }
    int32_t Get_File_Mode() { return m_openMode; }
    void Set_Del_On_Close(bool del) { m_deleteOnClose = del; }

protected:
    File() : m_filename("<no file>"), m_openMode(0), m_access(false), m_deleteOnClose(false) {}

protected:
    Utf8String m_filename;
    int32_t m_openMode;
    bool m_access;
    bool m_deleteOnClose;
};