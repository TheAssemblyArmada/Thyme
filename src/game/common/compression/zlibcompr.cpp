/**
 * @file
 *
 * @author feliwir
 *
 * @brief Zlib compression.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "zlibcompr.h"
#include <captainslog.h>
#include <zlib.h>

/**
 * Decompresses zlib format.
 */
int Zlib_Uncompress(void *dst, int dst_size, const void *src, int src_size)
{
    if (src_size == 0) {
        captainslog_error("Missing src_size for Zlib data");
        return 0;
    }

    uLongf dstSize = dst_size;
    uLong srcSize = src_size;

    int result = uncompress((Bytef *)dst, &dstSize, (const Bytef *)src, srcSize);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    return dstSize;
}

/**
 * Compresses to zlib data.
 */
int Zlib_Compress(void *dst, const void *src, int size, int level)
{
    uLongf dstSize = compressBound(size);
    int result = compress2((Bytef *)dst, &dstSize, (const Bytef *)src, size, level);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    return dstSize;
}
