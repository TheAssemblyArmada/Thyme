/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 *
 * @brief Basic header files and defines that are always needed.
 *
 * @copyright BaseConfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_MACROS_H
#define BASE_MACROS_H

// The ubiquitous stringify macros for formatting strings.
#ifndef STRINGIZE
#define STRINGIZE_HELPER(str) #str
#define STRINGIZE(str) STRINGIZE_HELPER(str)
#define STRINGIZE_JOIN(str1, str2) STRINGIZE_HELPER(str1 ## str2)
#endif // STRINGIZE

// Define some C++ keywords when standard is less than C++11, mainly for watcom support
#if __cplusplus <= 199711L && (!defined _MSC_VER || _MSC_VER < 1600)
#define nullptr NULL
#define override
#define final
#define static_assert(x, ...)
#define constexpr
#define noexcept
#endif

// These allow evaluation of compiler specific attributes and intrinics on GCC like compilers.
// If they don't exist we want them to evaluate to false.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

// This section defines some keywords defining calling conventions
// where the keywords needed differ between compilers.
#if !defined _MSC_VER && !defined __WATCOMC__
#ifndef __fastcall
#if __has_attribute(fastcall) && defined __i386__
#define __fastcall __attribute__((fastcall))
#else
#define __fastcall
#endif
#endif

#ifndef __cdecl
#if __has_attribute(cdecl) && defined __i386__
#define __cdecl __attribute__((cdecl))
#else
#define __cdecl
#endif
#endif

#ifndef __stdcall
#if __has_attribute(stdcall) && defined __i386__
#define __stdcall __attribute__((stdcall))
#else
#define __stdcall
#endif
#endif
#endif // _MSC_VER

// This section defines some keywords controlling inlining and unused variables
// where the keywords needed differ between compilers.
#ifdef _MSC_VER
#define __noinline __declspec(noinline)
#define __unused __pragma(warning(suppress : 4100 4101))
#define __mayalias
#define __noreturn __declspec(noreturn)
#define __nothrow __declspec(nothrow)
#define __selectany __declspec(selectany)
#define __novtable __declspec(novtable)
#else
#ifndef __forceinline
#if __has_attribute(__always_inline__)
#define __forceinline inline __attribute__((__always_inline__))
#else
#define __forceinline inline
#endif
#endif

#ifndef __unused
#if __has_attribute(unused)
#define __unused __attribute__((unused))
#else
#define __unused
#endif
#endif

#ifndef __noinline
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

#ifndef __noreturn
#if __has_attribute(__noreturn__)
#define __noreturn __attribute__((__noreturn__))
#else
#define __noreturn
#endif
#endif

#ifndef __nothrow
#if __has_attribute(__nothrow__)
#define __nothrow __attribute__((__nothrow__))
#else
#define __nothrow
#endif
#endif

#ifndef __selectany
#if __has_attribute(__selectany__)
#define __selectany __attribute__((__selectany__))
#else
#define __selectany
#endif
#endif

#ifndef __novtable
#if __has_attribute(__novtable__)
#define __novtable __attribute__((__novtable__))
#else
#define __novtable
#endif
#endif
#endif // COMPILER_MSVC

/**
 * Returns the count of items in a built-in C array. This is a common technique
 * which is often used to help properly calculate the number of items in an
 * array at runtime in order to prevent overruns, etc.
 *
 * Example usage :
 *     int array[95];
 *     size_t arrayCount = ARRAY_SIZE(array);     // arrayCount is 95.
 */
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif // !ARRAY_SIZE

/**
 * Use to determine the size of a public struct member:
 *
 * Example usage:
 *  typedef struct _ABC {
 *      sint32    A;
 *      sint32    B;
 *      sint16    C;
 *  } ABC, * PTR_ABC
 *
 *  SIZE_OF(struct _ABC, C)
 */
#ifndef SIZE_OF
#define SIZE_OF(typ, id) sizeof(((typ *)0)->id)
#endif // !SIZE_OF

/**
 * Defines operator overloads to enable bit operations on enum values, useful for
 * using an enum to define flags for a bitfield.
 *
 * Example usage:
 *  enum MyEnum {
 *      ENUM_A = 0,
 *      ENUM_B = 1,
 *      ENUM_C = 2,
 *  };
 *
 *  DEFINE_ENUMERATION_BITWISE_OPERATORS(MyEnum);
 */
#ifdef __cplusplus
#if !defined(DEFINE_ENUMERATION_BITWISE_OPERATORS)
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    __forceinline constexpr ENUMTYPE operator|(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) | ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator&(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) & ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator~(ENUMTYPE const a) { return ENUMTYPE(~((int)a)); } \
    __forceinline constexpr ENUMTYPE operator^(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return ENUMTYPE(((int)a) ^ ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) ^= ((int &)b)); } \
    __forceinline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) &= ((int &)b)); } \
    __forceinline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((int &)a) |= ((int &)b)); } \
    __forceinline constexpr ENUMTYPE operator<<(ENUMTYPE a, int const b) { return (ENUMTYPE)(((int)a) << ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator>>(ENUMTYPE a, int const b) { return (ENUMTYPE)(((int)a) >> ((int)b)); } \
    __forceinline ENUMTYPE &operator<<=(ENUMTYPE &a, int const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) << ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator>>=(ENUMTYPE &a, int const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) >> ((int)b)); \
    } \
    }
#endif // !DEFINE_ENUMERATION_BITWISE_OPERATORS
#else
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

/**
 * Defines operator overloads to enable stadard math operations on an enum.
 * Useful when an enum represents a range that can be iterated over.
 *
 * Example usage:
 *  enum MyEnum {
 *      ENUM_A = 0,
 *      ENUM_B = 1,
 *      ENUM_C = 2,
 *  };
 *
 *  DEFINE_ENUMERATION_OPERATORS(MyEnum);
 */
#ifdef __cplusplus
#if !defined(DEFINE_ENUMERATION_OPERATORS)
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) \
    extern "C++" { \
    __forceinline ENUMTYPE operator++(ENUMTYPE const &a) { return (ENUMTYPE)(++((int &)a)); } \
    __forceinline ENUMTYPE operator--(ENUMTYPE const &a) { return (ENUMTYPE)(--((int &)a)); } \
    __forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) + ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) - ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) * ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) / ((int)b)); \
    } \
    __forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE)(((int)a) % ((int)b)); \
    } \
    __forceinline ENUMTYPE &operator+=(ENUMTYPE &a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) + ((ENUMTYPE)b)); \
    } \
    __forceinline ENUMTYPE &operator-=(ENUMTYPE &a, ENUMTYPE const b) \
    { \
        return (ENUMTYPE &)((int &)a = ((int &)a) - ((ENUMTYPE)b)); \
    } \
    __forceinline ENUMTYPE operator++(ENUMTYPE const &a, int const b) { return (ENUMTYPE)(++((int &)a)); } \
    __forceinline ENUMTYPE operator--(ENUMTYPE const &a, int const b) { return (ENUMTYPE)(--((int &)a)); } \
    __forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) + ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) - ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) * ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) / ((int)b)); } \
    __forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, int const b) { return (ENUMTYPE)(((int)a) % ((int)b)); } \
    __forceinline ENUMTYPE &operator+=(ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) + ((int)b)); } \
    __forceinline ENUMTYPE &operator-=(ENUMTYPE &a, int const b) { return (ENUMTYPE &)((int &)a = ((int &)a) - ((int)b)); } \
    }
#endif // !DEFINE_ENUMERATION_OPERATORS
#else
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

#endif // BASE_MACROS_H
