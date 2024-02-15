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
#pragma once

#include "always.h"
#include "critsection.h"
#include <cstdarg>
#include <cstddef>

#ifdef BUILD_WITH_ICU
#include <unicode/ustdio.h>
#include <unicode/ustring.h>
#elif defined PLATFORM_WINDOWS
#include <wchar.h>
#else
#error Support for utf16<->utf8 conversion not found for this platform.
#endif

class Utf8String;

class Utf16String
{
public:
    using value_type = unichar_t;
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
    };

    struct UnicodeStringData
    {
#ifdef GAME_DEBUG_STRUCTS
        unichar_t *debug_ptr;
#endif // GAME_DEBUG_STRUCTS

        volatile int16_t ref_count;
        uint16_t num_chars_allocated;

        // Actual string data is stored immediately after the UnicodeStringData header.
        // wchar_a to avoid strict aliasing issues on gcc/clang
        const unichar_t *Peek() const { return reinterpret_cast<const unichar_a *>(&this[1]); }
        unichar_t *Peek() { return reinterpret_cast<unichar_a *>(&this[1]); }
    };

    Utf16String() : m_data(nullptr) {}
    Utf16String(const unichar_t *s);
    Utf16String(Utf16String const &string);
    // Utf16String(Utf8String const &string);
    ~Utf16String();

    Utf16String &operator=(const unichar_t *s)
    {
        Set(s);
        return *this;
    }

    Utf16String &operator=(Utf16String const &string)
    {
        Set(string);
        return *this;
    }

    Utf16String &operator=(Utf8String const &string)
    {
        Translate(string);
        return *this;
    }

    Utf16String &operator=(const char *s)
    {
        Translate(s);
        return *this;
    }

    // Utf16String &operator=(Utf8String const &string) { Set(string); return *this; }

    Utf16String &operator+=(unichar_t s)
    {
        Concat(s);
        return *this;
    }

    Utf16String &operator+=(const unichar_t *s)
    {
        Concat(s);
        return *this;
    }

    Utf16String &operator+=(Utf16String const &s)
    {
        Concat(s);
        return *this;
    }
    // Utf16String &operator+=(Utf8String const &string);

    unichar_t operator[](size_type index) const { return Get_Char(index); }

    void Validate();
    const unichar_t *Peek() const;
    void Release_Buffer();
    size_type Get_Length() const;
    void Clear();
    unichar_t Get_Char(size_type index) const;
    const unichar_t *Str() const;
    unichar_t *Get_Buffer_For_Read(size_type len);
    void Set(const unichar_t *s);
    void Set(Utf16String const &string);

    void Translate(Utf8String const &utf8_string);
    void Translate(const char *utf8_string);

    void Concat(unichar_t c);
    void Concat(const unichar_t *s);
    void Concat(Utf16String const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(const unichar_t *format, ...);
    void Format(Utf16String format, ...);
    void Format_VA(const unichar_t *format, va_list args);
    void Format_VA(Utf16String &format, va_list args);

    size_type Compare(const unichar_t *s) const { return u_strcmp(Str(), s); };
    size_type Compare(Utf16String const &string) const { return u_strcmp(Str(), string.Str()); };

    size_type Compare_No_Case(const unichar_t *s) const { return u_strcasecmp(Str(), s, U_COMPARE_CODE_POINT_ORDER); };
    size_type Compare_No_Case(Utf16String const &string) const
    {
        return u_strcasecmp(Str(), string.Str(), U_COMPARE_CODE_POINT_ORDER);
    };

    bool Next_Token(Utf16String *tok, Utf16String delims);

    bool Is_None() const
    {
        return m_data != nullptr && u_strcasecmp(Peek(), U_CHAR("None"), U_COMPARE_CODE_POINT_ORDER) == 0;
    }
    bool Is_Empty() const { return Get_Length() <= 0; }
    bool Is_Not_Empty() const { return !Is_Empty(); }
    bool Is_Not_None() const { return !Is_None(); }

    friend bool operator==(Utf16String const &left, Utf16String const &right) { return left.Compare(right) == 0; }
    friend bool operator==(Utf16String const &left, const unichar_t *right) { return left.Compare(right) == 0; }
    friend bool operator==(const unichar_t *left, Utf16String const &right) { return right.Compare(left) == 0; }

    friend bool operator!=(Utf16String const &left, Utf16String const &right) { return left.Compare(right) != 0; }
    friend bool operator!=(Utf16String const &left, const unichar_t *right) { return left.Compare(right) != 0; }
    friend bool operator!=(const unichar_t *left, Utf16String const &right) { return right.Compare(left) != 0; }

    friend bool operator<(Utf16String const &left, Utf16String const &right) { return left.Compare(right) < 0; }
    friend bool operator<(Utf16String const &left, const unichar_t *right) { return left.Compare(right) < 0; }
    friend bool operator<(const unichar_t *left, Utf16String const &right) { return !(right.Compare(left) < 0); }

    friend bool operator>(Utf16String const &left, Utf16String const &right) { return left.Compare(right) > 0; }
    friend bool operator>(Utf16String const &left, const unichar_t *right) { return left.Compare(right) > 0; }
    friend bool operator>(const unichar_t *left, Utf16String const &right) { return !(right.Compare(left) > 0); }

public:
    static Utf16String const s_emptyString;

protected:
    unichar_t *Peek();

private:
    void Translate_Internal(const char *utf8_string, const size_type utf8_len);

    void Ensure_Unique_Buffer_Of_Size(size_type chars_needed,
        bool keep_data = false,
        const unichar_t *str_to_cpy = nullptr,
        const unichar_t *str_to_cat = nullptr);

    UnicodeStringData *m_data;
};

struct UnicodeStringLessThan
{
    bool operator()(Utf16String a, Utf16String b) const { return a.Compare_No_Case(b) < 0; }
};

#ifdef GAME_DLL
extern SimpleCriticalSectionClass *&g_unicodeStringCriticalSection;
#else
extern SimpleCriticalSectionClass *g_unicodeStringCriticalSection;
#endif
