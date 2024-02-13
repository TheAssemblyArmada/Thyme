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

#include "compiler.h"
#include "platform.h"
#include "stringex.h"
#include "typeoperators.h"

#ifdef PLATFORM_WINDOWS
#include <windef.h>
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

#if defined __cplusplus
template<size_t Size> size_t strlcat_tpl(char (&dst)[Size], const char *src)
{
    return strlcat(dst, src, Size);
}

template<size_t Size> size_t strlcpy_tpl(char (&dst)[Size], const char *src)
{
    return strlcpy(dst, src, Size);
}

template<size_t Size> size_t u_strlcpy_tpl(unichar_t (&dst)[Size], const unichar_t *src)
{
    return u_strlcpy(dst, src, Size);
}

template<size_t Size> size_t u_strlcat_tpl(unichar_t (&dst)[Size], const unichar_t *src)
{
    return u_strlcat(dst, src, Size);
}
#endif

#endif // BASE_ALWAYS_H
