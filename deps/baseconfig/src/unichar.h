/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Header smoothing the differences between ICU and standard WinAPI.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_UNICHAR_H
#define BASE_UNICHAR_H
#include <wchar.h>

#ifdef BUILD_WITH_ICU
#include <unicode/uchar.h>
typedef UChar unichar_t;
#elif defined _WIN32
typedef wchar_t unichar_t;
#define u_strlen wcslen
#define u_strcpy wcscpy
#define u_strcat wcscat
#define u_vsnprintf_u vswprintf
#define u_strcmp wcscmp
#define u_strcasecmp(x, y, z) _wcsicmp(x, y)
#define u_isspace iswspace
#define u_tolower towlower
#define u_strchr(x, y) wcschr(x, y)
#define U_COMPARE_CODE_POINT_ORDER 0x8000
#else
#error This platform requires ICU to support unichar.
#endif

#if __has_attribute(__may_alias__)
typedef unichar_t __attribute__((__may_alias__)) unichar_a;
#else
typedef unichar_t unichar_a;
#endif

#endif
