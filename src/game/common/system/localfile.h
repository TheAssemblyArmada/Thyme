/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Interface for local file IO.
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
#include "file.h"

class LocalFile : public File
{
    IMPLEMENT_ABSTRACT_POOL(LocalFile);

public:
    enum
    {
        INVALID_HANDLE = -1
    };

protected:
    virtual ~LocalFile() override;

protected:
    // Only the factory class, Win32LocalFileSystem can create file instances.
    LocalFile();

public:
    virtual bool Open(const char *filename, int mode) override;
    virtual void Close() override;
    virtual int Read(void *dst, int bytes) override;
    virtual int Write(void const *src, int bytes) override;
    virtual int Seek(int offset, SeekMode mode) override;
    virtual void Next_Line(char *dst, int bytes) override;
    virtual bool Scan_Int(int &integer) override;
    virtual bool Scan_Real(float &real) override;
    virtual bool Scan_String(Utf8String &string) override;
    virtual void *Read_Entire_And_Close() override;
    virtual File *Convert_To_RAM_File() override;

protected:
    int m_fileHandle;

    static int s_totalOpen;
};
