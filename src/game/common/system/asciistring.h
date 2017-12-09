/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class for handling strings that have a single byte as a code point.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#ifndef ASCIISTRING_H
#define ASCIISTRING_H

#include "always.h"
#include "memdynalloc.h"

class UnicodeString;

class AsciiString
{
    // So we can hook functions we think should be private.
    friend void Setup_Hooks();
    friend class UnicodeString;

public:
    enum
    {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 32767,
    };

    struct AsciiStringData
    {
#ifdef GAME_DEBUG_STRUCTS
        char *debug_ptr;
#endif // GAME_DEBUG_STRUCTS

        uint16_t ref_count;
        uint16_t num_chars_allocated;

        void Inc_Ref_Count()
        {
#ifdef COMPILER_MSVC
            InterlockedIncrement16((volatile short *)&ref_count);
#elif defined COMPILER_GNUC || defined COMPILER_CLANG
            __sync_add_and_fetch(&ref_count, 1);
#else
#error Compiler not supported, add atomic increment to asciistring.h
#endif
        }

        void Dec_Ref_Count()
        {
#ifdef COMPILER_MSVC
            InterlockedDecrement16((volatile short *)&ref_count);
#elif defined COMPILER_GNUC || defined COMPILER_CLANG
            __sync_sub_and_fetch(&ref_count, 1);
#else
#error Compiler not supported, add atomic decrement to asciistring.h
#endif
        }

        char *Peek()
        {
            // Actual string data is stored immediately after the AsciiStringData header.
            return reinterpret_cast<char *>(&this[1]);
        }
    };

    AsciiString();
    AsciiString(const char *s);
    AsciiString(AsciiString const &string);
    // AsciiString(UnicodeString const &stringSrc);
    ~AsciiString() { Release_Buffer(); }

    AsciiString &operator=(char *s);
    AsciiString &operator=(const char *s);
    AsciiString &operator=(AsciiString const &stringSrc);
    // AsciiString &operator=(UnicodeString const &stringSrc);

    AsciiString &operator+=(char s);
    AsciiString &operator+=(const char *s);
    AsciiString &operator+=(AsciiString const &s);
    // AsciiString &operator+=(UnicodeString const &stringSrc);

    operator const char *() { return Str(); }

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
    void Set(AsciiString const &string);

    void Translate(UnicodeString const &stringSrc);

    // Concat should probably be private and += used as the preferred interface.
    void Concat(char c);
    void Concat(const char *s);
    void Concat(AsciiString const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(const char *format, ...);
    void Format(AsciiString format, ...);

    // Compare funcs should probably be private and operators should be friends and the
    // preferred interface.
    int Compare(const char *s) const { return strcmp(Str(), s); }
    int Compare(AsciiString const &string) const { return strcmp(Str(), string.Str()); }

    int Compare_No_Case(const char *s) const { return strcasecmp(Str(), s); }
    int Compare_No_Case(AsciiString const &string) const { return strcasecmp(Str(), string.Str()); }

    // I assume these do this, though have no examples in binaries.
    char *Find(char c) { return strchr(Peek(), c); }
    char *Reverse_Find(char c) { return strrchr(Peek(), c); }

    bool Starts_With(const char *p) const;
    bool Ends_With(const char *p) const;

    bool Starts_With_No_Case(const char *p) const;
    bool Ends_With_No_Case(const char *p) const;

    bool Next_Token(AsciiString *tok, const char *seps);

    bool Is_None() const { return m_data != nullptr && strcasecmp(Peek(), "None") == 0; }
    bool Is_Empty() const { return  m_data == nullptr || *m_data->Peek() == '\0'; }
    bool Is_Not_Empty() const { return !Is_Empty(); }
    bool Is_Not_None() const { return !Is_None(); }

    AsciiString Posix_Path() const;
    AsciiString Windows_Path() const;

#ifdef GAME_DEBUG
    void Debug_Ignore_Leaks();
#endif // GAME_DEBUG
public:
    static AsciiString const s_emptyString;

private:
    // Probably supposed to be private
    void Ensure_Unique_Buffer_Of_Size(
        int chars_needed, bool keep_data = false, const char *str_to_copy = nullptr, const char *str_to_cat = nullptr);

    void Format_VA(const char *format, va_list args);
    void Format_VA(AsciiString &format, va_list args);

    AsciiStringData *m_data;
};

inline AsciiString &AsciiString::operator=(char *s)
{
    Set(s);

    return *this;
}

inline AsciiString &AsciiString::operator=(const char *s)
{
    Set(s);

    return *this;
}

inline AsciiString &AsciiString::operator=(AsciiString const &stringSrc)
{
    Set(stringSrc);

    return *this;
}

// AsciiString &operator=(UnicodeString const &stringSrc);

inline AsciiString &AsciiString::operator+=(char s)
{
    Concat(s);

    return *this;
}

inline AsciiString &AsciiString::operator+=(const char *s)
{
    Concat(s);

    return *this;
}

inline AsciiString &AsciiString::operator+=(AsciiString const &s)
{
    Concat(s);

    return *this;
}

inline bool operator==(AsciiString const &left, AsciiString const &right)
{
    return left.Compare(right) == 0;
}

inline bool operator==(AsciiString const &left, const char *right)
{
    return left.Compare(right) == 0;
}

inline bool operator==(const char *left, AsciiString const &right)
{
    return right.Compare(left) == 0;
}

inline bool operator!=(AsciiString const &left, AsciiString const &right)
{
    return left.Compare(right) != 0;
}

inline bool operator!=(AsciiString const &left, const char *right)
{
    return left.Compare(right) != 0;
}

inline bool operator!=(const char *left, AsciiString const &right)
{
    return right.Compare(left) != 0;
}

inline bool operator<(AsciiString const &left, AsciiString const &right)
{
    return left.Compare(right) < 0;
}

inline bool operator<(AsciiString const &left, const char *right)
{
    return left.Compare(right) < 0;
}

inline bool operator<(const char *left, AsciiString const &right)
{
    return right.Compare(left) >= 0;
}

inline bool operator>(AsciiString const &left, AsciiString const &right)
{
    return left.Compare(right) > 0;
}

inline bool operator>(AsciiString const &left, const char *right)
{
    return left.Compare(right) < 0;
}

inline bool operator>(const char *left, AsciiString const &right)
{
    return right.Compare(left) >= 0;
}

inline AsciiString operator+(const AsciiString &a, const AsciiString &b)
{
    AsciiString retval = a;
    retval += b;

    return retval;
}

#endif // _ASCIISTRING_H
