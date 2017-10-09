/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Stream object providing a memory backed seekable data stream.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "cachedfileinputstream.h"
#include "filesystem.h"
#include "minmax.h"

CachedFileInputStream::~CachedFileInputStream()
{
    if (m_cachedData != nullptr) {
        delete[] m_cachedData;
    }
}

/**
* @brief Read data from the cached file.
*/
int CachedFileInputStream::Read(void *dst, int size)
{
    if (m_cachedData != nullptr) {
        if (m_cachePos + size > m_cachedSize) {
            size = m_cachedSize - m_cachePos;
        }

        if (size > 0) {
            memcpy(dst, &m_cachedData[m_cachePos], size);
            m_cachePos += size;
        }

        return size;
    }

    return 0;
}

/**
* @brief Get current position in the stream.
*/
unsigned CachedFileInputStream::Tell()
{
    return m_cachePos;
}

/**
* @brief Seek to an absolute position in the stream.
*/
bool CachedFileInputStream::Absolute_Seek(unsigned pos)
{
    if (pos < m_cachedSize) {
        m_cachePos = pos;
    } else {
        m_cachePos = m_cachedSize;
    }

    return true;
}

bool CachedFileInputStream::Eof()
{
    return m_cachedSize == m_cachePos;
}

bool CachedFileInputStream::Open(AsciiString filename)
{
#if 0
    File *file = g_theFileSystem->Open(filename, File::BINARY | File::READ);
    m_cachedSize = 0;

    if (file != nullptr) {
        m_cachedSize = file->Size();

        if (m_cachedSize > 0) {
            m_cachedData = static_cast<uint8_t *>(file->Read_All_And_Close());
            file = nullptr;
        }
    }

    // TODO Needs CompressionManager class to handle compression here

    return m_cachedSize > 0;
#elif !defined THYME_STANDALONE
    return Call_Method<bool, CachedFileInputStream, AsciiString>(0x00571E00, this, filename);
#else
    return false;
#endif
}

void CachedFileInputStream::Close()
{
    if (m_cachedData) {
        delete[] m_cachedData;
        m_cachedData = nullptr;
    }

    m_cachePos = 0;
    m_cachedSize = 0;
}
