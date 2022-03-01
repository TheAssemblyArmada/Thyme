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

template<
    typename ExpectedType,
    typename ElemType,
    size_t Size,
    typename Pred>
constexpr ExpectedType ReturnWithListCheck(ElemType (&list)[Size], Pred pred)
{
#ifndef THYME_USE_STLPORT
    static_assert(std::is_convertible<decltype(list), ExpectedType>::value,
        "Wrong type passed to parse function!");
    static_assert(Size > 0,
        "List with invalid size passed to parse function!");
    static_assert(pred()[Size - 1] == nullptr,
        "List without null terminator passed to parse function!");
#endif
    return static_cast<ExpectedType>(list);
}

} // namespace Thyme

#define FIELD_PARSE_LAST \
        FieldParse { \
            nullptr, \
            nullptr, \
            nullptr, \
            0 \
        }
#define FIELD_PARSE_BOOL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Bool, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), bool>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_UNSIGNED_BYTE(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Unsigned_Byte, \
            nullptr, \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), uint8_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_INT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Int, \
            nullptr, \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), int32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ASCIISTRING(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_AsciiString, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Utf8String>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_INDEX_LIST(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Index_List, \
            Thyme::ReturnWithListCheck<const char *const *>(user_data, []() constexpr { return user_data; }), \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), int32_t>(offsetof(classtype, classmember)) \
        }
// clang-format on
