/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implementation of LocalFile file io interface. Despite the class name, the API used is Posix even on windows.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "localfile.h"

class Win32LocalFile : public LocalFile
{
    IMPLEMENT_POOL(Win32LocalFile);
    friend class Win32LocalFileSystem;

public:
    enum
    {
        INVALID_HANDLE = -1
    };

protected:
    virtual ~Win32LocalFile() override;

private:
    // Only the factory class, Win32LocalFileSystem can create file instances.
    Win32LocalFile();

public:
    virtual bool Open(const char *filename, int mode) override;
    virtual int Read(void *dst, int bytes) override;
    virtual int Write(void const *src, int bytes) override;
    virtual int Seek(int offset, File::SeekMode mode) override;
    virtual void Next_Line(char *dst, int bytes) override;
    virtual bool Scan_Int(int &integer) override;
    virtual bool Scan_Real(float &real) override;
    virtual bool Scan_String(Utf8String &string) override;

private:
    int FileHandle;
};