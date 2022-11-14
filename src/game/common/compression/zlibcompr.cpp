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
    if (src_size <= 0) {
        captainslog_error("Missing src_size for Zlib data");
        return 0;
    }

    uLongf zlib_dst_size = dst_size;
    uLong zlib_src_size = src_size;

    int result =
        uncompress(static_cast<Bytef *>(dst), &zlib_dst_size, static_cast<const Bytef *>(src), zlib_src_size);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    return zlib_dst_size;
}

/**
 * Compresses to zlib data.
 */
int Zlib_Compress(void *dst, int dst_size, const void *src, int src_size, int level)
{
    uLongf zlib_dst_size = dst_size;
    int result = compress2(static_cast<Bytef *>(dst), &zlib_dst_size, static_cast<const Bytef *>(src), src_size, level);
    if (result != Z_OK) {
        captainslog_error("Failed to compress Zlib data. Error=%i", result);
        return 0;
    }

    return zlib_dst_size;
}

/**
 * Returns the max size for the specified amount of bytes.
 */
int Zlib_MaxSize(int size)
{
    return compressBound(size);
}
