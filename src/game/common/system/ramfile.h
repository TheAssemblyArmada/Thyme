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

protected:
    ~RAMFile() override;

public:
    RAMFile();

    bool Open(const char *filename, int mode) override;
    void Close() override;
    int Read(void *dst, int bytes) override;
    int Write(void const *src, int bytes) override;
    int Seek(int offset, SeekMode mode) override;
    void Next_Line(char *dst, int bytes) override;
    bool Scan_Int(int &integer) override;
    bool Scan_Real(float &real) override;
    bool Scan_String(Utf8String &string) override;

    void *Read_Entire_And_Close() override;
    RAMFile *Convert_To_RAM_File() override { return this; }
    virtual bool Open(File *file);
    virtual bool Open_From_Archive(File *file, Utf8String const &name, int pos, int size);
    virtual bool Copy_Data_To_File(File *file);

protected:
    char *m_data;
    int m_pos;
    int m_size;
};
