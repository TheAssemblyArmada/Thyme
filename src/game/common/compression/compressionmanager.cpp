/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Compression manager.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "compressionmanager.h"
#include "endiantype.h"
#include "refpack.h"
#include <captainslog.h>
#include <cstring>

using std::memcmp;

const char *CompressionManager::s_compressionNames[COMPRESSION_COUNT] = { "No compression",
    "RefPack",
    "LZH light (Nox, J2K)",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "zlib compress",
    "B-Tree compression",
    "Huffman Tree compression" };

/**
 * @brief Detect if the data is compressed.
 */
bool CompressionManager::Is_Data_Compressed(const void *data, int32_t size)
{
    return Get_Compression_Type(data, size) != COMPRESSION_NONE;
}

/**
 * @brief Get type of compression used based on a small header.
 */
CompressionType CompressionManager::Get_Compression_Type(const void *data, int32_t size)
{
    if (size <= 7) {
        return COMPRESSION_NONE;
    }

    CompressionType type = COMPRESSION_NONE;

    if (!memcmp(data, "NOX", 4)) {
        type = COMPRESSION_NOX;
    }

    if (!memcmp(data, "ZL1", 4)) {
        type = COMPRESSION_ZL1;
    }

    if (!memcmp(data, "ZL2", 4)) {
        type = COMPRESSION_ZL2;
    }

    if (!memcmp(data, "ZL3", 4)) {
        type = COMPRESSION_ZL3;
    }

    if (!memcmp(data, "ZL4", 4)) {
        type = COMPRESSION_ZL4;
    }

    if (!memcmp(data, "ZL5", 4)) {
        type = COMPRESSION_ZL5;
    }

    if (!memcmp(data, "ZL6", 4)) {
        type = COMPRESSION_ZL6;
    }

    if (!memcmp(data, "ZL7", 4)) {
        type = COMPRESSION_ZL7;
    }

    if (!memcmp(data, "ZL8", 4)) {
        type = COMPRESSION_ZL8;
    }

    if (!memcmp(data, "ZL9", 4)) {
        type = COMPRESSION_ZL9;
    }

    if (!memcmp(data, "EAB", 4)) {
        type = COMPRESSION_EAB;
    }

    if (!memcmp(data, "EAH", 4)) {
        type = COMPRESSION_EAH;
    }

    if (!memcmp(data, "EAR", 4)) {
        type = COMPRESSION_EAR;
    }

    return type;
}

/**
 * @brief Get uncompressed size based on a small header.
 */
int32_t CompressionManager::Get_Uncompressed_Size(const void *data, int32_t size)
{
    if (size < 8) {
        return size;
    }

    CompressionType type = Get_Compression_Type(data, size);

    if (type == COMPRESSION_NONE || type >= COMPRESSION_COUNT) {
        return size;
    }

    return le32toh(static_cast<const int32_t *>(data)[1]);
}

/**
 * @brief Decompress possibly compressed data. Only handles RefPack compression.
 */
int32_t CompressionManager::Decompress_Data(void *src, int32_t src_size, void *dst, int32_t dst_size)
{
    if (src_size <= 7) {
        return 0;
    }

    switch (Get_Compression_Type(src, src_size)) {
        case COMPRESSION_EAR: // RefPack
            src_size -= 8;
            return RefPack_Uncompress(dst, static_cast<const uint8_t *>(src) + 8, &src_size);

        // Original game handles all these formats, ZH only appears to use RefPack however.
        case COMPRESSION_NONE:
        case COMPRESSION_NOX:
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        default:
            captainslog_error("Compression format '%s' unhandled, file a bug report.\n",
                Get_Compression_Name(Get_Compression_Type(src, src_size)));
            break;
    }

    return 0;
}
