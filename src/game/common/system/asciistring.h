/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling strings that have a series of bytes as a code point.
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
#include <cstdarg>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <strings.h>

// Using STLPort seems to screw up some of the C++11 header inclusions.
#ifndef GAME_DLL
#include <atomic>
#endif

class Utf16String;

class Utf8String
{
    // So we can hook functions we think should be private.
    friend void Setup_Hooks();

public:
    using value_type = char;
    using size_type = int;
    using difference_type = std::ptrdiff_t;
    using reference = value_type &;
    using const_reference = const value_type &;
    using pointer = value_type *;
    using const_pointer = const value_type *;

    enum
    {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 0x7FFF,
        MAX_TO_LOWER_BUF_LEN = 2060,
    };

    struct AsciiStringData
    {
#ifdef GAME_DEBUG_STRUCTS
        char *debug_ptr;
#endif // GAME_DEBUG_STRUCTS

#ifndef GAME_DLL
        std::atomic<int16_t> ref_count;
#else
        volatile int16_t ref_count;
#endif
        uint16_t num_chars_allocated;

        void Inc_Ref_Count()
        {
#ifndef GAME_DLL
            ++ref_count;
#elif defined PLATFORM_WINDOWS
            InterlockedIncrement16(&ref_count);
#endif
        }

        void Dec_Ref_Count()
        {
#ifndef GAME_DLL
            --ref_count;
#elif defined PLATFORM_WINDOWS
            InterlockedDecrement16(&ref_count);
#endif
        }

        // Actual string data is stored immediately after the AsciiStringData header.
        const char *Peek() const { return reinterpret_cast<const char *>(&this[1]); }
        char *Peek() { return reinterpret_cast<char *>(&this[1]); }
    };

    Utf8String();
    Utf8String(const char *s);
    Utf8String(Utf8String const &string);
    // Utf8String(Utf16String const &stringSrc);
    ~Utf8String() { Release_Buffer(); }

    Utf8String &operator=(const char *s);
    Utf8String &operator=(Utf8String const &stringSrc);
    // Utf8String &operator=(Utf16String const &stringSrc);

    Utf8String &operator+=(char s);
    Utf8String &operator+=(const char *s);
    Utf8String &operator+=(Utf8String const &s);
    // Utf8String &operator+=(Utf16String const &stringSrc);

    char operator[](size_type index) const { return Get_Char(index); }

    void Validate();
    const char *Peek() const;
    void Release_Buffer();
    size_type Get_Length() const;

    void Clear() { Release_Buffer(); }
    char Get_Char(size_type index) const;
    const char *Str() const;
    char *Get_Buffer_For_Read(size_type len);
    // These two should probably be private with the = operator being the preferred interface?
    void Set(const char *s);
    void Set(Utf8String const &string);

    void Translate(Utf16String const &utf16_string);
    void Translate(const unichar_t *utf16_string);

    // Concat should probably be private and += used as the preferred interface.
    void Concat(char c);
    void Concat(const char *s);
    void Concat(Utf8String const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(const char *format, ...);
    void Format(Utf8String format, ...);

    // Compare functions should probably be private and operators should be friends and the.
    // preferred interface.
    size_type Compare(const char *s) const { return strcmp(Str(), s); }
    size_type Compare(Utf8String const &string) const { return strcmp(Str(), string.Str()); }

    size_type Compare_No_Case(const char *s) const { return strcasecmp(Str(), s); }
    size_type Compare_No_Case(Utf8String const &string) const { return strcasecmp(Str(), string.Str()); }

    // I assume these do this, though have no examples in binaries.
    const char *Find(char c) const { return strchr(Str(), c); }
    const char *Reverse_Find(char c) const { return strrchr(Str(), c); }

    bool Starts_With(const char *p) const;
    bool Ends_With(const char *p) const;

    bool Starts_With_No_Case(const char *p) const;
    bool Ends_With_No_Case(const char *p) const;

    bool Next_Token(Utf8String *tok, const char *seps = nullptr);

    bool Is_None() const { return m_data != nullptr && strcasecmp(Peek(), "None") == 0; }
    bool Is_Empty() const { return m_data == nullptr || *m_data->Peek() == '\0'; }
    bool Is_Not_Empty() const { return !Is_Empty(); }
    bool Is_Not_None() const { return !Is_None(); }

    Utf8String Posix_Path() const;
    Utf8String Windows_Path() const;

    friend bool operator==(Utf8String const &left, Utf8String const &right) { return left.Compare(right) == 0; }
    friend bool operator==(Utf8String const &left, const char *right) { return left.Compare(right) == 0; }
    friend bool operator==(const char *left, Utf8String const &right) { return right.Compare(left) == 0; }

    friend bool operator!=(Utf8String const &left, Utf8String const &right) { return left.Compare(right) != 0; }
    friend bool operator!=(Utf8String const &left, const char *right) { return left.Compare(right) != 0; }
    friend bool operator!=(const char *left, Utf8String const &right) { return right.Compare(left) != 0; }

    friend bool operator<(Utf8String const &left, Utf8String const &right) { return left.Compare(right) < 0; }
    friend bool operator<(Utf8String const &left, const char *right) { return left.Compare(right) < 0; }
    friend bool operator<(const char *left, Utf8String const &right) { return !(right.Compare(left) < 0); }

    friend bool operator>(Utf8String const &left, Utf8String const &right) { return left.Compare(right) > 0; }
    friend bool operator>(Utf8String const &left, const char *right) { return left.Compare(right) > 0; }
    friend bool operator>(const char *left, Utf8String const &right) { return !(right.Compare(left) > 0); }

    friend Utf8String operator+(const Utf8String &a, const Utf8String &b)
    {
        Utf8String retval = a;
        retval += b;

        return retval;
    }

#ifdef GAME_DEBUG
    void Debug_Ignore_Leaks();
#endif // GAME_DEBUG
public:
    static Utf8String const s_emptyString;

#ifdef GAME_DLL
    Utf8String *Hook_Ctor1(const char *s);
    Utf8String *Hook_Ctor2(Utf8String const &string);
#endif

protected:
    char *Peek();

private:
    void Translate_Internal(const unichar_t *utf16_string, const size_type utf16_len);

    // Probably supposed to be private
    void Ensure_Unique_Buffer_Of_Size(
        size_type chars_needed, bool keep_data = false, const char *str_to_copy = nullptr, const char *str_to_cat = nullptr);
    void Free_Bytes();
    void Format_VA(const char *format, va_list args);
    void Format_VA(Utf8String &format, va_list args);

    AsciiStringData *m_data;
};

inline Utf8String &Utf8String::operator=(const char *s)
{
    Set(s);

    return *this;
}

inline Utf8String &Utf8String::operator=(Utf8String const &stringSrc)
{
    Set(stringSrc);

    return *this;
}

// Utf8String &operator=(Utf16String const &stringSrc);

inline Utf8String &Utf8String::operator+=(char s)
{
    Concat(s);

    return *this;
}

inline Utf8String &Utf8String::operator+=(const char *s)
{
    Concat(s);

    return *this;
}

inline Utf8String &Utf8String::operator+=(Utf8String const &s)
{
    Concat(s);

    return *this;
}

// Windows to POSIX names
// strcmpi = strcasecmp
// strnicmp = strncasecmp
// sprintf_s = snprintf
