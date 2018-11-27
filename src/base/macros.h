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

// These allow evaluation of compiler specific attributes and intrinics on GCC like compilers.
// If they don't exist we want them to evaluate to false.
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

/**
 * Returns the count of items in a built-in C array. This is a common technique
 * which is often used to help properly calculate the number of items in an
 * array at runtime in order to prevent overruns, etc.
 *
 * Example usage :
 *     int array[95];
 *     size_t arrayCount = ARRAY_SIZE(array);     // arrayCount is 95.
 */
#if !defined(ARRAY_SIZE)
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
#if !defined(SIZE_OF)
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
#if defined(__cplusplus)
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
#if defined(__cplusplus)
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
