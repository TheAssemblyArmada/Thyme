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
#include <stdarg.h>

#ifdef BUILD_WITH_ICU
#include <unicode/ustring.h>
#include <unicode/ustdio.h>
#elif defined PLATFORM_WINDOWS
#include <wchar.h>
#else
#error Support for utf16<->utf8 conversion not found for this platform.
#endif

class Utf8String;

class Utf16String
{
public:
    friend class Utf8String;

    enum
    {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 32767,
    };

    struct UnicodeStringData
    {
#ifdef GAME_DEBUG_STRUCTS
        unichar_t *debug_ptr;
#endif // GAME_DEBUG_STRUCTS

        uint16_t ref_count;
        uint16_t num_chars_allocated;

        unichar_t *Peek()
        {
            // Actual string data is stored immediately after the UnicodeStringData header.
            // wchar_a to avoid strict aliasing issues on gcc/clang
            return reinterpret_cast<unichar_a *>(&this[1]);
        }
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

    operator const unichar_t *() const { return Str(); }

    // TODO
    // unichar_t *operator[](int index) const { return m_data->Peek()[index]; }

    void Validate();
    unichar_t *Peek() const;
    void Release_Buffer();
    void Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data = false, const unichar_t *str_to_cpy = nullptr,
        const unichar_t *str_to_cat = nullptr);
    int Get_Length() const;
    void Clear();
    unichar_t Get_Char(int) const;
    const unichar_t *Str() const;
    unichar_t *Get_Buffer_For_Read(int len);
    void Set(const unichar_t *s);
    void Set(Utf16String const &string);

    void Translate(Utf8String const &string);
    void Translate(const char *string);

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

    int Compare(const unichar_t *s) const { return u_strcmp(Str(), s); };
    int Compare(Utf16String const &string) const { return u_strcmp(Str(), string.Str()); };

    int Compare_No_Case(const unichar_t *s) const { return u_strcasecmp(Str(), s, U_COMPARE_CODE_POINT_ORDER); };
    int Compare_No_Case(Utf16String const &string) const
    {
        return u_strcasecmp(Str(), string.Str(), U_COMPARE_CODE_POINT_ORDER);
    };

    bool Next_Token(Utf16String *tok, Utf16String delims);

    bool Is_None()
    {
        return m_data != nullptr && u_strcasecmp(Peek(), (const unichar_t *)u"None", U_COMPARE_CODE_POINT_ORDER) == 0;
    }
    bool Is_Empty() { return Get_Length() <= 0; }
    bool Is_Not_Empty() { return !Is_Empty(); }
    bool Is_Not_None() { return !Is_None(); }

    friend bool operator==(Utf16String const &left, Utf16String const &right) { return left.Compare(right) == 0; }
    friend bool operator==(Utf16String const &left, const unichar_t *right) { return left.Compare(right) == 0; }
    friend bool operator==(const unichar_t *left, Utf16String const &right) { return right.Compare(left) == 0; }

    friend bool operator!=(Utf16String const &left, Utf16String const &right) { return left.Compare(right) != 0; }
    friend bool operator!=(Utf16String const &left, const unichar_t *right) { return left.Compare(right) != 0; }
    friend bool operator!=(const unichar_t *left, Utf16String const &right) { return right.Compare(left) != 0; }

    friend bool operator<(Utf16String const &left, Utf16String const &right) { return left.Compare(right) < 0; }
    friend bool operator<(Utf16String const &left, const unichar_t *right) { return left.Compare(right) < 0; }
    friend bool operator<(const unichar_t *left, Utf16String const &right) { return right.Compare(left) < 0; }

    friend bool operator>(Utf16String const &left, Utf16String const &right) { return left.Compare(right) > 0; }
    friend bool operator>(Utf16String const &left, const unichar_t *right) { return left.Compare(right) > 0; }
    friend bool operator>(const unichar_t *left, Utf16String const &right) { return right.Compare(left) > 0; }

private:
    static Utf16String const EmptyString;

    UnicodeStringData *m_data;
};

#ifdef GAME_DLL
extern SimpleCriticalSectionClass *&g_unicodeStringCriticalSection;
#else
extern SimpleCriticalSectionClass *g_unicodeStringCriticalSection;
#endif
