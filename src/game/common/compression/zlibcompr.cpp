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
int Zlib_Uncompress(void *dst, const void *src, int *size)
{
    if (size == nullptr) {
        captainslog_error("Missing srcSize for Zlib data");
        return 0;
    }

    uLongf dstSize = 0;
    uLong srcSize = *size;

    int result = uncompress((Bytef *)dst, &dstSize, (const Bytef *)src, srcSize);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    if (size != nullptr) {
        *size = dstSize;
    }
    return dstSize;
}

/**
 * Compresses to zlib data.
 */
int Zlib_Compress(void *dst, const void *src, int size, int level)
{
    uLongf dstSize = 0;
    int result = compress2((Bytef *)dst, &dstSize, (const Bytef *)src, size, level);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    return dstSize;
}
