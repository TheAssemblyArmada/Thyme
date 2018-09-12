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
#include "critsection.h"
#include "gamedebug.h"
#include "stringex.h"
#include <stdio.h>

#if !defined THYME_USE_ICU && defined PLATFORM_WINDOWS
#include <wctype.h>
#endif

#ifndef THYME_STANDALONE
SimpleCriticalSectionClass *&g_unicodeStringCriticalSection = Make_Global<SimpleCriticalSectionClass *>(0x00A2A294);
#else
SimpleCriticalSectionClass *g_unicodeStringCriticalSection = nullptr;
#endif

Utf16String const Utf16String::EmptyString;

Utf16String::Utf16String(const unichar_t *s) : m_data(nullptr)
{
    if (s != nullptr) {
        size_t len = u_strlen(s);

        if (len > 0) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s, nullptr);
        }
    }
}

Utf16String::Utf16String(Utf16String const &string) : m_data(string.m_data)
{
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

unichar_t *Utf16String::Peek() const
{
    ASSERT_PRINT(m_data != nullptr, "null string ptr");

    // Actual string data is stored immediately after the UnicodeStringData header.
    return m_data->Peek();
}

void Utf16String::Release_Buffer()
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if (m_data != nullptr) {
        if (--m_data->ref_count == 0) {
            g_dynamicMemoryAllocator->Free_Bytes(m_data);
        }

        m_data = nullptr;
    }
}

void Utf16String::Ensure_Unique_Buffer_Of_Size(
    int chars_needed, bool keep_data, const unichar_t *str_to_cpy, const unichar_t *str_to_cat)
{
    if (m_data != nullptr && m_data->ref_count == 1 && m_data->num_chars_allocated >= chars_needed) {
        if (str_to_cpy != nullptr) {
            u_strcpy(Peek(), str_to_cpy);
        }

        if (str_to_cat != nullptr) {
            u_strcat(Peek(), str_to_cat);
        }

    } else {
        // this block would have been a macro like DEBUG_CRASH(numCharsNeeded + 8 > MAX_LEN, THROW_02);
        // if ( numCharsNeeded + 8 > MAX_LEN ) {
        // *&preserveData = 0xDEAD0002;
        // throw(&preserveData, &_TI1_AW4ErrorCode__);
        //}

        int size = g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(
            sizeof(unichar_t) * chars_needed + sizeof(UnicodeStringData));
        UnicodeStringData *new_data =
            reinterpret_cast<UnicodeStringData *>(g_dynamicMemoryAllocator->Allocate_Bytes_No_Zero(size));

        new_data->ref_count = 1;
        new_data->num_chars_allocated = (size - sizeof(UnicodeStringData)) / sizeof(unichar_t);
#ifdef GAME_DEBUG_STRUCTS
        new_data->debug_ptr = new_data->Peek();
#endif

        if (m_data != nullptr && keep_data) {
            u_strcpy(new_data->Peek(), Peek());
        } else {
            *new_data->Peek() = (unichar_t)u'\0';
        }

        if (str_to_cpy != nullptr) {
            u_strcpy(new_data->Peek(), str_to_cpy);
        }

        if (str_to_cat != nullptr) {
            u_strcat(new_data->Peek(), str_to_cat);
        }

        Release_Buffer();
        m_data = new_data;
    }
}

int Utf16String::Get_Length() const
{
    if (m_data != nullptr) {
        return u_strlen(m_data->Peek());
    }

    return 0;
}

void Utf16String::Clear()
{
    Release_Buffer();
}

unichar_t Utf16String::Get_Char(int index) const
{
    if (m_data != nullptr) {
        return m_data->Peek()[index];
    }

    return u'\0';
}

const unichar_t *Utf16String::Str() const
{
    static const unichar_t *TheNullChr = (const unichar_t *)u"";

    if (m_data != nullptr) {
        return Peek();
    }

    return TheNullChr;
}

unichar_t *Utf16String::Get_Buffer_For_Read(int len)
{
    ASSERT_PRINT(len > 0, "No need to allocate 0 len strings.");

    Ensure_Unique_Buffer_Of_Size(len + 1, false, nullptr, nullptr);

    return Peek();
}

void Utf16String::Set(const unichar_t *s)
{
    if (m_data != nullptr || s != m_data->Peek()) {
        size_t len;

        if (s && (len = u_strlen(s) + 1, len != 1)) {
            Ensure_Unique_Buffer_Of_Size(len, false, s, nullptr);
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

void Utf16String::Translate(Utf8String const &string)
{
    Release_Buffer();

    int str_len = string.Get_Length();

    for (int i = 0; i < str_len; ++i) {
        unichar_t c;

        if (string.m_data != nullptr) {
            c = string.Get_Char(i);
        } else {
            c = (unichar_t)u'\0';
        }

        Concat(c);
    }
}

void Utf16String::Translate(const char *string)
{
    Release_Buffer();

#if defined THYME_USE_ICU // Use ICU convertors
    int32_t length;
    UErrorCode error = U_ZERO_ERROR;
    u_strFromUTF8(nullptr, 0, &length, string, -1, &error);

    if (U_SUCCESS(error) && length > 0) {
        u_strFromUTF8(Get_Buffer_For_Read(length), length, nullptr, string, -1, &error);

        if (U_FAILURE(error)) {
            Clear();
        }
    }
#elif defined PLATFORM_WINDOWS // Use WIN32 API convertors.
    int length = MultiByteToWideChar(CP_UTF8, 0, string, -1, nullptr, 0);

    if (length > 0) {
        MultiByteToWideChar(CP_UTF8, 0, string, -1, Get_Buffer_For_Read(length), length);
    }
#else // Naive copy, this is what the original does.
    int str_len = strlen(string);

    for (int i = 0; i < str_len; ++i) {
        unichar_t c;

        if (string[i] != '\0') {
            c = string[i];
        } else {
            c = (unichar_t)u'\0';
        }

        Concat(c);
    }
#endif
}

void Utf16String::Concat(unichar_t c)
{
    unichar_t str[2];

    str[0] = c;
    str[1] = (unichar_t)u'\0';
    Concat(str);
}

void Utf16String::Concat(const unichar_t *s)
{
    size_t len = u_strlen(s);

    if (len > 0) {
        if (m_data != nullptr) {
            Ensure_Unique_Buffer_Of_Size(u_strlen(Peek()) + len + 1, true, 0, s);
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
    for (char i = *str; i != '\0'; i = *(++str)) {
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

    for (int i = u_strlen(Peek()) - 1; i >= 0; --i) {
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

    for (unichar_t *c = buf; *c != (unichar_t)u'\0'; ++c) {
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

    int len = u_strlen(Peek());

    if (len > 0) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len] = (unichar_t)u'\0';
    }
}

void Utf16String::Format(const unichar_t *format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void Utf16String::Format(Utf16String format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void Utf16String::Format_VA(const unichar_t *format, va_list args)
{
    unichar_t buf[MAX_FORMAT_BUF_LEN];

    ASSERT_THROW_PRINT(u_vsnprintf_u(buf, sizeof(buf), format, args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

void Utf16String::Format_VA(Utf16String &format, va_list args)
{
    unichar_t buf[MAX_FORMAT_BUF_LEN];

    ASSERT_THROW_PRINT(u_vsnprintf_u(buf, sizeof(buf), format.Str(), args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

bool Utf16String::Next_Token(Utf16String *tok, Utf16String delims)
{
    if (m_data == nullptr) {
        return false;
    }

    if (*Peek() == (unichar_t)u'\0' || this == tok) {
        return false;
    }

    // If no separators provided, default to white space.
    if (delims == nullptr) {
        delims = (const unichar_t *)u" \n\r\t";
    }

#if THYME_USE_ICU
    unichar_t *start = Peek();

    // Find next instance of token or end of string
    for (unichar_t c = *start; c != (unichar_t)u'\0'; c = *(++start)) {
        if (u_strchr(delims, c) == nullptr) {
            break;
        }
    }

    if (*start == (unichar_t)u'\0') {
        Release_Buffer();
        tok->Release_Buffer();

        return false;
    }

    unichar_t *end = start;

    // Find next instance of token or end of string.
    for (unichar_t c = *end; c != (unichar_t)u'\0'; c = *(++end)) {
        if (u_strchr(delims, c) != nullptr) {
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
    tokstr[end - start] = (unichar_t)u'\0';
    Set(end);

    return true;
#else
    const unichar_t *start = Str();

    size_t pos = wcscspn(Peek(), delims.Str());

    // Check if the position of the next token is not the start of data anyway.
    if (&(Peek()[pos]) > Peek()) {
        unichar_t *read_buffer = tok->Get_Buffer_For_Read(pos + 1);
        memcpy(read_buffer, Peek(), pos);
        read_buffer[pos] = (unichar_t)u'\0';
        Set(&(Peek()[pos]));

        return true;
    }

    return false;
#endif
}
