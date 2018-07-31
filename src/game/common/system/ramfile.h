/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements RAM file IO.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "file.h"

class RAMFile : public File
{
    IMPLEMENT_POOL(RAMFile);

public:
    RAMFile();
    virtual ~RAMFile();

    virtual bool Open(const char *filename, int mode) override;
    virtual void Close() override;
    virtual int Read(void *dst, int bytes) override;
    virtual int Write(void const *src, int bytes) override;
    virtual int Seek(int offset, File::SeekMode mode) override;
    virtual void Next_Line(char *dst, int bytes) override;
    virtual bool Scan_Int(int &integer) override;
    virtual bool Scan_Real(float &real) override;
    virtual bool Scan_String(AsciiString &string) override;

    virtual void *Read_All_And_Close() override;
    virtual RAMFile *Convert_To_RAM() override { return this; }
    virtual bool Open(File *file);
    virtual bool Open_From_Archive(File *file, AsciiString const &name, int pos, int size);
    virtual bool Copy_To_File(File *file);

protected:
    char *Data;
    int Pos;
    int Size;
};
