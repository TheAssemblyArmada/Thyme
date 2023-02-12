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
#pragma once

#include "always.h"

enum CompressionType : int32_t
{
    COMPRESSION_NONE,
    COMPRESSION_EAR, // RefPack
    COMPRESSION_NOX, // LZH Light
    COMPRESSION_ZL1,
    COMPRESSION_ZL2,
    COMPRESSION_ZL3,
    COMPRESSION_ZL4,
    COMPRESSION_ZL5,
    COMPRESSION_ZL6,
    COMPRESSION_ZL7,
    COMPRESSION_ZL8,
    COMPRESSION_ZL9,
    COMPRESSION_EAB, // BTree
    COMPRESSION_EAH, // Huffman
    COMPRESSION_COUNT,
};

struct ComprHeader
{
    char fourcc[4];
    uint32_t uncomp_size;
};

class CompressionManager
{
public:
    static bool Is_Data_Compressed(const void *data, int size);
    static CompressionType Get_Prefered_Compression();
    static CompressionType Get_Compression_Type(const void *data, int size);
    static int Get_Max_Compressed_Size(int size, CompressionType type);
    // Thyme specific: Get the FourCC for this compression type
    static uint32_t Get_Compression_FourCC(CompressionType type);
    // Thyme specific: Get the compression type by the FourCC
    static CompressionType Get_Compression_Type_By_FourCC(uint32_t fourcc);
    static int Get_Uncompressed_Size(const void *data, int size);
    static int Compress_Data(CompressionType type, void *src, int src_size, void *dst, int dst_size);
    static int Decompress_Data(void *src, int src_size, void *dst, int dst_size);
    static const char *Get_Compression_Name(CompressionType type) { return s_compressionNames[type]; }

private:
    static const char *s_compressionNames[COMPRESSION_COUNT];
};
