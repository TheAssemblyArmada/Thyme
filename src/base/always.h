/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Basic header files and defines that are always needed.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "config.h"

#include "bittype.h"
#include "compiler.h"
#include "intrinsics.h"
#include "macros.h"
#include "platform.h"
#include "stringex.h"
#include "targetver.h"
#include <sys/stat.h>
#include <sys/types.h>

#ifdef PLATFORM_WINDOWS
#define NOMINMAX 1
#include <windows.h>
// Include after windows.h
#include "utf.h"
#define NAME_MAX FILENAME_MAX

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

// Enable inline recursion for MSVC
#ifdef COMPILER_MSVC
#pragma inline_recursion(on)
#endif

// This section defines some keywords defining calling conventions
// where the keywords needed differ between compilers.
#if !defined COMPILER_MSVC
#if !defined(__fastcall)
#if __has_attribute(fastcall)
#define __fastcall __attribute__((fastcall))
#else
#define __fastcall
#endif
#endif

#if !defined(__cdecl)
#if __has_attribute(cdecl)
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif

#if !defined(__stdcall)
#if __has_attribute(stdcall)
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif
#endif // !defined COMPILER_MSVC

// This section defines some keywords controlling inlining and unused variables
// where the keywords needed differ between compilers.
#ifdef COMPILER_MSVC
#define __noinline __declspec(noinline)
#define __unused __pragma(warning(suppress : 4100 4101))
#define __mayalias
#else
#if !defined(__forceinline)
#if __has_attribute(__always_inline__)
#define __forceinline inline __attribute__((__always_inline__))
#else
#define __forceinline inline
#endif
#endif

#if !defined(__unused)
#if __has_attribute(unused)
#define __unused __attribute__((unused))
#else
#define __unused
#endif
#endif

#if !defined(__noinline)
#if __has_attribute(noinline)
#define __noinline __attribute__((noinline))
#else
#define __noinline
#endif
#endif

#ifndef __mayalias
#if __has_attribute(__may_alias__)
#define __mayalias __attribute__((__may_alias__))
#else
#define __mayalias
#endif
#endif
#endif // COMPILER_MSVC

// Alias the ICU unicode functions when not building against it.
#if !defined BUILD_WITH_ICU && defined PLATFORM_WINDOWS
#define u_strlen wcslen
#define u_strcpy wcscpy
#define u_strcat wcscat
#define u_vsnprintf_u vswprintf
#define u_strcmp wcscmp
#define u_strcasecmp(x, y, z) _wcsicmp(x, y)
#define u_isspace iswspace
#define u_tolower towlower
#define U_COMPARE_CODE_POINT_ORDER 0x8000
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

typedef struct stat stat_t;
