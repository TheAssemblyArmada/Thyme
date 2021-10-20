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
#include <endian.h>

typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(__APPLE__) && defined(__MACH__)
#include <libkern/OSByteOrder.h>

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

typedef UInt16 SwapType16;
typedef UInt32 SwapType32;
typedef UInt64 SwapType64;

#elif defined(__OpenBSD__)
#include <sys/endian.h>

typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
#include <sys/endian.h>

#define be16toh(x) betoh16(x)
#define le16toh(x) letoh16(x)

#define be32toh(x) betoh32(x)
#define le32toh(x) letoh32(x)

#define be64toh(x) betoh64(x)
#define le64toh(x) letoh64(x)

typedef uint16_t SwapType16;
typedef uint32_t SwapType32;
typedef uint64_t SwapType64;

#elif defined(_WIN32)
#include <stdlib.h>

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

typedef unsigned short SwapType16;
typedef unsigned long SwapType32;
typedef unsigned long long SwapType64;

#else
#error platform not supported
#endif

#ifdef __cplusplus

// clang-format off

static_assert(sizeof(SwapType16) == 2, "expected size does not match");
static_assert(sizeof(SwapType32) == 4, "expected size does not match");
static_assert(sizeof(SwapType64) == 8, "expected size does not match");

namespace Detail
{
template <typename Type, size_t Size = sizeof(Type)> struct htobeHelper;
template <typename Type, size_t Size = sizeof(Type)> struct htoleHelper;
template <typename Type, size_t Size = sizeof(Type)> struct betohHelper;
template <typename Type, size_t Size = sizeof(Type)> struct letohHelper;

// 2 byte integer, enum
template <typename Type> struct htobeHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(htobe16(static_cast<SwapType16>(value))); } };
template <typename Type> struct htoleHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(htole16(static_cast<SwapType16>(value))); } };
template <typename Type> struct betohHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(be16toh(static_cast<SwapType16>(value))); } };
template <typename Type> struct letohHelper<Type, 2> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType16>(value))); } };
// 4 byte integer, enum
template <typename Type> struct htobeHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(htobe32(static_cast<SwapType32>(value))); } };
template <typename Type> struct htoleHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(htole32(static_cast<SwapType32>(value))); } };
template <typename Type> struct betohHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(be32toh(static_cast<SwapType32>(value))); } };
template <typename Type> struct letohHelper<Type, 4> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType32>(value))); } };
// 8 byte integer, enum
template <typename Type> struct htobeHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(htobe64(static_cast<SwapType64>(value))); } };
template <typename Type> struct htoleHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(htole64(static_cast<SwapType64>(value))); } };
template <typename Type> struct betohHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(be64toh(static_cast<SwapType64>(value))); } };
template <typename Type> struct letohHelper<Type, 8> { static inline Type swap(Type value) { return static_cast<Type>(le16toh(static_cast<SwapType64>(value))); } };
// float
template <> struct htobeHelper<float, 4> { static inline float swap(float value) { SwapType32 v = htobe32(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct htoleHelper<float, 4> { static inline float swap(float value) { SwapType32 v = htole32(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct betohHelper<float, 4> { static inline float swap(float value) { SwapType32 v = be32toh(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
template <> struct letohHelper<float, 4> { static inline float swap(float value) { SwapType32 v = le16toh(*reinterpret_cast<SwapType32*>(&value)); return *reinterpret_cast<float*>(&v); } };
// double
template <> struct htobeHelper<double, 8> { static inline double swap(double value) { SwapType64 v = htobe64(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct htoleHelper<double, 8> { static inline double swap(double value) { SwapType64 v = htole64(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct betohHelper<double, 8> { static inline double swap(double value) { SwapType64 v = be64toh(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
template <> struct letohHelper<double, 8> { static inline double swap(double value) { SwapType64 v = le16toh(*reinterpret_cast<SwapType64*>(&value)); return *reinterpret_cast<double*>(&v); } };
} // namespace Detail

// c++ template functions, takes any 2, 4, 8 bytes, including float, double, enum

// Host to big endian
template<typename Type> inline Type htobe(Type value) { return Detail::htobeHelper<Type>::swap(value); }
// Host to little endian
template<typename Type> inline Type htole(Type value) { return Detail::htoleHelper<Type>::swap(value); }
// Big endian to host
template<typename Type> inline Type betoh(Type value) { return Detail::betohHelper<Type>::swap(value); }
// Little endian to host
template<typename Type> inline Type letoh(Type value) { return Detail::letohHelper<Type>::swap(value); }

// Host to big endian
template<typename Type> inline void htobe_ref(Type &value) { value = Detail::htobeHelper<Type>::swap(value); }
// Host to little endian
template<typename Type> inline void htole_ref(Type &value) { value = Detail::htoleHelper<Type>::swap(value); }
// Big endian to host
template<typename Type> inline void betoh_ref(Type &value) { value = Detail::betohHelper<Type>::swap(value); }
// Little endian to host
template<typename Type> inline void letoh_ref(Type &value) { value = Detail::letohHelper<Type>::swap(value); }

// clang-format on

#endif // __cplusplus

// Byte access macros for different word sizes for Little Endian.
#if defined(_WIN32) || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define GETBYTE32(x, n)    (*((uint8_t*)&(x)+n))
#define GETSBYTE32(x, n)   (*((int8_t*)&(x)+n))
#define GETBYTE16(x, n)    (*((uint8_t*)&(x)+n))
#define GETSBYTE16(x, n)   (*((int8_t*)&(x)+n))
#define GETWORD32(x, n)    (*((uint16_t*)&(x)+n))
#define GETSWORD32(x, n)   (*((int16_t*)&(x)+n))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define GETBYTE32(x, n)    (*((uint8_t*)&(x)+(3-n)))
#define GETSBYTE32(x, n)   (*((int8_t*)&(x)+(3-n)))
#define GETBYTE16(x, n)    (*((uint8_t*)&(x)+(1-n)))
#define GETSBYTE16(x, n)   (*((int8_t*)&(x)+(1-n)))
#define GETWORD32(x, n)    (*((uint16_t*)&(x)+(1-n)))
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
