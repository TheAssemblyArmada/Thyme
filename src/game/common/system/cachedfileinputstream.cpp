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
#include "compressionmanager.h"
#include "filesystem.h"
#include <algorithm>

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
    if (m_cachedData == nullptr) {
        return 0;
    }

    if (m_cachePos + size > m_cachedSize) {
        size = m_cachedSize - m_cachePos;
    }

    if (size != 0) {
        memcpy(dst, &m_cachedData[m_cachePos], size);
        m_cachePos += size;
    }

    return size;
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
bool CachedFileInputStream::Absolute_Seek(unsigned int pos)
{
    if (pos > m_cachedSize) {
        pos = m_cachedSize;
    }

    m_cachePos = pos;
    return true;
}

/**
 * @brief Check if we are at the end of the file.
 */
bool CachedFileInputStream::Eof()
{
    return m_cachedSize == m_cachePos;
}

/**
 * @brief Open a file as an input stream.
 */
bool CachedFileInputStream::Open(Utf8String filename)
{
    File *file = g_theFileSystem->Open_File(filename.Str(), File::BINARY | File::READ);
    m_cachedSize = 0;

    if (file != nullptr) {
        m_cachedSize = file->Size();

        if (m_cachedSize > 0) {
            m_cachedData = static_cast<uint8_t *>(file->Read_Entire_And_Close());
            file = nullptr;
        }

        m_cachePos = 0;
    }

    // Handle compressed data.
    if (CompressionManager::Is_Data_Compressed(m_cachedData, m_cachedSize)) {
        unsigned int decomp_size = CompressionManager::Get_Uncompressed_Size(m_cachedData, m_cachedSize);
        uint8_t *decomp_data = new uint8_t[decomp_size];

        if (CompressionManager::Decompress_Data(m_cachedData, m_cachedSize, decomp_data, decomp_size) == decomp_size) {
            delete[] m_cachedData;
            m_cachedData = decomp_data;
            m_cachedSize = decomp_size;
        } else {
            delete[] decomp_data;
        }
    }

    if (file != nullptr) {
        file->Close();
    }

    return m_cachedSize != 0;
}

/**
 * @brief Close an input stream.
 */
void CachedFileInputStream::Close()
{
    if (m_cachedData) {
        delete[] m_cachedData;
        m_cachedData = nullptr;
    }

    m_cachePos = 0;
    m_cachedSize = 0;
}
