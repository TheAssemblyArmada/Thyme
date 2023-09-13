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

#include "always.h"
#include "asciistring.h"
#include "endiantype.h"
#include "rtsutilsw3d.h"
#include <ctime>

#ifdef PLATFORM_WINDOWS
#include <mmsystem.h>
#include <synchapi.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_SELECT
#include <sys/select.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

namespace rts
{

// memset, but for patterns larger than a byte.
template<typename T> void tmemset(void *dst, T value, size_t size)
{
    size_t i;
    for (i = 0; i < (size & (~(sizeof(value) - 1))); i += sizeof(value)) {
        memcpy(((char *)dst) + i, &value, sizeof(value));
    }

    for (; i < size; i++) {
        ((char *)dst)[i] = ((char *)&value)[i & (sizeof(value) - 1)];
    }
}

// Less than comparator for STL containers.
// Use only for string classes.
template<typename T> struct less_than_nocase
{
    bool operator()(const T &left, const T &right) const { return (left.Compare_No_Case(right) < 0); }
};

template<typename T> struct hash
{
    size_t operator()(const T &object) const { return static_cast<size_t>(object); }
};

template<> struct hash<Utf8String>
{
    size_t operator()(const Utf8String &object) const
    {
        const char *c = object.Str();
        // This is the same implementation as STL Ports const char* hashing function
        // We do not call the STL Ports version as it is non C++ standard
        size_t hash = 0;
        for (; *c != '\0'; ++c) {
            hash = *c + 5 * hash;
        }

        return hash;
    }
};

inline uint32_t FourCC_From_String(const char *str)
{
    char buf[5] = {};
    strlcpy_tpl(buf, str);
#ifdef SYSTEM_LITTLE_ENDIAN
    return (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
#else
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
#endif
}

} // namespace rts
