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
    ~StreamingArchiveFile() override;

public:
    StreamingArchiveFile();

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
    RAMFile *Convert_To_RAM_File() override;
    bool Open(File *file) override;
    bool Open_From_Archive(File *file, Utf8String const &name, int pos, int size) override;
    bool Copy_Data_To_File(File *file) override;

protected:
    File *m_archiveFile;
    int m_fileStart;
    int m_fileSize;
    int m_filePos;
};
