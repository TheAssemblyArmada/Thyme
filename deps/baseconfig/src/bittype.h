/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Includes for defined width integers and aliasing versions for type punning.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_BITTYPE_H
#define BASE_BITTYPE_H

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS

#include "macros.h"

#if _MSC_VER >= 1600 || !defined _MSC_VER
#include <inttypes.h>
#else
    
#if (_MSC_VER < 1300)
typedef signed char       int8_t;
typedef signed short      int16_t;
typedef signed int        int32_t;
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned int      uint32_t;
#else
typedef signed __int8     int8_t;
typedef signed __int16    int16_t;
typedef signed __int32    int32_t;
typedef unsigned __int8   uint8_t;
typedef unsigned __int16  uint16_t;
typedef unsigned __int32  uint32_t;
#endif
typedef signed __int64       int64_t;
typedef unsigned __int64     uint64_t;

typedef int8_t    int_least8_t;
typedef int16_t   int_least16_t;
typedef int32_t   int_least32_t;
typedef int64_t   int_least64_t;
typedef uint8_t   uint_least8_t;
typedef uint16_t  uint_least16_t;
typedef uint32_t  uint_least32_t;
typedef uint64_t  uint_least64_t;

typedef int8_t    int_fast8_t;
typedef int16_t   int_fast16_t;
typedef int32_t   int_fast32_t;
typedef int64_t   int_fast64_t;
typedef uint8_t   uint_fast8_t;
typedef uint16_t  uint_fast16_t;
typedef uint32_t  uint_fast32_t;
typedef uint64_t  uint_fast64_t;

#ifdef _WIN64
typedef signed __int64    intptr_t;
typedef unsigned __int64  uintptr_t;
#else
typedef _W64 signed int   intptr_t;
typedef _W64 unsigned int uintptr_t;
#endif

#define INT8_MIN     ((int8_t)_I8_MIN)
#define INT8_MAX     _I8_MAX
#define INT16_MIN    ((int16_t)_I16_MIN)
#define INT16_MAX    _I16_MAX
#define INT32_MIN    ((int32_t)_I32_MIN)
#define INT32_MAX    _I32_MAX
#define INT64_MIN    ((int64_t)_I64_MIN)
#define INT64_MAX    _I64_MAX
#define UINT8_MAX    _UI8_MAX
#define UINT16_MAX   _UI16_MAX
#define UINT32_MAX   _UI32_MAX
#define UINT64_MAX   _UI64_MAX

#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST8_MAX    INT8_MAX
#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST16_MAX   INT16_MAX
#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST32_MAX   INT32_MAX
#define INT_LEAST64_MIN   INT64_MIN
#define INT_LEAST64_MAX   INT64_MAX
#define UINT_LEAST8_MAX   UINT8_MAX
#define UINT_LEAST16_MAX  UINT16_MAX
#define UINT_LEAST32_MAX  UINT32_MAX
#define UINT_LEAST64_MAX  UINT64_MAX

#define INT_FAST8_MIN    INT8_MIN
#define INT_FAST8_MAX    INT8_MAX
#define INT_FAST16_MIN   INT16_MIN
#define INT_FAST16_MAX   INT16_MAX
#define INT_FAST32_MIN   INT32_MIN
#define INT_FAST32_MAX   INT32_MAX
#define INT_FAST64_MIN   INT64_MIN
#define INT_FAST64_MAX   INT64_MAX
#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT16_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX

#ifdef _WIN64
#define INTPTR_MIN   INT64_MIN
#define INTPTR_MAX   INT64_MAX
#define UINTPTR_MAX  UINT64_MAX
#else
#define INTPTR_MIN   INT32_MIN
#define INTPTR_MAX   INT32_MAX
#define UINTPTR_MAX  UINT32_MAX
#endif

#define INTMAX_MIN   INT64_MIN
#define INTMAX_MAX   INT64_MAX
#define UINTMAX_MAX  UINT64_MAX

#ifdef _WIN64
#define PTRDIFF_MIN  _I64_MIN
#define PTRDIFF_MAX  _I64_MAX
#else
#define PTRDIFF_MIN  _I32_MIN
#define PTRDIFF_MAX  _I32_MAX
#endif

#define SIG_ATOMIC_MIN  INT_MIN
#define SIG_ATOMIC_MAX  INT_MAX

#ifndef SIZE_MAX
#ifdef _WIN64
#define SIZE_MAX  _UI64_MAX
#else
#define SIZE_MAX  _UI32_MAX
#endif
#endif

#define WINT_MIN  0
#define WINT_MAX  _UI16_MAX

#define INT8_C(val)  val##i8
#define INT16_C(val) val##i16
#define INT32_C(val) val##i32
#define INT64_C(val) val##i64

#define UINT8_C(val)  val##ui8
#define UINT16_C(val) val##ui16
#define UINT32_C(val) val##ui32
#define UINT64_C(val) val##ui64

#define INTMAX_C   INT64_C
#define UINTMAX_C  UINT64_C

#endif

#include <limits.h>
#include <stddef.h>
#include <sys/types.h>

#ifndef _MSC_VER
typedef int BOOL; // For ABI compatibility with old code only, do not use in new code.
#endif

// Use these when using pointers to do type punning.
#if __has_attribute(__may_alias__)
    typedef float __attribute__((__may_alias__)) float_a;
    typedef int32_t __attribute__((__may_alias__)) int32_a;
    typedef uint32_t __attribute__((__may_alias__)) uint32_a;
    typedef double __attribute__((__may_alias__)) double_a;
    typedef int64_t __attribute__((__may_alias__)) int64_a;
    typedef uint64_t __attribute__((__may_alias__)) uint64_a;
#else
    // MSVC doesn't currently enforce strict aliasing.
    typedef float float_a;
    typedef int32_t int32_a;
    typedef uint32_t uint32_a;
    typedef double double_a;
    typedef int64_t int64_a;
    typedef uint64_t uint64_a;
#endif

// Union to use for type punning floats.
union float_int_tp
{
    uint32_a integer;
    float_a real;
};

#endif // BASE_BITTYPE_H
