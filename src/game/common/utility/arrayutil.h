/**
 * @file
 *
 * @author xezon
 *
 * @brief Array utility functions. (Thyme Feature)
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "arrayview.h"
#include "asciistring.h"
#include "stringutil.h"
#include "unicodestring.h"
#include <captainslog.h>

namespace rts
{

template<typename ValueType> inline constexpr array_view<ValueType> Make_Array_View(ValueType *begin, std::size_t size)
{
    return array_view<ValueType>(begin, size);
}

template<typename ValueType> inline constexpr array_view<ValueType> Make_Array_View(ValueType *begin, ValueType *end)
{
    return array_view<ValueType>(begin, end);
}

inline CHAR_TRAITS_CONSTEXPR array_view<char> Make_Array_View(char *cstring)
{
    return array_view<char>(cstring, String_Length(cstring));
}

inline CHAR_TRAITS_CONSTEXPR array_view<const char> Make_Array_View(const char *cstring)
{
    return array_view<const char>(cstring, String_Length(cstring));
}

inline CHAR_TRAITS_CONSTEXPR array_view<unichar_t> Make_Array_View(unichar_t *cstring)
{
    return array_view<unichar_t>(cstring, String_Length(cstring));
}

inline CHAR_TRAITS_CONSTEXPR array_view<const unichar_t> Make_Array_View(const unichar_t *cstring)
{
    return array_view<const unichar_t>(cstring, String_Length(cstring));
}

// There is no Make_Array_View for mutable Utf8String and Utf16String. This is intentional.
// Writing to string buffer directly is not good practice, due the reference counted nature of this string.
// All modifications must take place on unique instance only. Use Make_Resized_Array_View function instead.

inline array_view<const typename Utf8String::value_type> Make_Array_View(const Utf8String &instance)
{
    return array_view<const typename Utf8String::value_type>(instance.Str(), instance.Get_Length());
}

inline array_view<const typename Utf16String::value_type> Make_Array_View(const Utf16String &instance)
{
    return array_view<const typename Utf16String::value_type>(instance.Str(), instance.Get_Length());
}

namespace detail
{
template<typename CharType, typename ObjectType, typename SizeType>
inline array_view<CharType> Utf_String_Resized_Array_View(ObjectType &object, SizeType size)
{
    using object_type = ObjectType;
    using char_type = CharType;
    using size_type = SizeType;

    captainslog_assert(size >= 0);
    if (size == 0) {
        return array_view<char_type>();
    }
    CHAR_TRAITS_CONSTEXPR char_type null_char = Get_Char<char_type>('\0');
    object_type copy = object;
    char_type *peek = object.Get_Buffer_For_Read(size); // Allocates + 1
    const char_type *reader = copy.Str();
    const char_type *end = peek + size;
    char_type *inserter = peek;
    // Copy original string as far as it goes.
    while (*reader != null_char && inserter != end) {
        *inserter++ = *reader++;
    }
    // Fill the rest with null.
    while (inserter != end + 1) {
        *inserter++ = null_char;
    }
    return array_view<char_type>(peek, size);
}
} // namespace detail

// Create array_view<> from Utf8String and resize the string with the given size.
inline array_view<typename Utf8String::value_type> Make_Resized_Array_View(
    Utf8String &utfstring, typename Utf8String::size_type size)
{
    return detail::Utf_String_Resized_Array_View<typename Utf8String::value_type>(utfstring, size);
}

// Create array_view<> from Utf16String and resize the string with the given size.
inline array_view<typename Utf16String::value_type> Make_Resized_Array_View(
    Utf16String &utfstring, typename Utf16String::size_type size)
{
    return detail::Utf_String_Resized_Array_View<typename Utf16String::value_type>(utfstring, size);
}

} // namespace rts
