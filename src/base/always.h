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

#include "bittype.h"
#include "config.h"
#include "macros.h"
#include "targetver.h"
#include <sys/stat.h>
#include <sys/types.h>

#if defined(PLATFORM_WINDOWS)
#define NOMINMAX
#include <windows.h>
#define NAME_MAX FILENAME_MAX

#if !defined(PATH_MAX)
#define PATH_MAX MAX_PATH
#endif

// Temp until build system is refactored to add it.
#define D3D8_BUILD

#endif

// Alias the ICU unicode functions when not building against it.
#ifndef THYME_USE_ICU
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

// Few defines to keep things straight between windows and posix
#if defined(PLATFORM_WINDOWS)
// Not sure what flags need passing to MinGW-w64 GCC to get wcscasecmp without this
#define wcscasecmp(a, b) _wcsicmp(a, b)
#define localtime_r(a, b) localtime_s(b, a)
typedef struct stat stat_t;

#if !defined(PLATFORM_MINGW)
#define strdup(s) _strdup(s)
#define strcasecmp(a, b) _stricmp(a, b)
#define strncasecmp(a, b, c) _strnicmp(a, b, c)
#endif

// These are apparently implemented correctly in MSVC 2015
#if defined(COMPILER_MSVC) && (COMPILER_VERSION < 1900)
#define snprintf(...) _snprintf_s(__VA_ARGS__)
#define vsnprintf(...) _vsnprintf_s(__VA_ARGS__)
#endif

#else
typedef struct stat stat_t;
#endif // PLATFORM_WINDOWS
