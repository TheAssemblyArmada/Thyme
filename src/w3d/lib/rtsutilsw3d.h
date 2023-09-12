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
#include "endiantype.h"
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

inline unsigned Get_Time()
{
#ifdef PLATFORM_WINDOWS
    return timeGetTime();
#else
    struct timeval now;
    gettimeofday(&now, nullptr);
    return (now.tv_sec * 1000) + (now.tv_usec / 1000);
#endif
}

inline void Sleep_Ms(int interval)
{
#if defined PLATFORM_WINDOWS
    ::Sleep(interval);
#elif defined HAVE_NANOSLEEP
    struct timespec ts;
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;
    nanosleep(&ts, nullptr);
#elif defined HAVE_USLEEP
    usleep(1000 * interval);
#elif defined HAVE_SYS_SELECT_H
    struct timeval tv;
    tv.tv_sec = interval / 1000;
    tv.tv_usec = (interval % 1000) * 1000;
    select(0, nullptr, nullptr, nullptr, &tv);
#else
#error Add sleep function in rtsutilsw3d.h
#endif
}

template<int a, int b, int c, int d> struct FourCC
{
#ifdef SYSTEM_LITTLE_ENDIAN
    static const uint32_t value = (((((d << 8) | c) << 8) | b) << 8) | a;
#else
    static const uint32_t value = (((((a << 8) | b) << 8) | c) << 8) | d;
#endif
};

} // namespace rts
