/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Implements streaming archive file IO.
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
#include "gamememory.h"
#include "ramfile.h"

class StreamingArchiveFile : public RAMFile
{
    IMPLEMENT_POOL(StreamingArchiveFile);

protected:
    virtual ~StreamingArchiveFile() override;

public:
    StreamingArchiveFile();

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
    virtual RAMFile *Convert_To_RAM_File() override;
    virtual bool Open(File *file) override;
    virtual bool Open_From_Archive(File *file, Utf8String const &name, int pos, int size) override;
    virtual bool Copy_Data_To_File(File *file) override;

protected:
    File *m_archiveFile;
    int m_fileStart;
    int m_fileSize;
    int m_filePos;
};
