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
#include "rtsutils.h"
#if BUILD_WITH_ZLIB
#include "zlibcompr.h"
#endif
#include <captainslog.h>
#include <cstring>

using rts::FourCC;
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
 * @brief Thyme specific: Get the FourCC for the corresponding compression type
 */
uint32_t CompressionManager::Get_Compression_FourCC(CompressionType type)
{
    switch (type) {
        case COMPRESSION_EAR:
            return FourCC<'E', 'A', 'R', '\0'>::value;
        case COMPRESSION_ZL1:
            return FourCC<'Z', 'L', '1', '\0'>::value;
        case COMPRESSION_ZL2:
            return FourCC<'Z', 'L', '2', '\0'>::value;
        case COMPRESSION_ZL3:
            return FourCC<'Z', 'L', '3', '\0'>::value;
        case COMPRESSION_ZL4:
            return FourCC<'Z', 'L', '4', '\0'>::value;
        case COMPRESSION_ZL5:
            return FourCC<'Z', 'L', '5', '\0'>::value;
        case COMPRESSION_ZL6:
            return FourCC<'Z', 'L', '6', '\0'>::value;
        case COMPRESSION_ZL7:
            return FourCC<'Z', 'L', '7', '\0'>::value;
        case COMPRESSION_ZL8:
            return FourCC<'Z', 'L', '8', '\0'>::value;
        case COMPRESSION_ZL9:
            return FourCC<'Z', 'L', '9', '\0'>::value;
        case COMPRESSION_NONE:
        default:
            captainslog_error("Compression format '%s' unhandled", Get_Compression_Name(type));
            return FourCC<0, 0, 0, 0>::value;
    }
}

/**
 * @brief Thyme specific: Get the FourCC for the corresponding compression type
 */
CompressionType CompressionManager::Get_Compression_Type_By_FourCC(uint32_t fourcc)
{
    switch (fourcc) {
        case FourCC<'E', 'A', 'R', '\0'>::value:
            return COMPRESSION_EAR;
        case FourCC<'Z', 'L', '1', '\0'>::value:
            return COMPRESSION_ZL1;
        case FourCC<'Z', 'L', '2', '\0'>::value:
            return COMPRESSION_ZL2;
        case FourCC<'Z', 'L', '3', '\0'>::value:
            return COMPRESSION_ZL3;
        case FourCC<'Z', 'L', '4', '\0'>::value:
            return COMPRESSION_ZL4;
        case FourCC<'Z', 'L', '5', '\0'>::value:
            return COMPRESSION_ZL5;
        case FourCC<'Z', 'L', '6', '\0'>::value:
            return COMPRESSION_ZL6;
        case FourCC<'Z', 'L', '7', '\0'>::value:
            return COMPRESSION_ZL7;
        case FourCC<'Z', 'L', '8', '\0'>::value:
            return COMPRESSION_ZL8;
        case FourCC<'Z', 'L', '9', '\0'>::value:
            return COMPRESSION_ZL9;
        default:
            captainslog_error("Compression fourcc '%u' unhandled", fourcc);
            return COMPRESSION_NONE;
    }
}

#ifdef GAME_DLL
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
#else
/**
 * @brief Get type of compression used based on a small header.
 */
CompressionType CompressionManager::Get_Compression_Type(const void *data, int size)
{
    if (size < sizeof(ComprHeader)) {
        return COMPRESSION_NONE;
    }

    const uint32_t fourCC = rts::FourCC_From_String(static_cast<const char *>(data));
    return Get_Compression_Type_By_FourCC(fourCC);
}
#endif

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

    uint32_t fourcc = Get_Compression_FourCC(type);

    // Initialize the header. We could think about not writing the fourcc when format is not supported,
    // but probably doesn't really matter
    ComprHeader *header = static_cast<ComprHeader *>(dst);
    memcpy(header->fourcc, &fourcc, sizeof(uint32_t));
    header->uncomp_size = 0;

    uint32_t compr_size = 0;
    uint8_t *dst_data = static_cast<uint8_t *>(dst) + sizeof(ComprHeader);

    switch (type) {
        case COMPRESSION_EAR:
            compr_size = RefPack_Compress(dst_data, src, src_size, nullptr);
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
            break;
#endif
        case COMPRESSION_NONE:
        case COMPRESSION_NOX:
        case COMPRESSION_EAB:
        case COMPRESSION_EAH:
        default:
            captainslog_error(
                "Compression format '%s' unhandled", Get_Compression_Name(Get_Compression_Type(src, src_size)));
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
            captainslog_error(
                "Compression format '%s' unhandled", Get_Compression_Name(Get_Compression_Type(src, src_size)));
            break;
    }

    return 0;
}
