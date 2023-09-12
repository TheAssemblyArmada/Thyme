/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for generating Cyclic Redundancy Checks.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "realcrc.h"
#include <cctype>

using std::toupper;

template<typename T, int N> inline constexpr Array<T, N> calculateCRC()
{
    Array<T, N> lookup;
    const unsigned long POLYNOMIAL = 0xEDB88320;
    unsigned long remainder = 0;
    unsigned char b = 0;
    do {
        // Start with the data byte
        remainder = b;
        for (unsigned long bit = 8; bit > 0; --bit) {
            if (remainder & 1)
                remainder = (remainder >> 1) ^ POLYNOMIAL;
            else
                remainder = (remainder >> 1);
        }
        lookup[(size_t)b] = remainder;
    } while (0 != ++b);

    return lookup;
}

const Array<uint32_t, 256> g_crc32Table = calculateCRC<uint32_t, 256>();

uint32_t CRC_Memory(void const *data, size_t bytes, uint32_t crc)
{
    uint8_t const *buf = static_cast<uint8_t const *>(data);
    crc = ~crc;

    for (unsigned int i = 0; i < bytes; ++i) {
        crc = g_crc32Table[buf[i] ^ (uint8_t)crc] ^ (crc >> 8);
    }

    return ~crc;
}

uint32_t CRC_String(const char *string, uint32_t crc)
{
    uint8_t const *buf = reinterpret_cast<uint8_t const *>(string);
    crc = ~crc;

    for (; *buf != 0; ++buf) {
        crc = g_crc32Table[*buf ^ (uint8_t)crc] ^ (crc >> 8);
    }

    return ~crc;
}

uint32_t CRC_Stringi(char const *string, uint32_t crc)
{
    uint8_t const *buf = reinterpret_cast<uint8_t const *>(string);
    crc = ~crc;

    for (; *buf != 0; ++buf) {
        crc = g_crc32Table[toupper(*buf) ^ (uint8_t)crc] ^ (crc >> 8);
    }

    return ~crc;
}
