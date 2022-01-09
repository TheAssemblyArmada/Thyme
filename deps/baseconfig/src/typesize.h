/**
 * @file
 *
 * @author xezon
 *
 * @brief Basic header files and defines that are always needed.
 *
 * @copyright BaseConfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_TYPESIZE_H
#define BASE_TYPESIZE_H

#include "bittype.h"

#ifdef __cplusplus
extern "C++" {

// clang-format off

template<size_t Size> struct SignedIntegerForSize;
template<size_t Size> struct UnsignedIntegerForSize;

template<> struct SignedIntegerForSize<1>{ typedef int8_t  type; };
template<> struct SignedIntegerForSize<2>{ typedef int16_t type; };
template<> struct SignedIntegerForSize<4>{ typedef int32_t type; };
template<> struct SignedIntegerForSize<8>{ typedef int64_t type; };

template<> struct UnsignedIntegerForSize<1>{ typedef uint8_t  type; };
template<> struct UnsignedIntegerForSize<2>{ typedef uint16_t type; };
template<> struct UnsignedIntegerForSize<4>{ typedef uint32_t type; };
template<> struct UnsignedIntegerForSize<8>{ typedef uint64_t type; };

template<class T> struct SignedInteger{ typedef typename SignedIntegerForSize<sizeof(T)>::type type; };
template<class T> struct UnsignedInteger{ typedef typename UnsignedIntegerForSize<sizeof(T)>::type type; };

// clang-format on
}
#endif // __cplusplus

#endif // BASE_TYPESIZE_H
