/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Basic header files and defines that are always needed.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_ALWAYS_H
#define BASE_ALWAYS_H

#include "config.h"
#include "targetver.h"

#include "bittype.h"
#include "compiler.h"
#include "intrinsics.h"
#include "macros.h"
#include "platform.h"
#include "stringex.h"
#include "unichar.h"

#ifdef PLATFORM_WINDOWS
#include <windef.h>
#include "utf.h"
#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#endif

// Enable inline recursion for MSVC
#ifdef COMPILER_MSVC
#pragma inline_recursion(on)
#endif

// Based on the build system generated config.h information we define some stuff here
// for cross platform consistency.
#if defined HAVE__STRICMP && !defined HAVE_STRCASECMP
#define strcasecmp _stricmp
#endif

#if defined HAVE__STRNICMP && !defined HAVE_STRNCASECMP
#define strncasecmp _strnicmp
#endif

#ifndef HAVE_STRLWR
#define strlwr ex_strlwr
#endif

#ifndef HAVE_STRUPR
#define strupr ex_strupr
#endif

#ifndef HAVE_STRLCAT
#define strlcat ex_strlcat
#endif

#ifndef HAVE_STRLCPY
#define strlcpy ex_strlcpy
#endif

#ifndef HAVE_STRTRIM
#define strtrim ex_strtrim
#endif

#if !defined HAVE_STD_CLAMP && defined __cplusplus
#include <functional>

namespace std
{
    template<class T, class Compare>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi, Compare comp)
    {
        return comp(v, lo) ? lo : comp(hi, v) ? hi : v;
    }

    template<class T>
    constexpr const T &clamp(const T &v, const T &lo, const T &hi)
    {
        return clamp(v, lo, hi, std::less<T>());
    }
}
#endif

#endif // BASE_ALWAYS_H
