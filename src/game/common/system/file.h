/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Base class for file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

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

class File : public MemoryPoolObject
{
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
    };

public:
    virtual ~File();

    virtual bool Open(const char *filename, int mode);
    virtual void Close();
    virtual int Read(void *dst, int bytes) = 0;
    virtual int Write(void const *src, int bytes) = 0;
    virtual int Seek(int offset, File::SeekMode mode) = 0;
    virtual void Next_Line(char *dst, int bytes) = 0;
    virtual bool Scan_Int(int &integer) = 0;
    virtual bool Scan_Real(float &real) = 0;
    virtual bool Scan_String(AsciiString &string) = 0;
    virtual bool Print(const char *format, ...);
    virtual int Size();
    virtual int Position();

    virtual void *Read_All_And_Close() = 0;
    virtual File *Convert_To_RAM() = 0;

    AsciiString &Get_File_Name() { return m_filename; }
    int Get_File_Mode() { return m_openMode; }
    void Set_Del_On_Close(bool del) { m_deleteOnClose = del; }

protected:
    File() : m_filename("<no file>"), m_openMode(0), m_access(false), m_deleteOnClose(false) {}

protected:
    AsciiString m_filename;
    int m_openMode;
    bool m_access;
    bool m_deleteOnClose;
};