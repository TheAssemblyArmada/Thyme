/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Macros for handling different platform endian formats.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_ENDIANTYPE_H
#define BASE_ENDIANTYPE_H

#include "bittype.h"

#if defined(__linux__)
#include	<endian.h>

#elif defined(__APPLE__) && defined(__MACH__)
#include	<libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
 
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
 
#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#elif defined(__OpenBSD__)
#include	<sys/endian.h>

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include	<sys/endian.h>

#define be16toh(x) betoh16(x)
#define le16toh(x) letoh16(x)

#define be32toh(x) betoh32(x)
#define le32toh(x) letoh32(x)

#define be64toh(x) betoh64(x)
#define le64toh(x) letoh64(x)

//
// If building for Windows...
//
#elif defined(_WIN32)
#include	<stdlib.h>
    #define htobe16(x) _byteswap_ushort(x)
    #define htole16(x) (x)
    #define be16toh(x) _byteswap_ushort(x)
    #define le16toh(x) (x)
 
    #define htobe32(x) _byteswap_ulong(x)
    #define htole32(x) (x)
    #define be32toh(x) _byteswap_ulong(x)
    #define le32toh(x) (x)
 
    #define htobe64(x) _byteswap_uint64(x)
    #define htole64(x) (x)
    #define be64toh(x) _byteswap_uint64(x)
    #define le64toh(x) (x)
// Unsupported platform, report this to the compiler.
#else
#error platform not supported
#endif 	// Platform specifics

// Byte access macros for different word sizes for Little Endian.
#if defined(_WIN32) || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define GETBYTE32(x, n)    (*((uint8_t*)&(x)+n))
    #define GETSBYTE32(x, n)   (*((int8_t*)&(x)+n))
    #define GETBYTE16(x, n)    (*((uint8_t*)&(x)+n))
    #define GETSBYTE16(x, n)   (*((int8_t*)&(x)+n))
    #define GETWORD32(x, n)    (*((uint16_t*)&(x)+n))
    #define GETSWORD32(x, n)   (*((int16_t*)&(x)+n))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define GETBYTE32(x, n)	   (*((uint8_t*)&(x)+(3-n)))
    #define GETSBYTE32(x, n)   (*((int8_t*)&(x)+(3-n)))
    #define GETBYTE16(x, n)	   (*((uint8_t*)&(x)+(1-n)))
    #define GETSBYTE16(x, n)   (*((int8_t*)&(x)+(1-n)))
    #define GETWORD32(x, n)	   (*((uint16_t*)&(x)+(1-n)))
    #define GETSWORD32(x, n)   (*((int16_t*)&(x)+(1-n)))
#else
#error byte order not supported
#endif

#define WRITE_LE_UINT16(p, value) ((p)[0] = ((value) & 0xFF), (p)[1] = (((value) >> 8) & 0xFF))
#define WRITE_LE_UINT32(p, value) ((p)[0] = ((value) & 0xFF), (p)[1] = (((value) >> 8) & 0xFF), (p)[2] = (((value) >> 16) & 0xFF), (p)[3] = (((value) >> 24) & 0xFF))
#define READ_LE_UINT16(p) ((uint16_t)(p)[0] | ((uint16_t)(p)[1] << 8))
#define READ_LE_UINT32(p) ((uint32_t)(p)[0] | ((uint32_t)(p)[1] << 8) | ((uint32_t)(p)[2] << 16) | ((uint32_t)(p)[3] << 24))
#define GETBYTE(x, n)    (*((uint8_t*)&(x)+n))
#define GETSBYTE(x, n)   (*((int8_t*)&(x)+n))

#endif // BASE_ENDIANTYPE_H
