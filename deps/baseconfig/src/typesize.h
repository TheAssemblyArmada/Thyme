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

template<size_t S> struct IntegerForSize;

template<> struct IntegerForSize<1>
{
    typedef int8_t type;
};

template<> struct IntegerForSize<2>
{
    typedef int16_t type;
};

template<> struct IntegerForSize<4>
{
    typedef int32_t type;
};

template<> struct IntegerForSize<8>
{
    typedef int64_t type;
};

// Used as an approximation of std::underlying_type<T>
template<class T> struct SizedInteger
{
    typedef typename IntegerForSize<sizeof(T)>::type type;
};
}
#endif // __cplusplus

#endif // BASE_TYPESIZE_H
