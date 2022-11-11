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
#if BUILD_WITH_ZLIB
#include "zlibcompr.h"
#endif
#include <captainslog.h>
#include <cstring>

using std::memcmp;

const char *CompressionManager::s_compressionNames[COMPRESSION_COUNT] = { "No compression",
    "RefPack",
    "LZH light (Nox, J2K)",
    "zlib compress 1",
    "zlib compress 2",
    "zlib compress 3",
    "zlib compress 4",
    "zlib compress 5",
    "zlib compress 6",
    "zlib compress 7",
    "zlib compress 8",
    "zlib compress 9",
    "B-Tree compression",
    "Huffman Tree compression" };

/**
 * @brief Detect if the data is compressed.
 */
bool CompressionManager::Is_Data_Compressed(const void *data, int size)
{
    return Get_Compression_Type(data, size) != COMPRESSION_NONE;
}

/**
 * @brief Get prefered compression type.
 */
CompressionType CompressionManager::Get_Prefered_Compression()
{
    return COMPRESSION_EAR;
}

/**
 * @brief Get the FourCC for the corresponding compression type
 */
const char *CompressionManager::Get_Compression_FourCC(CompressionType type)
{
    switch (type) {
        case COMPRESSION_EAR:
            return "EAR\0";
        case COMPRESSION_ZL1:
            return "ZL1\0";
        case COMPRESSION_ZL2:
            return "ZL2\0";
        case COMPRESSION_ZL3:
            return "ZL3\0";
        case COMPRESSION_ZL4:
            return "ZL4\0";
        case COMPRESSION_ZL5:
            return "ZL5\0";
        case COMPRESSION_ZL6:
            return "ZL6\0";
        case COMPRESSION_ZL7:
            return "ZL7\0";
        case COMPRESSION_ZL8:
            return "ZL8\0";
        case COMPRESSION_ZL9:
            return "ZL9\0";
        case COMPRESSION_NONE:
        default:
            captainslog_error("Compression format '%s' unhandled, file a bug report.\n", Get_Compression_Name(type));
            return "\0\0\0\0";
    }
}

/**
 * @brief Get type of compression used based on a small header.
 */
CompressionType CompressionManager::Get_Compression_Type(const void *data, int size)
{
    if (size < sizeof(ComprHeader)) {
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
 * @brief Get the maximum size this uncompressed data will use up after compression
 */
int CompressionManager::Get_Max_Compressed_Size(int size, CompressionType type)
{
    switch (type) {
        case COMPRESSION_EAR:
            return size + sizeof(ComprHeader);
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
#if BUILD_WITH_ZLIB
            return Zlib_MaxSize(size) + sizeof(ComprHeader);
#endif
        default:
            return 0;
    }
}

/**
 * @brief Get uncompressed size based on a small header.
 */
int CompressionManager::Get_Uncompressed_Size(const void *data, int size)
{
    if (size < sizeof(ComprHeader)) {
        return size;
    }

    CompressionType type = Get_Compression_Type(data, size);

    if (type == COMPRESSION_NONE || type >= COMPRESSION_COUNT) {
        return size;
    }

    const ComprHeader *header = static_cast<const ComprHeader *>(data);
    return le32toh(header->uncomp_size);
}

/**
 * @brief Compress uncompressed data. Only handles RefPack and ZLib compression.
 */
int CompressionManager::Compress_Data(CompressionType type, void *src, int src_size, void *dst, int dst_size)
{
    if (dst_size < sizeof(ComprHeader)) {
        return 0;
    }

    const char *fourcc = Get_Compression_FourCC(type);

    // Initialize the header. We could think about not writing the fourcc when format is not supported,
    // but probably doesn't really matter
    ComprHeader *header = static_cast<ComprHeader *>(dst);
    memcpy(header->fourcc, fourcc, 4);
    header->uncomp_size = 0;

    uint32_t compr_size = 0;
    uint8_t *dst_data = static_cast<uint8_t *>(dst) + sizeof(ComprHeader);

    switch (type) {
        case COMPRESSION_EAR:
            compr_size = RefPack_Compress(dst_data, src, src_size, false);
            if (compr_size > 0) {
                header->uncomp_size = src_size;
                return compr_size + sizeof(ComprHeader);
            }
            break;
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
#if BUILD_WITH_ZLIB
            compr_size = Zlib_Compress(dst_data, dst_size - sizeof(ComprHeader), src, src_size, type + 1 - COMPRESSION_ZL1);
            if (compr_size > 0) {
                header->uncomp_size = src_size;
                return compr_size + sizeof(ComprHeader);
            }
#endif
        case COMPRESSION_NONE:
        case COMPRESSION_NOX:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        default:
            captainslog_error("Compression format '%s' unhandled, file a bug report.\n",
                Get_Compression_Name(Get_Compression_Type(src, src_size)));
            break;
    }

    return 0;
}

/**
 * @brief Decompress possibly compressed data. Only handles RefPack and ZLib compression.
 */
int CompressionManager::Decompress_Data(void *src, int src_size, void *dst, int dst_size)
{
    if (src_size < sizeof(ComprHeader)) {
        return 0;
    }

    switch (Get_Compression_Type(src, src_size)) {
        case COMPRESSION_EAR: // RefPack
            src_size -= sizeof(ComprHeader);
            return RefPack_Uncompress(dst, static_cast<const uint8_t *>(src) + sizeof(ComprHeader), &src_size);
        case COMPRESSION_ZL1:
        case COMPRESSION_ZL2:
        case COMPRESSION_ZL3:
        case COMPRESSION_ZL4:
        case COMPRESSION_ZL5:
        case COMPRESSION_ZL6:
        case COMPRESSION_ZL7:
        case COMPRESSION_ZL8:
        case COMPRESSION_ZL9:
#if BUILD_WITH_ZLIB
            src_size -= sizeof(ComprHeader);
            return Zlib_Uncompress(dst, dst_size, static_cast<const uint8_t *>(src) + sizeof(ComprHeader), src_size);
#endif
        case COMPRESSION_NONE:
        case COMPRESSION_NOX:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        default:
            captainslog_error("Compression format '%s' unhandled, file a bug report.\n",
                Get_Compression_Name(Get_Compression_Type(src, src_size)));
            break;
    }

    return 0;
}
