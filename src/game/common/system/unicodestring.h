////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: UNICODESTRING.H
//
//        Author:: CCHyper
//
//  Contributors:: OmniBlade
//
//   Description:: 
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef UNICODESTRING_H
#define UNICODESTRING_H


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include <wchar.h>

#define g_unicodeStringCriticalSection (Make_Global<SimpleCriticalSectionClass*>(0x00A2A294))

class AsciiString;

class UnicodeString
{
public:
    friend class AsciiString;
        
    enum {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 32767,
    };

    struct UnicodeStringData
    {
    #ifdef GAME_DEBUG_STRUCTS
        wchar_t *debug_ptr;
    #endif // GAME_DEBUG_STRUCTS

        uint16_t ref_count;
        uint16_t num_chars_allocated;

        wchar_t *Peek()
        {
            // Actual string data is stored immediately after the UnicodeStringData header.
            // wchar_a to avoid strict aliasing issues on gcc/clang
            return reinterpret_cast<wchar_a*>(&this[1]);
        }

    };

    UnicodeString();
    UnicodeString(const wchar_t *s);
    UnicodeString(char16_t const *s);
    UnicodeString(UnicodeString const &string);
    //UnicodeString(AsciiString const &string);
    ~UnicodeString();

    UnicodeString &operator=(const wchar_t *s) { Set(s); return *this; }
    UnicodeString &operator=(char16_t const *s) { Set(s); return *this; }
    UnicodeString &operator=(UnicodeString const &string) { Set(string); return *this; }
    //UnicodeString &operator=(AsciiString const &string) { Set(string); return *this; }

    UnicodeString &operator+=(wchar_t s) { Concat(s); return *this; }
    UnicodeString &operator+=(const wchar_t *s) { Concat(s); return *this; }
    UnicodeString &operator+=(UnicodeString const &s) { Concat(s); return *this; }
    //UnicodeString &operator+=(AsciiString const &string);

    //TODO
    //wchar_t *operator[](int index) const { return m_data->Peek()[index]; }

    void Validate();
    wchar_t *Peek() const;
    void Release_Buffer();
    void Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data = false, const wchar_t *str_to_cpy = nullptr, const wchar_t *str_to_cat = nullptr);
    int Get_Length() const;
    void Clear();
    wchar_t Get_Char(int) const;
    const wchar_t *Str() const;
    wchar_t *Get_Buffer_For_Read(int len);
    void Set(const wchar_t *s);
    void Set(const char16_t *s);
    void Set(UnicodeString const &string);

    void Translate(AsciiString const &string);

    void Concat(wchar_t c);
    void Concat(const wchar_t *s);
    void Concat(UnicodeString const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(const wchar_t *format, ...);
    void Format(UnicodeString format, ...);
    void Format_VA(const wchar_t *format, char *args);
    void Format_VA(UnicodeString &format, char *args);

    int Compare(const wchar_t *s) const { return wcscmp(Str(), s); };
    int Compare(UnicodeString const &string) const { return wcscmp(Str(), string.Str()); };

    int Compare_No_Case(const wchar_t *s) const { return wcscasecmp(Str(), s); };
    int Compare_No_Case(UnicodeString const &string) const { return wcscasecmp(Str(), string.Str()); };

    bool Next_Token(UnicodeString *tok, UnicodeString delims);

    bool Is_None() { return m_data != nullptr && wcscasecmp(Peek(), L"None") == 0; }
    bool Is_Empty() { return Get_Length() <= 0; }
    bool Is_Not_Empty() { return !Is_Empty(); }
    bool Is_Not_None() { return !Is_None(); }

private:
    static wchar_t *Char16_To_WChar(wchar_t *dst, char16_t const *src);

    // 
    static UnicodeString const EmptyString;

    // 
    UnicodeStringData *m_data;
};

inline bool operator==(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) == 0; }
inline bool operator==(UnicodeString const &left, const wchar_t *right) { return left.Compare(right) == 0; }
inline bool operator==(const wchar_t *left, UnicodeString const &right) { return right.Compare(left) == 0; }

inline bool operator!=(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) != 0; }
inline bool operator!=(UnicodeString const &left, const wchar_t *right) { return left.Compare(right) != 0; }
inline bool operator!=(const wchar_t *left, UnicodeString const &right) { return right.Compare(left) != 0; }

inline bool operator<(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) < 0; }
inline bool operator<(UnicodeString const &left, const wchar_t *right) { return left.Compare(right) < 0; }
inline bool operator<(const wchar_t *left, UnicodeString const &right) { return right.Compare(left) < 0; }

inline bool operator>(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) > 0; }
inline bool operator>(UnicodeString const &left, const wchar_t *right) { return left.Compare(right) > 0; }
inline bool operator>(const wchar_t *left, UnicodeString const &right) { return right.Compare(left) > 0; }

#endif // _UNICODESTRING_H
