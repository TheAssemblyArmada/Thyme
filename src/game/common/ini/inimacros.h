/**
 * @file
 *
 * @author xezon
 *
 * @brief Helper macros for INI Parse Tables
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef THYME_USE_STLPORT
#include <type_traits>
#endif

// clang-format off
namespace Thyme
{

template<typename CheckedType, typename ExpectedType, typename ValueType>
constexpr ValueType ReturnWithSameCheck(ValueType value)
{
#ifndef THYME_USE_STLPORT
    using CheckedTypeNoRef = std::remove_reference_t<CheckedType>;
    static_assert(std::is_same<CheckedTypeNoRef, ExpectedType>::value,
        "Wrong type passed to parse function!");
#endif
    return value;
}

template<typename CheckedType, typename ExpectedType, typename ValueType>
constexpr ValueType ReturnWithEquivalentCheck(ValueType value)
{
#ifndef THYME_USE_STLPORT
    if constexpr (std::is_enum<CheckedType>::value) {
        using CheckedTypeNoRef = std::remove_reference_t<CheckedType>;
        using CheckedTypeNoRefUnderlying = std::underlying_type_t<CheckedTypeNoRef>;
        static_assert(std::is_same<CheckedTypeNoRefUnderlying, ExpectedType>::value,
            "Wrong type passed to parse function!");
    } else {
        using CheckedTypeNoRef = std::remove_reference_t<CheckedType>;
        static_assert(std::is_same<CheckedTypeNoRef, ExpectedType>::value,
            "Wrong type passed to parse function!");
    }
#endif
    return value;
}

} // namespace Thyme

#define FIELD_PARSE_LAST \
        FieldParse { \
            nullptr, \
            nullptr, \
            nullptr, \
            0 \
        }
#define FIELD_PARSE_ASCIISTRING(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_AsciiString, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Utf8String>(offsetof(classtype, classmember)) \
        }
// clang-format on
