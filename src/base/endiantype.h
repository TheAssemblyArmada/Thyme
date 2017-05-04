////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ENDIANTYPE.H
//
//        Author:: OmniBlade
//
//  Contributors:: CCHyper
//
//   Description:: Base class for archive file handling.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef BASE_ENDIANTYPE_H
#define BASE_ENDIANTYPE_H


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include	"bittype.h"

//
// If building for Linux...
//
#if defined(PLATFORM_LINUX)
#include	<endian.h>

//
// If building for Mac OS X...
//
#elif defined(PLATFORM_OSX)
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

//
// If building for OpenBSD...
//
#elif defined(__OpenBSD__)
#include	<sys/endian.h>

//
// If building for NetBSD, FreeBSD or DragonFly...
//
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
#elif defined(PLATFORM_WINDOWS)
#include	<stdlib.h>

#if defined(SYSTEM_LITTLE_ENDIAN)
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
#endif // SYSTEM_LITTLE_ENDIAN

//
// Unsupported platform, report this to the compiler.
//
#else
COMPILER_ERROR("platform not supported");
#endif 	//PLATFORM_LINUX || PLATFORM_OSX || PLATFORM_WINDOWS

//
// Byte access macros for different word sizes for Little Endian.
//
#if defined(SYSTEM_LITTLE_ENDIAN)
    #define GETBYTE32(x, n)    (*((uint8*)&(x)+n))
    #define GETSBYTE32(x, n)   (*((sint8*)&(x)+n))
    #define GETBYTE16(x, n)    (*((uint8*)&(x)+n))
    #define GETSBYTE16(x, n)   (*((sint8*)&(x)+n))
    #define GETWORD32(x, n)    (*((uint16*)&(x)+n))
    #define GETSWORD32(x, n)   (*((sint16*)&(x)+n))

    #define htolef(x)           (x)
    #define leftoh(x)           (x)

//
// Byte access macros for different word sizes for Big Endian.
//
#elif defined(SYSTEM_BIG_ENDIAN)
    #define GETBYTE32(x, n)	   (*((uint8*)&(x)+(3-n)))
    #define GETSBYTE32(x, n)   (*((sint8*)&(x)+(3-n)))
    #define GETBYTE16(x, n)	   (*((uint8*)&(x)+(1-n)))
    #define GETSBYTE16(x, n)   (*((sint8*)&(x)+(1-n)))
    #define GETWORD32(x, n)	   (*((uint16*)&(x)+(1-n)))
    #define GETSWORD32(x, n)   (*((sint16*)&(x)+(1-n)))

    #define htolef(x)           *(float*)(&(htole32(*(uint32_a*)(&x))))
    #define leftoh(x)           *(float*)(&(le32toh(*(uint32_a*)(&x))))
//
// Unsupported byte order, report this to the compiler.
//
#else
COMPILER_ERROR("byte order not supported");
#endif

////////////////////////////////////////////////////////////////////////////////
//  
////////////////////////////////////////////////////////////////////////////////
#define WRITE_LE_UINT16(p, value) ((p)[0] = ((value) & 0xFF), (p)[1] = (((value) >> 8) & 0xFF))
#define WRITE_LE_UINT32(p, value) ((p)[0] = ((value) & 0xFF), (p)[1] = (((value) >> 8) & 0xFF), (p)[2] = (((value) >> 16) & 0xFF), (p)[3] = (((value) >> 24) & 0xFF))
#define READ_LE_UINT16(p) ((uint16)(p)[0] | ((uint16)(p)[1] << 8))
#define READ_LE_UINT32(p) ((uint32)(p)[0] | ((uint32)(p)[1] << 8) | ((uint32)(p)[2] << 16) | ((uint32)(p)[3] << 24))
#define GETBYTE(x, n)    (*((uint8*)&(x)+n))
#define GETSBYTE(x, n)   (*((sint8*)&(x)+n))

#endif	// _ENDIAN_H
