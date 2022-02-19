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

#include "gamememory.h"
#include "ramfile.h"

class StreamingArchiveFile : public RAMFile
{
    IMPLEMENT_POOL(StreamingArchiveFile);

protected:
    virtual ~StreamingArchiveFile() override;

public:
    StreamingArchiveFile();

    virtual bool Open(const char *filename, int32_t mode) override;
    virtual void Close() override;
    virtual int32_t Read(void *dst, int32_t bytes) override;
    virtual int32_t Write(void const *src, int32_t bytes) override;
    virtual int32_t Seek(int32_t offset, File::SeekMode mode) override;
    virtual void Next_Line(char *dst, int32_t bytes) override;
    virtual bool Scan_Int(int32_t &integer) override;
    virtual bool Scan_Real(float &real) override;
    virtual bool Scan_String(Utf8String &string) override;

    virtual void *Read_All_And_Close() override;
    virtual RAMFile *Convert_To_RAM() override { return this; }
    virtual bool Open(File *file) override;
    virtual bool Open_From_Archive(File *file, Utf8String const &name, int32_t pos, int32_t size) override;
    virtual bool Copy_To_File(File *file) override { return false; }

protected:
    File *m_archiveFile;
    int32_t m_fileStart;
    int32_t m_fileSize;
    int32_t m_filePos;
};
