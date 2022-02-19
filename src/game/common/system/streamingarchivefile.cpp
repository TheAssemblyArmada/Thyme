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

bool StreamingArchiveFile::Open(const char *filename, int32_t mode)
{
    File *basefile = g_theFileSystem->Open(filename, mode);

    if (basefile == nullptr) {
        return false;
    }

    return Open(basefile);
}

void StreamingArchiveFile::Close()
{
    File::Close();
}

int32_t StreamingArchiveFile::Read(void *dst, int32_t bytes)
{
    if (m_archiveFile == nullptr) {
        return 0;
    }

    m_archiveFile->Seek(m_filePos + m_fileStart, START);

    if (m_filePos + bytes > m_fileSize) {
        bytes = m_fileSize - m_filePos;
    }

    int32_t read_len = m_archiveFile->Read(dst, bytes);

    m_filePos += read_len;

    return bytes;
}

int32_t StreamingArchiveFile::Write(void const *src, int32_t bytes)
{
    return -1;
}

int32_t StreamingArchiveFile::Seek(int32_t offset, File::SeekMode mode)
{
    switch (mode) {
        case START:
            m_filePos = std::clamp(offset, 0, m_fileSize);
            break;

        case CURRENT:
            m_filePos = std::clamp(m_filePos + offset, 0, m_fileSize);
            break;

        case END:
            m_filePos = std::clamp(m_fileSize + offset, 0, m_fileSize);
            break;

        default:
            return -1;
    }

    return m_filePos;
}

void StreamingArchiveFile::Next_Line(char *dst, int32_t bytes) {}

bool StreamingArchiveFile::Scan_Int(int32_t &integer)
{
    return false;
}

bool StreamingArchiveFile::Scan_Real(float &real)
{
    return false;
}

bool StreamingArchiveFile::Scan_String(Utf8String &string)
{
    return false;
}

void *StreamingArchiveFile::Read_All_And_Close()
{
    return nullptr;
}

bool StreamingArchiveFile::Open(File *file)
{
    return true;
}

bool StreamingArchiveFile::Open_From_Archive(File *file, Utf8String const &name, int32_t pos, int32_t size)
{
    if (file == nullptr || !File::Open(name.Str(), READ | BINARY | STREAMING)) {
        return false;
    }

    m_archiveFile = file;
    m_fileStart = pos;
    m_fileSize = size;
    m_filePos = 0;

    // Check that the file start position and size are actually valid
    if (m_archiveFile->Seek(m_fileStart, START) == m_fileStart) {
        if (m_archiveFile->Seek(m_fileSize, CURRENT) == m_fileStart + m_fileSize) {
            m_archiveFile->Seek(m_fileStart, START);

            return true;
        }
    }

    return false;
}
