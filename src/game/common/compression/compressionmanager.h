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

#ifndef COMPRESSIONMANAGER_H
#define COMPRESSIONMANAGER_H

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

class CompressionManager
{
public:
    static bool Is_Data_Compressed(const void *data, int size);
    static CompressionType Get_Compression_Type(const void *data, int size);
    static int Get_Uncompressed_Size(const void *data);
    static int Decompress_Data(void *src, int src_size, void *dst, int dst_size);
    static const char *Get_Compression_Name(CompressionType type) { return s_compressionNames[type]; }
#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    static const char *s_compressionNames[COMPRESSION_COUNT];
};

#ifndef THYME_STANDALONE
#include "hooker.h"

inline void CompressionManager::Hook_Me()
{
    Hook_Function(0x006F5FE0, &Get_Compression_Type);
    Hook_Function(0x006F6110, &Decompress_Data);
}
#endif

#endif
