/**
 * @file
 *
 * @author xezon
 *
 * @brief String utility functions. (Thyme Feature)
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
#include <string>
#include <typesize.h>
#include <unichar.h>

#if defined(THYME_USE_STLPORT)
#define CHAR_TRAITS_CONSTEXPR
#else
#define CHAR_TRAITS_CONSTEXPR constexpr
#endif

namespace rts
{

// Get a single character. For use in templates.
template<typename CharType> inline CHAR_TRAITS_CONSTEXPR CharType Get_Char(const char ch);

template<> inline CHAR_TRAITS_CONSTEXPR char Get_Char<char>(const char ch)
{
    return ch;
}

template<> inline CHAR_TRAITS_CONSTEXPR unichar_t Get_Char<unichar_t>(const char ch)
{
    const int i_ch = std::char_traits<char>::to_int_type(ch);
    const unichar_t u_ch = std::char_traits<unichar_t>::to_char_type(i_ch);
    return u_ch;
}

// Get an empty null terminated c string. For use in templates.
template<typename CharType> inline constexpr const CharType *Get_Null_Str();

template<> inline constexpr const char *Get_Null_Str<char>()
{
    return "";
}

template<> inline constexpr const unichar_t *Get_Null_Str<unichar_t>()
{
    return U_CHAR("");
}

// Ascii whitespace is any of these
// ' ' (0x20)(SPC)space
// '\t'(0x09)(TAB)horizontal tab
// '\n'(0x0a)(LF)newline
// '\v'(0x0b)(VT)vertical tab
// '\f'(0x0c)(FF)feed
// '\r'(0x0d)(CR)carriage return
template<typename CharType> CHAR_TRAITS_CONSTEXPR bool Is_Ascii_Whitespace(CharType ch)
{
    return ch == Get_Char<CharType>(' ') || ch == Get_Char<CharType>('\t') || ch == Get_Char<CharType>('\n')
        || ch == Get_Char<CharType>('\v') || ch == Get_Char<CharType>('\f') || ch == Get_Char<CharType>('\r');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool Is_Null(CharType ch)
{
    return ch == Get_Char<CharType>('\0');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool Is_Space(CharType ch)
{
    return ch == Get_Char<CharType>(' ');
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR bool Is_Null_Or_Ascii_Whitespace(CharType ch)
{
    return Is_Null(ch) || Is_Ascii_Whitespace(ch);
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR std::size_t String_Length(const CharType *src)
{
    const CharType null_char = Get_Char<CharType>('\0');
    const CharType *end = src;
    while (*end != null_char) {
        ++end;
    }
    return (end - src);
}

template<typename CharType> CHAR_TRAITS_CONSTEXPR int String_Compare(const CharType *s1, const CharType *s2)
{
    using UnsignedType = typename UnsignedIntegerForSize<sizeof(CharType)>::type;
    const CharType null_char = Get_Char<CharType>('\0');

    while (*s1 != null_char && *s1 == *s2) {
        ++s1;
        ++s2;
    }
    const auto i1 = *reinterpret_cast<const UnsignedType *>(s1);
    const auto i2 = *reinterpret_cast<const UnsignedType *>(s2);
    return (i1 > i2) - (i2 > i1);
}

template<typename CharType>
CHAR_TRAITS_CONSTEXPR int String_N_Compare(const CharType *s1, const CharType *s2, std::size_t count)
{
    using UnsignedType = typename UnsignedIntegerForSize<sizeof(CharType)>::type;
    const CharType null_char = Get_Char<CharType>('\0');

    while (count != 0 && *s1 != null_char && *s1 == *s2) {
        ++s1;
        ++s2;
        --count;
    }
    if (count == 0) {
        return 0;
    } else {
        const auto i1 = *reinterpret_cast<const UnsignedType *>(s1);
        const auto i2 = *reinterpret_cast<const UnsignedType *>(s2);
        return (i1 > i2) - (i2 > i1);
    }
}

// Strips leading and trailing spaces. Returns length of new string after strip. Writes null over all stripped characters at
// the end. Compatible with UTF-8 and UTF-16.
template<typename CharType> std::size_t Strip_Leading_And_Trailing_Spaces(CharType *dest)
{
    const CharType null_char = Get_Char<CharType>('\0');
    const CharType *reader = dest;
    const CharType *reader_end = dest + String_Length(dest);
    CharType *writer = dest;
    const CharType *writer_end = reader_end;

    for (; reader != reader_end && Is_Space(*reader); ++reader) {
    }

    for (; reader != reader_end && Is_Space(*(reader_end - 1)); --reader_end) {
    }

    while (reader != reader_end) {
        *writer++ = *reader++;
    }

    const std::size_t len = writer - dest;

    while (writer != writer_end) {
        *writer++ = null_char;
    }

    return len;
}

// Strips leading, trailing and duplicate spaces. Preserves other whitespace characters such as LF and strips surrounding
// spaces. Returns length of new string after strip. Writes null over all stripped characters at the end. Compatible with
// UTF-8 and UTF-16.
template<typename CharType> std::size_t Strip_Obsolete_Spaces(CharType *dest)
{
    const CharType null_char = Get_Char<CharType>('\0');
    CharType prev_char = Get_Char<CharType>(' ');
    const CharType *reader = dest;
    CharType *writer = dest;

    for (; !Is_Null(*reader) && Is_Space(*reader); ++reader) {
    }

    while (!Is_Null(*reader)) {
        CharType curr_char = *reader;
        CharType next_char = *++reader;

        if (Is_Space(curr_char) && (Is_Null_Or_Ascii_Whitespace(next_char) || Is_Ascii_Whitespace(prev_char))) {
            continue;
        }

        *writer++ = curr_char;
        prev_char = curr_char;
    }

    const std::size_t len = (writer - dest);

    while (writer != reader) {
        *writer++ = null_char;
    }

    return len;
}

// Replaces any search character with replacement character. Compatible with UTF-16. Compatible with UTF-8 if search and
// replace are ASCII characters.
template<typename CharType> void Replace_Characters(CharType *dest, const CharType *search, CharType replace)
{
    CharType *writer = dest;

    for (; !Is_Null(*writer); ++writer) {
        const CharType *searcher = search;

        for (; !Is_Null(*searcher); ++searcher) {
            if (*writer == *searcher) {
                *writer = replace;
                break;
            }
        }
    }
}

// Replaces search character sequence with replacement character sequence. Returns count of characters copied to destination
// string, not including null terminator. Compatible with UTF-16. Compatible with UTF-8 if search and replace are ASCII
// characters.
template<typename CharType>
std::size_t Replace_Character_Sequence(
    CharType *dest, std::size_t size, const CharType *src, const CharType *search, const CharType *replace)
{
    const CharType null_char = Get_Char<CharType>('\0');
    const std::size_t search_len = String_Length(search);
    const std::size_t replace_len = String_Length(replace);
    const CharType *reader = src;
    const CharType *writer_end = dest + size - 1;
    CharType *writer = dest;
    std::size_t replace_count = 0;

    while (*reader != null_char && writer != writer_end) {
        if (replace_count > 0) {
            *writer++ = *(replace + replace_len - replace_count);
            if (--replace_count == 0) {
                reader += search_len;
            }
        } else if (String_N_Compare(reader, search, search_len) == 0) {
            replace_count = replace_len;
        } else {
            *writer++ = *reader++;
        }
    }

    *writer = null_char;

    return (writer - dest);
}

// Replaces search character sequence(s) with replacement character sequence(s). Returns count of characters copied to
// destination string, not including null terminator. Compatible with UTF-16. Compatible with UTF-8 if search and replace are
// ASCII characters.
template<typename CharType, std::size_t Count>
std::size_t Replace_Character_Sequences(CharType *dest,
    std::size_t size,
    const CharType *src,
    const CharType *(&search)[Count],
    const CharType *(&replace)[Count])
{
    const CharType null_char = Get_Char<CharType>('\0');
    std::size_t i;
    std::size_t search_len[Count];
    std::size_t replace_len[Count];
    for (i = 0; i < Count; ++i) {
        search_len[i] = String_Length(search[i]);
        replace_len[i] = String_Length(replace[i]);
    }
    const CharType *reader = src;
    const CharType *writer_end = dest + size - 1;
    CharType *writer = dest;
    std::size_t replace_count = 0;

    while (*reader != null_char && writer != writer_end) {
        if (replace_count > 0) {
            *writer++ = *(replace[i] + replace_len[i] - replace_count);
            if (--replace_count == 0) {
                reader += search_len[i];
            }
        } else {
            for (i = 0; i < Count; ++i) {
                if (String_N_Compare(reader, search[i], search_len[i]) == 0) {
                    replace_count = replace_len[i];
                    break;
                }
            }
            if (replace_count == 0) {
                *writer++ = *reader++;
            }
        }
    }

    *writer = null_char;

    return (writer - dest);
}

template<typename CharType> bool Is_Search_Character(CharType ch, const CharType *search)
{
    for (; !Is_Null(*search); ++search) {
        if (ch == *search) {
            return true;
        }
    }
    return false;
}

// Strips string characters by given search characters. Compatible with UTF-16. Compatible with UTF-8 if search and replace
// are ASCII characters.
template<typename CharType> void Strip_Characters(CharType *dest, const CharType *search)
{
    const CharType null_char = Get_Char<CharType>('\0');
    const CharType *reader = dest;
    CharType *writer = dest;

    for (; !Is_Null(*reader); ++reader) {
        if (Is_Search_Character(*reader, search)) {
            continue;
        }

        *writer++ = *reader;
    }

    while (writer != reader) {
        *writer++ = null_char;
    }
}

// Return the file extension of a given string.
template<typename StringType> const typename StringType::value_type *Get_File_Extension(const StringType &filename)
{
    using CharType = typename StringType::value_type;

    const char *begin = filename.begin();
    const char *end = filename.end() - 1;

    CharType ext_char = Get_Char<CharType>('.');
    CharType dir1_char = Get_Char<CharType>(':');
    CharType dir2_char = Get_Char<CharType>('/');
    CharType dir3_char = Get_Char<CharType>('\\');

    while (end != begin) {
        CharType curr_char = *end;
        if (curr_char == ext_char) {
            return end + 1;
        }
        if (curr_char == dir1_char || curr_char == dir2_char || curr_char == dir3_char) {
            return Get_Null_Str<CharType>();
        }
        --end;
    }
    return Get_Null_Str<CharType>();
}

} // namespace rts
