/**
 * @file
 *
 * @author xezon
 *
 * @brief Type traits, equivalent to std type traits (c++11, 14) (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

namespace rts
{
template<typename T> struct remove_cv
{
    using type = T;
};
template<typename T> struct remove_cv<const T>
{
    using type = T;
};
template<typename T> struct remove_cv<volatile T>
{
    using type = T;
};
template<typename T> struct remove_cv<const volatile T>
{
    using type = T;
};
template<typename T> struct remove_const
{
    using type = T;
};
template<typename T> struct remove_const<const T>
{
    using type = T;
};
template<typename T> struct remove_volatile
{
    using type = T;
};
template<typename T> struct remove_volatile<volatile T>
{
    using type = T;
};
template<typename T> struct remove_reference
{
    using type = T;
};
template<typename T> struct remove_reference<T &>
{
    using type = T;
};
template<typename T> struct remove_reference<T &&>
{
    using type = T;
};

template<typename T> using remove_cv_t = typename remove_cv<T>::type;
template<typename T> using remove_const_t = typename remove_const<T>::type;
template<typename T> using remove_volatile_t = typename remove_volatile<T>::type;
template<typename T> using remove_reference_t = typename remove_reference<T>::type;

// std::underlying_type uses compiler intrinsics to deduce the enum type.
// rts::underlying_type cannot do that automatically and instead relies on specializated template.
// Example:
//
// namespace MyNamespace
// {
//     enum class MyEnum : unsigned int
//     {
//          MyValue1,
//          MyValue2,
//     }
// }
//
// DEFINE_RTS_UNDERLYING_TYPE(MyNamespace::MyEnum, unsigned int);

template<typename EnumType> struct underlying_type;
template<typename EnumType> using underlying_type_t = typename underlying_type<EnumType>::type;
} // namespace rts

#define DEFINE_RTS_UNDERLYING_TYPE(EnumType, UnderlyingType) \
    namespace rts \
    { \
    template<> struct underlying_type<EnumType> \
    { \
        using type = UnderlyingType; \
    }; \
    }

namespace rts
{

// clang-format off
// Fundamental types cannot be used with std::underlying_type. Apply same principle to rts::underlying_type.

template<> struct underlying_type<signed char>{};
template<> struct underlying_type<unsigned char>{};
template<> struct underlying_type<signed short>{};
template<> struct underlying_type<unsigned short>{};
template<> struct underlying_type<signed int>{};
template<> struct underlying_type<unsigned int>{};
template<> struct underlying_type<signed long>{};
template<> struct underlying_type<unsigned long>{};
template<> struct underlying_type<signed long long>{};
template<> struct underlying_type<unsigned long long>{};
template<> struct underlying_type<float>{};
template<> struct underlying_type<double>{};

// clang-format on

} // namespace rts
