////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ASCIISTRING.H
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
#ifdef _MSC_VER
#pragma once
#endif // _MSC_VER

#ifndef _ASCIISTRING_H_
#define _ASCIISTRING_H_

////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include "memdynalloc.h"

class UnicodeString;

class AsciiString
{
    // So we can hook functions we think should be private.
    friend void Setup_Hooks();

    friend class UnicodeString;

public:
    enum {
        MAX_FORMAT_BUF_LEN = 2048,
        MAX_LEN = 32767,
    };

    struct AsciiStringData
    {
    #ifdef GAME_DEBUG_STRUCTS
        char *DebugPtr;
    #endif // GAME_DEBUG_STRUCTS

        uint16_t RefCount;
        uint16_t NumCharsAllocated;

        char *Peek()
        {
            // Actual string data is stored immediately after the AsciiStringData header.
            return reinterpret_cast<char *>(&this[1]);
        }
    };

    AsciiString();
    AsciiString(char const *s);
    AsciiString(AsciiString const &string);
    //AsciiString(UnicodeString const &stringSrc);
    ~AsciiString() { Release_Buffer(); }

    AsciiString &operator=(char *s) { Set(s); return *this; }
    AsciiString &operator=(char const *s) { Set(s); return *this; }
    AsciiString &operator=(AsciiString const &stringSrc) { Set(stringSrc); return *this; }
    //AsciiString &operator=(UnicodeString const &stringSrc);

    AsciiString &operator+=(char s) { Concat(s); return *this; }
    AsciiString &operator+=(char const *s) { Concat(s); return *this; }
    AsciiString &operator+=(AsciiString const &stringSrc) { Concat(stringSrc); return *this; }
    //AsciiString &operator+=(UnicodeString const &stringSrc);

    void Validate();
    char *Peek() const;
    void Release_Buffer();
    void Free_Bytes() { TheDynamicMemoryAllocator->Free_Bytes(Data); }
    int Get_Length() const;

    void Clear() { Release_Buffer(); }
    char Get_Char(int index) const;
    char const *Str() const;
    char *Get_Buffer_For_Read(int len);
    // These two should probably be private with the = operator being the preferred interface?
    void Set(char const *s);
    void Set(AsciiString const &string);

    void Translate(UnicodeString const &stringSrc);

    // Concat should probably be private and += used as the preferred interface.
    void Concat(char c);
    void Concat(char const *s);
    void Concat(AsciiString const &string) { Concat(string.Str()); }

    void Trim();
    void To_Lower();
    void Remove_Last_Char();

    void Format(char const *format, ...);
    void Format(AsciiString format, ...);
        
    // Compare funcs should probably be private and operators should be friends and the
    // preferred interface.
    int Compare(char const *s) const { return strcmp(Str(), s); }
    int Compare(AsciiString const &string) const { return strcmp(Str(), string.Str()); }

    int Compare_No_Case(char const *s) const { return stricmp(Str(), s); }
    int Compare_No_Case(AsciiString const &string) const { return stricmp(Str(), string.Str()); }
        
    // I assume these do this, though have no examples in binaries.
    char *Find(char c) { return strchr(Peek(), c); }
    char *Reverse_Find(char c) { return strrchr(Peek(), c); }

    bool Starts_With(char const *p);
    bool Ends_With(char const *p);

    bool Starts_With_No_Case(char const *p) const;
    bool Ends_With_No_Case(char const *p) const;

    bool Next_Token(AsciiString *tok, char const *seps);

    bool Is_None() const { return Data != nullptr && stricmp(Peek(), "None") == 0; }
    bool Is_Empty() const { return Get_Length() <= 0; }
    bool Is_Not_Empty() const { return !Is_Empty(); }
    bool Is_Not_None() const { return !Is_None(); }

#ifdef GAME_DEBUG
    void Debug_Ignore_Leaks();
#endif // GAME_DEBUG
public:
    static AsciiString const EmptyString;


private:
    //Probably supposed to be private
    void Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data = false, char const *str_to_copy = nullptr, char const *str_to_cat = nullptr);

    void Format_VA(char const *format, va_list args);
    void Format_VA(AsciiString &format, va_list args);

    AsciiStringData *Data;
};

inline bool operator==(AsciiString const &left, AsciiString const &right) { return left.Compare(right) == 0; }
inline bool operator==(AsciiString const &left, char const *right) { return left.Compare(right) == 0; }
inline bool operator==(char const *left, AsciiString const &right) { return right.Compare(left) == 0; }

inline bool operator!=(AsciiString const &left, AsciiString const &right) { return left.Compare(right) != 0; }
inline bool operator!=(AsciiString const &left, char const *right) { return left.Compare(right) != 0; }
inline bool operator!=(char const *left, AsciiString const &right) { return right.Compare(left) != 0; }

inline bool operator<(AsciiString const &left, AsciiString const &right) { return left.Compare(right) < 0; }
inline bool operator<(AsciiString const &left, char const *right) { return left.Compare(right) < 0; }
inline bool operator<(char const *left, AsciiString const &right) { return right.Compare(left) >= 0; }

inline bool operator>(AsciiString const &left, AsciiString const &right) { return left.Compare(right) > 0; }
inline bool operator>(AsciiString const &left, char const *right) { return left.Compare(right) < 0; }
inline bool operator>(char const *left, AsciiString const &right) { return right.Compare(left) >= 0; }

#endif // _ASCIISTRING_H_
