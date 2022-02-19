/**
 * @file
 *
 * @author OmniBlade
 * @author Tiberian Technologies
 *
 * @brief Another string class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "always.h"
#include "critsection.h"
#include <cstdarg>
#include <cstring>
#include <cwchar>

#ifdef BUILD_WITH_ICU
#include <unicode/ustring.h>
#endif

class StringClass
{
public:
    StringClass(bool hint_temporary);
    StringClass(int32_t initial_len = 0, bool hint_temporary = false);
    StringClass(const StringClass &string, bool hint_temporary = false);
    StringClass(const char *string, bool hint_temporary = false);
    StringClass(char ch, bool hint_temporary = false);
    StringClass(const unichar_t *string, bool hint_temporary = false);
    ~StringClass();

    bool operator==(const char *rvalue) const { return Compare(rvalue) == 0; }
    bool operator!=(const char *rvalue) const { return Compare(rvalue) != 0; }
    const StringClass &operator=(const StringClass &string);
    const StringClass &operator=(const char *string);
    const StringClass &operator=(char ch);
    const StringClass &operator=(const unichar_t *string);
    const StringClass &operator+=(const StringClass &string);
    const StringClass &operator+=(const char *string);
    const StringClass &operator+=(char ch);
    friend StringClass operator+(const StringClass &string1, const StringClass &string2);
    friend StringClass operator+(const char *string1, const StringClass &string2);
    friend StringClass operator+(const StringClass &string1, const char *string2);
    bool operator<(const char *string) const { return (strcmp(m_buffer, string) < 0); }
    bool operator<=(const char *string) const { return (strcmp(m_buffer, string) <= 0); }
    bool operator>(const char *string) const { return (strcmp(m_buffer, string) > 0); }
    bool operator>=(const char *string) const { return (strcmp(m_buffer, string) >= 0); }
    const char &operator[](int32_t index) const { return m_buffer[index]; }
    char &operator[](int32_t index) { return m_buffer[index]; }
    operator const char *() const { return m_buffer; }

    int32_t Compare(const char *string) const { return strcmp(m_buffer, string); }
    int32_t Compare_No_Case(const char *string) const { return strcasecmp(m_buffer, string); }
    int32_t Get_Length() const;
    bool Is_Empty() const { return (m_buffer[0] == m_nullChar); }
    void Erase(int32_t start_index, int32_t char_count);
    int32_t Format(const char *format, ...);
    int32_t Format_Args(const char *format, va_list &arg_list);
    void Truncate_Left(unsigned truncateLength);
    void Truncate_Right(unsigned truncateLength);
    void Trim_Left();
    void Trim_Right();
    void Trim();
    char *Get_Buffer(int32_t new_length);
    char *Peek_Buffer() { return m_buffer; }
    const char *Peek_Buffer() const { return m_buffer; }
    bool Copy_Wide(const unichar_t *source);
    void Release_Resources() { Free_String(); }
    unsigned Get_Hash() const;
    void To_Lower() { strlwr(m_buffer); }

private:
    struct HEADER
    {
        int32_t allocated_length;
        int32_t length;
    };

    enum
    {
        MAX_TEMP_STRING = 8,
        MAX_TEMP_LEN = 256 - sizeof(HEADER),
        MAX_TEMP_BYTES = (MAX_TEMP_LEN * sizeof(char)) + sizeof(HEADER),
        ALL_TEMP_STRINGS_USED_MASK = 0xFF
    };

    void Get_String(size_t length, bool is_temp);
    char *Allocate_Buffer(size_t length);
    void Resize(int32_t size);
    void Uninitialised_Grow(size_t length);
    void Free_String();
    void Store_Length(int32_t length);
    void Store_Allocated_Length(int32_t allocated_length);
    HEADER *Get_Header() const;
    int32_t Get_Allocated_Length() const;
    void Set_Buffer_And_Allocated_Length(char *buffer, size_t length);

    char *m_buffer;

    static char m_tempStrings[StringClass::MAX_TEMP_STRING][StringClass::MAX_TEMP_BYTES];
#ifdef GAME_DLL
    static FastCriticalSectionClass &m_mutex;
    static char &m_nullChar;
    static char *&m_emptyString;
    static unsigned &m_reserveMask;
#else
    static FastCriticalSectionClass m_mutex;
    static char m_nullChar;
    static char *m_emptyString;
    static unsigned m_reserveMask;
#endif
};

inline StringClass::StringClass(bool hint_temporary) : m_buffer(m_emptyString)
{
    Get_String(MAX_TEMP_LEN, hint_temporary);
    m_buffer[0] = m_nullChar;
}

inline StringClass::StringClass(int32_t initial_len, bool hint_temporary) : m_buffer(m_emptyString)
{
    Get_String(initial_len, hint_temporary);
    m_buffer[0] = m_nullChar;
}

inline StringClass::StringClass(char ch, bool hint_temporary) : m_buffer(m_emptyString)
{
    Get_String(2, hint_temporary);
    *this = ch;
}

inline StringClass::StringClass(const StringClass &string, bool hint_temporary) : m_buffer(m_emptyString)
{
    if (hint_temporary || (string.Get_Length() > 0)) {
        Get_String(string.Get_Length() + 1, hint_temporary);
    }

    *this = string;
}

inline StringClass::StringClass(const char *string, bool hint_temporary) : m_buffer(m_emptyString)
{
    size_t len = string ? strlen(string) : 0;

    if (hint_temporary || len > 0) {
        Get_String(len + 1, hint_temporary);
    }

    *this = string;
}

inline StringClass::StringClass(const unichar_t *string, bool hint_temporary) : m_buffer(m_emptyString)
{
    size_t len = string ? u_strlen(string) : 0;

    if (hint_temporary || len > 0) {
        Get_String(len + 1, hint_temporary);
    }

    *this = string;
}

inline StringClass::~StringClass()
{
    Free_String();
}

inline const StringClass &StringClass::operator=(const StringClass &string)
{
    int32_t len = string.Get_Length();
    Uninitialised_Grow(len + 1);
    Store_Length(len);

    memcpy(m_buffer, string.m_buffer, (len + 1) * sizeof(char));

    return *this;
}

inline const StringClass &StringClass::operator=(const char *string)
{
    if (string != 0) {
        size_t len = strlen(string);
        Uninitialised_Grow(len + 1);
        Store_Length(len);

        memcpy(m_buffer, string, (len + 1) * sizeof(char));
    }

    return *this;
}

inline const StringClass &StringClass::operator=(const unichar_t *string)
{
    if (string != 0) {
        Copy_Wide(string);
    }

    return *this;
}

inline const StringClass &StringClass::operator=(char ch)
{
    Uninitialised_Grow(2);

    m_buffer[0] = ch;
    m_buffer[1] = m_nullChar;
    Store_Length(1);

    return *this;
}

inline void StringClass::Erase(int32_t start_index, int32_t char_count)
{
    int32_t len = Get_Length();

    if (start_index < len) {
        if (start_index + char_count > len) {
            char_count = len - start_index;
        }

        memmove(&m_buffer[start_index],
            &m_buffer[start_index + char_count],
            (len - (start_index + char_count) + 1) * sizeof(char));

        Store_Length(len - char_count);
    }
}

inline void StringClass::Truncate_Left(unsigned truncateLength)
{
    unsigned length = Get_Length();

    if (length <= truncateLength) {
        Free_String();
    } else {
        int32_t newLength = length - truncateLength;
        memmove(m_buffer, m_buffer + truncateLength, newLength + 1);
        Store_Length(newLength);
    }
}

inline void StringClass::Truncate_Right(unsigned truncateLength)
{
    unsigned length = Get_Length();
    if (length <= truncateLength) {
        Free_String();
    } else {
        int32_t newLength = length - truncateLength;
        m_buffer[newLength] = '\0';
        Store_Length(newLength);
    }
}

inline void StringClass::Trim_Left()
{
    char *iter = m_buffer;
    for (; *iter != '\0' && *iter <= ' '; ++iter)
        ;

    Truncate_Left((int32_t)(iter - m_buffer));
}

inline void StringClass::Trim_Right()
{
    char *iter = m_buffer + Get_Length() - 1;
    for (; *iter != '\0' && *iter <= ' '; --iter)
        ;

    Truncate_Right((int32_t)(m_buffer + Get_Length() - 1 - iter));
}

inline void StringClass::Trim()
{
    Trim_Left();
    Trim_Right();
}

inline const StringClass &StringClass::operator+=(const char *string)
{
    int32_t cur_len = Get_Length();
    size_t src_len = strlen(string);
    int32_t new_len = cur_len + src_len;

    Resize(new_len + 1);
    Store_Length(new_len);
    memcpy(&m_buffer[cur_len], string, (src_len + 1) * sizeof(char));

    return *this;
}

inline const StringClass &StringClass::operator+=(char ch)
{
    int32_t cur_len = Get_Length();
    Resize(cur_len + 2);

    m_buffer[cur_len] = ch;
    m_buffer[cur_len + 1] = m_nullChar;

    if (ch != m_nullChar) {
        Store_Length(cur_len + 1);
    }

    return *this;
}

inline char *StringClass::Get_Buffer(int32_t new_length)
{
    Uninitialised_Grow(new_length);

    return m_buffer;
}

inline const StringClass &StringClass::operator+=(const StringClass &string)
{
    int32_t src_len = string.Get_Length();
    if (src_len > 0) {
        int32_t cur_len = Get_Length();
        int32_t new_len = cur_len + src_len;

        Resize(new_len + 1);
        Store_Length(new_len);

        memcpy(&m_buffer[cur_len], (const char *)string, (src_len + 1) * sizeof(char));
    }

    return *this;
}

inline StringClass operator+(const StringClass &string1, const StringClass &string2)
{
    StringClass new_string(string1, true);
    new_string += string2;
    return new_string;
}

inline StringClass operator+(const char *string1, const StringClass &string2)
{
    StringClass new_string(string1, true);
    new_string += string2;
    return new_string;
}

inline StringClass operator+(const StringClass &string1, const char *string2)
{
    StringClass new_string(string1, true);
    StringClass new_string2(string2, true);
    new_string += new_string2;

    return new_string;
}

inline int32_t StringClass::Get_Allocated_Length() const
{
    int32_t allocated_length = 0;

    if (m_buffer != m_emptyString) {
        HEADER *header = Get_Header();
        allocated_length = header->allocated_length;
    }

    return allocated_length;
}

inline int32_t StringClass::Get_Length() const
{
    size_t length = 0;

    if (m_buffer != m_emptyString) {
        HEADER *header = Get_Header();
        length = header->length;

        if (length == 0) {
            length = strlen(m_buffer);
            ((StringClass *)this)->Store_Length(length);
        }
    }

    return length;
}

inline void StringClass::Set_Buffer_And_Allocated_Length(char *buffer, size_t length)
{
    Free_String();
    m_buffer = buffer;

    if (m_buffer != m_emptyString) {
        Store_Allocated_Length(length);
        Store_Length(0);
    }
}

inline char *StringClass::Allocate_Buffer(size_t length)
{
    char *buffer = new char[(sizeof(char) * length) + sizeof(StringClass::HEADER)];

    HEADER *header = reinterpret_cast<HEADER *>(buffer);
    header->length = 0;
    header->allocated_length = length;

    return reinterpret_cast<char *>(buffer + sizeof(StringClass::HEADER));
}

inline StringClass::HEADER *StringClass::Get_Header() const
{
    return reinterpret_cast<HEADER *>(((char *)m_buffer) - sizeof(StringClass::HEADER));
}

inline void StringClass::Store_Allocated_Length(int32_t allocated_length)
{
    if (m_buffer != m_emptyString) {
        HEADER *header = Get_Header();
        header->allocated_length = allocated_length;
    }
}

inline void StringClass::Store_Length(int32_t length)
{
    if (m_buffer != m_emptyString) {
        HEADER *header = Get_Header();
        header->length = length;
    }
}

/**
 * Used when StringClass is used as a key for HashTemplateClass
 */
inline unsigned StringClass::Get_Hash() const
{
    int32_t length = Get_Length();

    if (length >= 8) {
        uint32_t value = *reinterpret_cast<uint32_t *>(&m_buffer[length - 8]);
        return (value >> 20) + value + (value >> 10) + (value >> 5);
    }

    unsigned result = 0;

    if (length != 0) {
        for (int32_t i = 0; i < length; ++i) {
            unsigned val = (uint8_t)m_buffer[i];
            result = val + 38 * result;
        }
    }

    return result;
}
