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
#include "streamingarchivefile.h"
#include "filesystem.h"
#include <algorithm>

StreamingArchiveFile::StreamingArchiveFile() : m_archiveFile(nullptr), m_fileStart(0), m_fileSize(0), m_filePos(0) {}

StreamingArchiveFile::~StreamingArchiveFile()
{
    File::Close();
}

bool StreamingArchiveFile::Open(const char *filename, int mode)
{
    File *basefile = g_theFileSystem->Open_File(filename, mode);

    return basefile == nullptr && Open(basefile);
}

void StreamingArchiveFile::Close()
{
    File::Close();
}

int StreamingArchiveFile::Read(void *dst, int bytes)
{
    if (m_archiveFile == nullptr) {
        return 0;
    }

    m_archiveFile->Seek(m_filePos + m_fileStart, START);

    if (m_filePos + bytes > m_fileSize) {
        bytes = m_fileSize - m_filePos;
    }

    int read_len = m_archiveFile->Read(dst, bytes);

    m_filePos += read_len;

    return read_len;
}

int StreamingArchiveFile::Write(void const *src, int bytes)
{
    captainslog_dbgassert(false, "Cannot write to streaming files.");
    return -1;
}

int StreamingArchiveFile::Seek(int offset, SeekMode mode)
{
    int pos;

    switch (mode) {
        case START:
            pos = offset;
            break;

        case CURRENT:
            pos = offset + m_filePos;
            break;

        case END:
            captainslog_dbgassert(
                offset <= 0, "StreamingArchiveFile::Seek - position should be <= 0 for a seek starting from the end.");
            pos = offset + m_fileSize;
            break;

        default:
            return -1;
    }

    if (pos >= 0) {
        if (pos > m_fileSize) {
            pos = m_fileSize;
        }
    } else {
        pos = 0;
    }

    m_filePos = pos;
    return m_filePos;
}

void StreamingArchiveFile::Next_Line(char *dst, int bytes)
{
    captainslog_dbgassert(false, "Should not call Next_Line on a streaming file.");
}

bool StreamingArchiveFile::Scan_Int(int &integer)
{
    captainslog_dbgassert(false, "Should not call Scan_Int on a streaming file.");
    return false;
}

bool StreamingArchiveFile::Scan_Real(float &real)
{
    captainslog_dbgassert(false, "Should not call Scan_Real on a streaming file.");
    return false;
}

bool StreamingArchiveFile::Scan_String(Utf8String &string)
{
    captainslog_dbgassert(false, "Should not call Scan_String on a streaming file.");
    return false;
}

void *StreamingArchiveFile::Read_Entire_And_Close()
{
    captainslog_dbgassert(false, "Are you sure you meant to Read_Entire_And_Close on a streaming file?");
    return nullptr;
}

RAMFile *StreamingArchiveFile::Convert_To_RAM_File()
{
    captainslog_dbgassert(false, "Are you sure you meant to Convert_To_RAM_File on a streaming file?");
    return this;
}

bool StreamingArchiveFile::Open(File *file)
{
    return true;
}

bool StreamingArchiveFile::Open_From_Archive(File *file, Utf8String const &name, int pos, int size)
{
    if (file == nullptr) {
        return false;
    }

    if (!File::Open(name.Str(), READ | BINARY | STREAMING)) {
        return false;
    }

    m_archiveFile = file;
    m_fileStart = pos;
    m_fileSize = size;
    m_filePos = 0;

    // Check that the file start position and size are actually valid
    if (m_archiveFile->Seek(m_fileStart, START) != pos) {
        return false;
    }

    if (m_archiveFile->Seek(m_fileSize, CURRENT) != size + m_fileStart) {
        return false;
    }

    m_archiveFile->Seek(m_fileStart, START);
    m_name = name;
    return true;
}

bool StreamingArchiveFile::Copy_Data_To_File(File *file)
{
    captainslog_dbgassert(false, "Are you sure you meant to Copy_Data_To_File on a streaming file?");
    return false;
}
