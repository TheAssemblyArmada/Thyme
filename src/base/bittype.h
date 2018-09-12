/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Fixed width types.
 *
 * Provides standard definitions for fixed width integers on all supported platforms and types to use for type punning
 * pointers.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

// We aren't supporting MSVC versions earlier than 2010
#if defined(__GNUC__) || defined(__clang__) || defined(__WATCOM__) || _MSC_VER >= 1600 // GNU C++ or clang
#include <limits.h>
#include <stdint.h>
#else // !_MSC_VER || !(__GNUC__ || __clang__ || __WATCOM__)
#error Unknown compiler.Please specify fixed - size integer types in 'bittype.h'.
#endif // _MSC_VER || (__GNUC__ || __clang__ || __WATCOM__)

#ifdef THYME_USE_ICU
#include <unicode/umachine.h>
typedef UChar unichar_t;
#else
typedef wchar_t unichar_t;
#endif

// Use these as pointers to do type punning.
#if defined(__GNUC__) || defined(__clang__)
typedef float __attribute__((__may_alias__)) float_a;
typedef int32_t __attribute__((__may_alias__)) int32_a;
typedef uint32_t __attribute__((__may_alias__)) uint32_a;
typedef double __attribute__((__may_alias__)) double_a;
typedef int64_t __attribute__((__may_alias__)) int64_a;
typedef uint64_t __attribute__((__may_alias__)) uint64_a;
typedef unichar_t __attribute__((__may_alias__)) unichar_a;
#else
// MSVC doesn't currently enforce strict aliasing.
typedef float float_a;
typedef int32_t int32_a;
typedef uint32_t uint32_a;
typedef double double_a;
typedef int64_t int64_a;
typedef uint64_t uint64_a;
typedef unichar_t unichar_a;
#endif

// Union to use for type punning.
union float_int_tp
{
    uint32_a integer;
    float_a real;
};
