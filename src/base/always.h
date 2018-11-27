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

//  Define nullptr when standard is less than C++x0
#if __cplusplus <= 199711L && !defined COMPILER_MSVC
#define nullptr NULL
#endif

#if defined(COMPILER_MSVC)
// Allow inline recursive functions within inline recursive functions.
#pragma inline_recursion(on)

#define __noinline __declspec(noinline)
#define __unused __pragma(warning(suppress : 4100 4101))
#define __mayalias
#pragma warning(push, 3)

#else // !COMPILER_MSVC
#if defined(COMPILER_GNUC) || defined(COMPILER_CLANG)
#if defined(PROCESSOR_X86) // Only applies to 32bit
#if !defined(__fastcall)
#define __fastcall __attribute__((fastcall))
#endif
#if !defined(__cdecl)
#define __cdecl __attribute__((cdecl))
#endif
#if !defined(__stdcall)
#define __stdcall __attribute__((stdcall))
#endif
#else
#if !defined(__fastcall)
#define __fastcall
#endif
#if !defined(__cdecl)
#define __cdecl
#endif
#if !defined(__stdcall)
#define __stdcall
#endif
#endif

#define __noinline __attribute__((noinline))
#define __unused __attribute__((unused))
#define __mayalias __attribute__((__may_alias__))

#if !defined(__forceinline)
#define __forceinline inline __attribute__((__always_inline__))
#endif
#else // !COMPILER_GNUC || !COMPILER_CLANG
#if !defined(__fastcall)
#define __fastcall
#endif
#if !defined(__cdecl)
#define __cdecl
#endif
#if !defined(__stdcall)
#define __stdcall
#endif
#if !defined(__forceinline)
#define __forceinline inline
#endif
#define __noinline
#define __unused
#define __mayalias
#endif // COMPILER_GNUC || COMPILER_CLANG

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
