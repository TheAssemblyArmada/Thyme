/**
 * @file
 *
 * @author OmniBlade
 * @author xezon
 *
 * @brief Same as Win32LocalFile, but with buffered read and write. (Thyme Feature)
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
#include "localfile.h"
#include <cstdio>

class Win32LocalFileSystem;
class StdLocalFileSystem;

namespace Thyme
{

class StandardFile : public LocalFile
{
    IMPLEMENT_POOL(StandardFile);

    friend class ::Win32LocalFileSystem;
    friend class StdLocalFileSystem;

protected:
    virtual ~StandardFile() override;

private:
    StandardFile();

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
    FILE *m_file;
};

} // namespace Thyme
