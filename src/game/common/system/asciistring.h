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
#include "memdynalloc.h"
#include <cstdarg>
#include <cstring>

// Using STLPort seems to screw up some of the C++11 header inclusions.
#ifndef GAME_DLL
#include <atomic>
#endif

class Utf16String;

class Utf8String
{
    // So we can hook functions we think should be private.
    friend void Setup_Hooks();
    friend class Utf16String;

public:
    enum
    {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 32767,
        MAX_TO_LOWER_BUF_LEN = 2060,
    };

    struct AsciiStringData
    {
#ifdef GAME_DEBUG_STRUCTS
        char *debug_ptr;
#endif // GAME_DEBUG_STRUCTS

#ifndef GAME_DLL
        std::atomic<uint16_t> ref_count;
#else
        uint16_t ref_count;
#endif
        uint16_t num_chars_allocated;

        void Inc_Ref_Count()
        {
#ifndef GAME_DLL
            ++ref_count;
#elif defined PLATFORM_WINDOWS
            InterlockedIncrement16((volatile short *)&ref_count);
#endif
        }

        void Dec_Ref_Count()
        {
#ifndef GAME_DLL
            --ref_count;
#elif defined PLATFORM_WINDOWS
            InterlockedDecrement16((volatile short *)&ref_count);
#endif
        }

        char *Peek()
        {
            // Actual string data is stored immediately after the AsciiStringData header.
            return reinterpret_cast<char *>(&this[1]);
        }
    };

    Utf8String();
    Utf8String(const char *s);
    Utf8String(Utf8String const &string);
    // Utf8String(Utf16String const &stringSrc);
    ~Utf8String() { Release_Buffer(); }

    Utf8String &operator=(char *s);
    Utf8String &operator=(const char *s);
    Utf8String &operator=(Utf8String const &stringSrc);
    // Utf8String &operator=(Utf16String const &stringSrc);

    Utf8String &operator+=(char s);
    Utf8String &operator+=(const char *s);
    Utf8String &operator+=(Utf8String const &s);
    // Utf8String &operator+=(Utf16String const &stringSrc);

    operator const char *() const { return Str(); }

    void Validate();
    char *Peek() const;
    void Release_Buffer();
    int Get_Length() const;

    void Clear() { Release_Buffer(); }
    char Get_Char(int index) const;
    const char *Str() const;
    char *Get_Buffer_For_Read(int len);
    // These two should probably be private with the = operator being the preferred interface?
    void Set(const char *s);
    void Set(Utf8String const &string);

    void Translate(Utf16String const &stringSrc);

    // Concat should probably be private and += used as the preferred interface.
    void Concat(char c);
    void Concat(const char *s);
    void Concat(Utf8String const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(const char *format, ...);
    void Format(Utf8String format, ...);

    // Compare funcs should probably be private and operators should be friends and the
    // preferred interface.
    int Compare(const char *s) const { return strcmp(Str(), s); }
    int Compare(Utf8String const &string) const { return strcmp(Str(), string.Str()); }

    int Compare_No_Case(const char *s) const { return strcasecmp(Str(), s); }
    int Compare_No_Case(Utf8String const &string) const { return strcasecmp(Str(), string.Str()); }

    // I assume these do this, though have no examples in binaries.
    char *Find(char c) { return strchr(Peek(), c); }
    char *Reverse_Find(char c) { return strrchr(Peek(), c); }

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
    friend bool operator<(const char *left, Utf8String const &right) { return right.Compare(left) >= 0; }
    friend bool operator>(Utf8String const &left, Utf8String const &right) { return left.Compare(right) > 0; }
    friend bool operator>(Utf8String const &left, const char *right) { return left.Compare(right) < 0; }
    friend bool operator>(const char *left, Utf8String const &right) { return right.Compare(left) >= 0; }

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

#ifdef GAME_DLL // This define is only set when building a hooking DLL.
    Utf8String *Hook_Ctor1(const char *s) { return new (this) Utf8String(s); }
    Utf8String *Hook_Ctor2(Utf8String const &string) { return new (this) Utf8String(string); }
#endif

private:
    // Probably supposed to be private
    void Ensure_Unique_Buffer_Of_Size(
        int chars_needed, bool keep_data = false, const char *str_to_copy = nullptr, const char *str_to_cat = nullptr);
    void Free_Bytes();
    void Format_VA(const char *format, va_list args);
    void Format_VA(Utf8String &format, va_list args);

    AsciiStringData *m_data;
};

inline Utf8String &Utf8String::operator=(char *s)
{
    Set(s);

    return *this;
}

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
