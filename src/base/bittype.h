/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Includes for defined width integers and aliasing versions for type punning.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "macros.h"
#include <inttypes.h>
#include <limits.h>

#ifdef BUILD_WITH_ICU
#include <unicode/uchar.h>
typedef UChar unichar_t;
#else
typedef wchar_t unichar_t;
#endif

// Use these when using pointers to do type punning.
#if __has_attribute(__may_alias__)
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

// Union to use for type punning floats.
union float_int_tp
{
    uint32_a integer;
    float_a real;
};
