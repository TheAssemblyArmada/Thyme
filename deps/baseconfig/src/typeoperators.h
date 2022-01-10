/**
 * @file
 *
 * @author CCHyper
 * @author OmniBlade
 * @author xezon
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
#ifndef BASE_TYPEOPERATORS_H
#define BASE_TYPEOPERATORS_H

#include "typesize.h"

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
// clang-format off
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) \
extern "C++" { \
__forceinline constexpr ENUMTYPE operator|(ENUMTYPE const a, ENUMTYPE const b) { return ENUMTYPE(((SizedInteger<ENUMTYPE>::type)a) | ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator&(ENUMTYPE const a, ENUMTYPE const b) { return ENUMTYPE(((SizedInteger<ENUMTYPE>::type)a) & ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator~(ENUMTYPE const a) { return ENUMTYPE(~((SizedInteger<ENUMTYPE>::type)a)); } \
__forceinline constexpr ENUMTYPE operator^(ENUMTYPE const a, ENUMTYPE const b) { return ENUMTYPE(((SizedInteger<ENUMTYPE>::type)a) ^ ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator^=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((SizedInteger<ENUMTYPE>::type &)a) ^= ((SizedInteger<ENUMTYPE>::type &)b)); } \
__forceinline ENUMTYPE &operator&=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((SizedInteger<ENUMTYPE>::type &)a) &= ((SizedInteger<ENUMTYPE>::type &)b)); } \
__forceinline ENUMTYPE &operator|=(ENUMTYPE &a, ENUMTYPE const &b) { return (ENUMTYPE &)(((SizedInteger<ENUMTYPE>::type &)a) |= ((SizedInteger<ENUMTYPE>::type &)b)); } \
__forceinline constexpr ENUMTYPE operator<<(ENUMTYPE a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) << ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator>>(ENUMTYPE a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) >> ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator<<=(ENUMTYPE &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) << ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator>>=(ENUMTYPE &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) >> ((SizedInteger<ENUMTYPE>::type)b)); } \
}
// clang-format on
#endif // !DEFINE_ENUMERATION_BITWISE_OPERATORS
#else
#define DEFINE_ENUMERATION_BITWISE_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

/**
 * Defines operator overloads to enable standard math operations on an enum.
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
// clang-format off
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) \
extern "C++" { \
__forceinline ENUMTYPE operator++(ENUMTYPE const &a) { return (ENUMTYPE)(++((SizedInteger<ENUMTYPE>::type &)a)); } \
__forceinline ENUMTYPE operator--(ENUMTYPE const &a) { return (ENUMTYPE)(--((SizedInteger<ENUMTYPE>::type &)a)); } \
__forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, ENUMTYPE const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) + ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, ENUMTYPE const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) - ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, ENUMTYPE const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) * ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, ENUMTYPE const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) / ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, ENUMTYPE const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) % ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator+=(ENUMTYPE &a, ENUMTYPE const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) + ((ENUMTYPE)b)); } \
__forceinline ENUMTYPE &operator-=(ENUMTYPE &a, ENUMTYPE const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) - ((ENUMTYPE)b)); } \
__forceinline ENUMTYPE operator++(ENUMTYPE const &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type &)a)++); } \
__forceinline ENUMTYPE operator--(ENUMTYPE const &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type &)a)--); } \
__forceinline constexpr ENUMTYPE operator+(ENUMTYPE const a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) + ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator-(ENUMTYPE const a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) - ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator*(ENUMTYPE const a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) * ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator/(ENUMTYPE const a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) / ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline constexpr ENUMTYPE operator%(ENUMTYPE const a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE)(((SizedInteger<ENUMTYPE>::type)a) % ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator+=(ENUMTYPE &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) + ((SizedInteger<ENUMTYPE>::type)b)); } \
__forceinline ENUMTYPE &operator-=(ENUMTYPE &a, SizedInteger<ENUMTYPE>::type const b) { return (ENUMTYPE &)((SizedInteger<ENUMTYPE>::type &)a = ((SizedInteger<ENUMTYPE>::type &)a) - ((SizedInteger<ENUMTYPE>::type)b)); } \
}
// clang-format on
#endif // !DEFINE_ENUMERATION_OPERATORS
#else
#define DEFINE_ENUMERATION_OPERATORS(ENUMTYPE) // NOP, C allows these operators.
#endif // __cplusplus

#endif // BASE_TYPEOPERATORS_H
