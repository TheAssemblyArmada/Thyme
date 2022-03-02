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

template<typename ExpectedType, typename ValueType>
constexpr const void *IntToUserdata(ValueType value)
{
    static_assert(sizeof(void *) == sizeof(uintptr_t));
    return reinterpret_cast<const void *>(static_cast<uintptr_t>(
        ReturnWithSameCheck<ValueType, ExpectedType>(value)));
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
#define FIELD_PARSE_UNSIGNED_SHORT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Unsigned_Short, \
            nullptr, \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), uint16_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_UNSIGNED_INT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Unsigned_Int, \
            nullptr, \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), uint32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_POSITIVE_NON_ZERO_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Positive_Non_Zero_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_PERCENT_TO_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Percent_To_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ANGLE_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Angle_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ANGULAR_VELOCITY_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Angular_Velocity_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ASCIISTRING(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_AsciiString, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Utf8String>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_QUOTED_ASCIISTRING(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Quoted_AsciiString, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Utf8String>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ASCIISTRING_VECTOR(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_AsciiString_Vector, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), std::vector<Utf8String>>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ASCIISTRING_VECTOR_APPEND(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_AsciiString_Vector_Append, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), std::vector<Utf8String>>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_RGB_COLOR(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_RGB_Color, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), RGBColor>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_RGBA_COLOR_INT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_RGBA_Color_Int, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), RGBAColorInt>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_COLOR_INT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Color_Int, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), int32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_COORD2D(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Coord2D, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Coord2D>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_COORD3D(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Coord3D, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Coord3D>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ICOORD2D(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_ICoord2D, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), ICoord2D>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ICOORD3D(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_ICoord3D, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), ICoord3D>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_INDEX_LIST(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Index_List, \
            Thyme::ReturnWithListCheck<const char *const *>(user_data, []() constexpr { return user_data; }), \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), int32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_BYTE_SIZED_INDEX_LIST(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Byte_Sized_Index_List, \
            Thyme::ReturnWithListCheck<const char *const *>(user_data, []() constexpr { return user_data; }), \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), unsigned char>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_DURATION_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Duration_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_DURATION_UNSIGNED_INT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Duration_Unsigned_Int, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), uint32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_DURATION_UNSIGNED_SHORT(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Duration_Unsigned_Short, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), uint16_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_VELOCITY_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Velocity_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_ACCELERATION_REAL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Acceleration_Real, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), float>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_BIT_IN_INT32(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Bit_In_Int32, \
            Thyme::IntToUserdata<uint32_t>(user_data) \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), int32_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_AND_TRANSLATE_LABEL(token, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_And_Translate_Label, \
            nullptr, \
            Thyme::ReturnWithSameCheck<decltype(classtype::classmember), Utf16String>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_BITSTRING8(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Bitstring8, \
            Thyme::ReturnWithListCheck<const char *const *>(user_data, []() constexpr { return user_data; }), \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), uint8_t>(offsetof(classtype, classmember)) \
        }
#define FIELD_PARSE_BITSTRING32(token, user_data, classtype, classmember) \
        FieldParse { \
            token, \
            &INI::Parse_Bitstring32, \
            Thyme::ReturnWithListCheck<const char *const *>(user_data, []() constexpr { return user_data; }), \
            Thyme::ReturnWithEquivalentCheck<decltype(classtype::classmember), uint32_t>(offsetof(classtype, classmember)) \
        }
// clang-format on