/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief String class handing "wide" chars.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "unicodestring.h"
#include "asciistring.h"
#include "errorcodes.h"
#include "memdynalloc.h"
#include <captainslog.h>
#include <stdio.h>
#if defined PLATFORM_WINDOWS
#include <utf.h>
#endif

#if !defined BUILD_WITH_ICU && defined PLATFORM_WINDOWS
#include <wctype.h>
#endif

#ifndef GAME_DLL
SimpleCriticalSectionClass *g_unicodeStringCriticalSection = nullptr;
#endif

Utf16String const Utf16String::s_emptyString;

Utf16String::Utf16String(const unichar_t *s) : m_data(nullptr)
{
    if (s != nullptr) {
        const size_type len = static_cast<size_type>(u_strlen(s));

        if (len > 0) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s, nullptr);
        }
    }
}

Utf16String::Utf16String(Utf16String const &string) : m_data(string.m_data)
{
    // #TODO Remove critical section and put atomic reference counter when standalone.

    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if (m_data != nullptr) {
        ++m_data->ref_count;
    }
}

Utf16String::~Utf16String()
{
    Release_Buffer();
}

void Utf16String::Validate() {}

const unichar_t *Utf16String::Peek() const
{
    captainslog_dbgassert(m_data != nullptr, "null string ptr");

    return m_data->Peek();
}

unichar_t *Utf16String::Peek()
{
    captainslog_dbgassert(m_data != nullptr, "null string ptr");

    return m_data->Peek();
}

void Utf16String::Release_Buffer()
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    Validate();

    if (m_data != nullptr) {
        if (--m_data->ref_count == 0) {
            g_dynamicMemoryAllocator->Free_Bytes(m_data);
        }

        m_data = nullptr;
    }
}

void Utf16String::Ensure_Unique_Buffer_Of_Size(
    size_type chars_needed, bool keep_data, const unichar_t *str_to_cpy, const unichar_t *str_to_cat)
{
    Validate();

    if (m_data != nullptr && m_data->ref_count == 1 && m_data->num_chars_allocated >= chars_needed) {
        if (str_to_cpy != nullptr) {
            // #BUGFIX Originally uses u_strcpy here. Use memmove to support overlaps gracefully.
            captainslog_dbgassert(u_strlen(str_to_cpy) == chars_needed - 1, "Length does not match");
            memmove(Peek(), str_to_cpy, chars_needed * sizeof(value_type));
        }

        if (str_to_cat != nullptr) {
            u_strcat(Peek(), str_to_cat);
        }

    } else {
        const int required_size = sizeof(unichar_t) * chars_needed + sizeof(UnicodeStringData);

        captainslog_relassert(required_size <= MAX_LEN, CODE_02, "Size exceeds max len");

        const int alloc_size = g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(required_size);
        UnicodeStringData *new_data =
            reinterpret_cast<UnicodeStringData *>(g_dynamicMemoryAllocator->Allocate_Bytes_No_Zero(alloc_size));

        new_data->ref_count = 1;
        new_data->num_chars_allocated = (alloc_size - sizeof(UnicodeStringData)) / sizeof(unichar_t);
#ifdef GAME_DEBUG_STRUCTS
        new_data->debug_ptr = new_data->Peek();
#endif

        if (m_data != nullptr && keep_data) {
            u_strcpy(new_data->Peek(), Peek());
        } else {
            *new_data->Peek() = U_CHAR('\0');
        }

        if (str_to_cpy != nullptr) {
            u_strcpy(new_data->Peek(), str_to_cpy);
        }

        if (str_to_cat != nullptr) {
            u_strcat(new_data->Peek(), str_to_cat);
        }

        Release_Buffer();
        m_data = new_data;
        Validate();
    }
}

Utf16String::size_type Utf16String::Get_Length() const
{
    if (m_data != nullptr) {
        return static_cast<size_type>(u_strlen(m_data->Peek()));
    }

    return 0;
}

void Utf16String::Clear()
{
    Release_Buffer();
}

unichar_t Utf16String::Get_Char(size_type index) const
{
    captainslog_dbgassert(index >= 0, "Index must be equal or larger than 0.");
    captainslog_dbgassert(index < Get_Length(), "Index must be smaller than length.");

    if (m_data != nullptr) { // #TODO Remove condition if possible; Utf8String does not have it
        return m_data->Peek()[index];
    }

    return U_CHAR('\0');
}

const unichar_t *Utf16String::Str() const
{
    static const unichar_t *TheNullChr = U_CHAR("");

    if (m_data != nullptr) {
        return Peek();
    }

    return TheNullChr;
}

unichar_t *Utf16String::Get_Buffer_For_Read(size_type len)
{
    captainslog_dbgassert(len > 0, "No need to allocate 0 len strings.");

    Ensure_Unique_Buffer_Of_Size(len + 1, false, nullptr, nullptr);

    return Peek();
}

void Utf16String::Set(const unichar_t *str)
{
    if (m_data != nullptr || str != m_data->Peek()) {
        const size_type len = str ? static_cast<size_type>(u_strlen(str)) : 0;

        if (len != 0) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, str, nullptr);
        } else {
            Release_Buffer();
        }
    }
}

void Utf16String::Set(Utf16String const &string)
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if (&string != this) {
        Release_Buffer();
        m_data = string.m_data;

        if (string.m_data != nullptr) {
            ++m_data->ref_count;
        }
    }
}

/**
 * Converts a Utf8 string to Utf16
 */
void Utf16String::Translate_Internal(const char *utf8_string, const size_type utf8_len)
{
    Release_Buffer();

#if defined BUILD_WITH_ICU
    // Use ICU converters.
    if (utf8_len > 0) {
        int32_t utf16_len;
        UErrorCode error = U_ZERO_ERROR;
        // Get utf16 string length.
        u_strFromUTF8(nullptr, 0, &utf16_len, utf8_string, utf8_len, &error);

        if (U_SUCCESS(error) && utf16_len > 0) {
            // Allocate and fill new utf16 string.
            unichar_t *utf16_buffer = Get_Buffer_For_Read(utf16_len);
            u_strFromUTF8(utf16_buffer, utf16_len, nullptr, utf8_string, utf8_len, &error);

            if (U_FAILURE(error)) {
                Clear();
            } else {
                // Add null terminator manually.
                utf16_buffer[utf16_len] = U_CHAR('\0');
            }
        }
    }
#elif defined PLATFORM_WINDOWS
    // Use WIN32 API converters.
    if (utf8_len > 0) {
        // Get utf16 string length.
        const size_type utf16_len = MultiByteToWideChar(CP_UTF8, 0, utf8_string, utf8_len, nullptr, 0);

        if (utf16_len > 0) {
            // Allocate and fill new utf16 string.
            unichar_t *utf16_buffer = Get_Buffer_For_Read(utf16_len);
            MultiByteToWideChar(CP_UTF8, 0, utf8_string, utf8_len, utf16_buffer, utf16_len);

            // Add null terminator manually.
            utf16_buffer[utf16_len] = U_CHAR('\0');
        }
    }
#else
    // Naive copy, this is what the original does.
    for (size_type i = 0; i < utf8_len; ++i) {
        unichar_t c = static_cast<unichar_t>(utf8_string[i]);
        Concat(c);
    }
#endif
}

void Utf16String::Translate(Utf8String const &utf8_string)
{
    Translate_Internal(utf8_string.Str(), utf8_string.Get_Length());
}

void Utf16String::Translate(const char *utf8_string)
{
    const size_type utf8_len = static_cast<size_type>(strlen(utf8_string));
    Translate_Internal(utf8_string, utf8_len);
}

void Utf16String::Concat(unichar_t c)
{
    unichar_t str[2];

    str[0] = c;
    str[1] = U_CHAR('\0');
    Concat(str);
}

void Utf16String::Concat(const unichar_t *s)
{
    const size_type add_len = static_cast<size_type>(u_strlen(s));

    if (add_len > 0) {
        if (m_data != nullptr) {
            const size_type cur_len = static_cast<size_type>(u_strlen(Peek()));
            Ensure_Unique_Buffer_Of_Size(cur_len + add_len + 1, true, nullptr, s);
        } else {
            Set(s);
        }
    }
}

void Utf16String::Trim()
{
    // No string, no Trim.
    if (m_data == nullptr) {
        return;
    }

    unichar_t *str = Peek();

    // Find first none space in string if not the first.
    for (unichar_t i = *str; i != U_CHAR('\0'); i = *(++str)) {
        if (!u_isspace(i)) {
            break;
        }
    }

    // If we had some spaces and moved Str, Set string to that position.
    if (str != Peek()) {
        Set(str);
    }

    // Oops, Set call broke the string.
    if (m_data == nullptr) {
        return;
    }

    const size_type len = static_cast<size_type>(u_strlen(Peek()));

    for (size_type i = len - 1; i >= 0; --i) {
        if (!u_isspace(Get_Char(i))) {
            break;
        }

        Remove_Last_Char();
    }
}

void Utf16String::To_Lower()
{
    unichar_t buf[MAX_FORMAT_BUF_LEN];

    if (m_data == nullptr) {
        return;
    }

    u_strcpy(buf, Peek());

    for (unichar_t *c = buf; *c != U_CHAR('\0'); ++c) {
        //*c = towlower(*c);
        *c = u_tolower(*c);
    }

    Set(buf);
}

void Utf16String::Remove_Last_Char()
{
    if (m_data == nullptr) {
        return;
    }

    const size_type len = static_cast<size_type>(u_strlen(Peek()));

    if (len > 0) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len - 1] = U_CHAR('\0');
    }
}

void Utf16String::Format(const unichar_t *format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
    va_end(va);
}

void Utf16String::Format(Utf16String format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
    va_end(va);
}

void Utf16String::Format_VA(const unichar_t *format, va_list args)
{
    unichar_t buf[MAX_FORMAT_BUF_LEN];
    const size_type res = u_vsnprintf_u(buf, ARRAY_SIZE(buf), format, args);
    captainslog_relassert(res > 0, 0xDEAD0002, "Unable to format buffer.");

    Set(buf);
}

void Utf16String::Format_VA(Utf16String &format, va_list args)
{
    unichar_t buf[MAX_FORMAT_BUF_LEN];
    const size_type res = u_vsnprintf_u(buf, ARRAY_SIZE(buf), format.Str(), args);
    captainslog_relassert(res > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

bool Utf16String::Next_Token(Utf16String *tok, Utf16String delims)
{
    if (m_data == nullptr) {
        return false;
    }

    if (*Peek() == U_CHAR('\0') || this == tok) {
        return false;
    }

    // If no separators provided, default to white space.
    if (delims == nullptr) {
        delims = U_CHAR(" \n\r\t");
    }

#if BUILD_WITH_ICU
    unichar_t *start = Peek();

    // Find next instance of token or end of string
    for (unichar_t c = *start; c != U_CHAR('\0'); c = *(++start)) {
        if (u_strchr(delims.Str(), c) == nullptr) {
            break;
        }
    }

    if (*start == U_CHAR('\0')) {
        Release_Buffer();
        tok->Release_Buffer();

        return false;
    }

    unichar_t *end = start;

    // Find next instance of token or end of string.
    for (unichar_t c = *end; c != U_CHAR('\0'); c = *(++end)) {
        if (u_strchr(delims.Str(), c) != nullptr) {
            break;
        }
    }

    if (end <= start) {
        Release_Buffer();
        tok->Release_Buffer();

        return false;
    }

    // Copy found region into provided Utf8String, then move this string
    // to start of next section.
    unichar_t *tokstr = tok->Get_Buffer_For_Read(end - start + 1);
    memcpy(tokstr, start, end - start);
    tokstr[end - start] = U_CHAR('\0');
    Set(end);

    return true;
#else
    const unichar_t *start = Str();

    const size_type pos = static_cast<size_type>(wcscspn(Peek(), delims.Str()));

    // Check if the position of the next token is not the start of data anyway.
    if (&(Peek()[pos]) > Peek()) {
        unichar_t *read_buffer = tok->Get_Buffer_For_Read(pos + 1);
        memcpy(read_buffer, Peek(), pos);
        read_buffer[pos] = U_CHAR('\0');
        Set(&(Peek()[pos]));

        return true;
    }

    return false;
#endif
}
