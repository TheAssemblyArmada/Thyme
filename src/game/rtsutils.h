/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Useful code that doesn't really fit anywhere else.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef RTSUTILS_H
#define RTSUTILS_H

#include "always.h"
#include "endiantype.h"

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#else
#include <sys/time.h>
#endif

namespace rts {

// memset, but for patterns larger than a byte.
template<typename T>
void tmemset(void *dst, T value, size_t size)
{
    size_t i;
    for ( i = 0; i < (size & (~(sizeof(value) - 1))); i += sizeof(value) ) {
        memcpy(((char*)dst) + i, &value, sizeof(value));
    }

    for ( ; i < size; i++ ) {
        ((char*)dst)[i] = ((char*)&value)[i & (sizeof(value) - 1)];
    }
}

// Less than comparator for STL containers.
// Use only for string classes.
template<typename T>
struct less_than_nocase
{
    bool operator()(const T &left, const T &right) const
    {
        return (left.Compare_No_Case(right) < 0);
    }
};

template<typename T>
struct equal_to
{
    bool operator()(const T &left, const T &right) const
    {
        return (left.Compare(right) == 0);
    }
};

template<typename T>
struct hash
{
    size_t operator()(const T &object) const
    {
        const char *c = reinterpret_cast<const char*>(object.Str());
        size_t hash = 0;

        do {
            hash = *c + 5 * hash;
        } while ( ++c );

        return hash;
    }
};

template <int a, int b, int c, int d>
struct FourCC
{
#ifdef SYSTEM_LITTLE_ENDIAN
    static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
#else
    static const uint32_t value = (((((a << 8) | b) << 8) | c) << 8) | d;
#endif
};

inline unsigned Get_Time()
{
#ifdef PLATFORM_WINDOWS
	return timeGetTime();
#else
	struct timeval now;
	gettimeofday(&now, nullptr);
	return now.tv_usec / 1000;
#endif
}

} // namespace rts

#endif // _RTSUTILS_H
