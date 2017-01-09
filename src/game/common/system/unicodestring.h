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
#ifdef COMPILER_MSVC
#pragma once
#endif // COMPILER_MSVC

#ifndef _UNICODESTRING_H_
#define _UNICODESTRING_H_


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "always.h"
#include <wchar.h>

#define UnicodeStringCriticalSection (Make_Global<SimpleCriticalSectionClass*>(0x00A2A294))

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

            // 
        #ifdef GAME_DEBUG_STRUCTS
            wchar_t *DebugPtr;
        #endif // GAME_DEBUG_STRUCTS

            // 
            unsigned short RefCount;

            // 
            unsigned short NumCharsAllocated;

            // 
            wchar_t *Peek()
            {
                // Actual string data is stored immediately after the UnicodeStringData header.
                return reinterpret_cast<wchar_t*>(&this[1]);
            }

        };

        UnicodeString();
        UnicodeString(wchar_t const *s);
        UnicodeString(UnicodeString const &string);
        //UnicodeString(AsciiString const &string);
        ~UnicodeString();

        UnicodeString &operator=(wchar_t *s) { Set(s); return *this; }
        UnicodeString &operator=(wchar_t const *s) { Set(s); return *this; }
        UnicodeString &operator=(UnicodeString const &string);
        //UnicodeString &operator=(AsciiString const &string) { Set(string); return *this; }

        UnicodeString &operator+=(wchar_t s);
        UnicodeString &operator+=(wchar_t const *s);
        UnicodeString &operator+=(UnicodeString const &string);
        //UnicodeString &operator+=(AsciiString const &string);

        //TODO
        //wchar_t *operator[](int index) const { return Data->Peek()[index]; }

        void Validate();
        wchar_t *Peek() const;
        void Release_Buffer();
        void Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data = false, wchar_t const *str_to_cpy = nullptr, wchar_t const *str_to_cat = nullptr);
        size_t Get_Length() const;
        void Clear();
        wchar_t Get_Char(int) const;
        wchar_t const *Str();
        wchar_t *Get_Buffer_For_Read(int len);
        void Set(wchar_t const *s);
        void Set(UnicodeString const &string);

        void Translate(AsciiString const &string);

        void Concat(wchar_t c);
        void Concat(wchar_t *s);
        void Concat(UnicodeString const &string) { Concat(string.Peek()); }

        void Trim();
        void To_Lower();
        void Remove_Last_Char();

        void Format(wchar_t const *format, ...);
        void Format(UnicodeString format, ...);
        void Format_VA(wchar_t const *format, char *args);
        void Format_VA(UnicodeString &format, char *args);

        int Compare(wchar_t const *s) const { return wcscmp(Peek(), s); };
        int Compare(UnicodeString const &string) const { return wcscmp(Peek(), string.Peek()); };

        int Compare_No_Case(wchar_t const *s) const { return wcscasecmp(Peek(), s); };
        int Compare_No_Case(UnicodeString const &string) const { return wcscasecmp(Peek(), string.Peek()); };

        bool Next_Token(UnicodeString *tok, UnicodeString delims);

        bool Is_None() { return Data != nullptr && wcscasecmp(Peek(), L"None") == 0; }
        bool Is_Empty() { return Data == nullptr || *Data->Peek() == L'\0'; }
        bool Is_Not_Empty() { return !Is_Empty(); }
        bool Is_Not_None() { return !Is_None(); }

    private:

        // 
        static UnicodeString const EmptyString;

        // 
        UnicodeStringData *Data;
};

inline bool operator==(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) == 0; }
inline bool operator==(UnicodeString const &left, wchar_t const *right) { return left.Compare(right) == 0; }
inline bool operator==(wchar_t const *left, UnicodeString const &right) { return right.Compare(left) == 0; }

inline bool operator!=(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) != 0; }
inline bool operator!=(UnicodeString const &left, wchar_t const *right) { return left.Compare(right) != 0; }
inline bool operator!=(wchar_t const *left, UnicodeString const &right) { return right.Compare(left) != 0; }

inline bool operator<(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) < 0; }
inline bool operator<(UnicodeString const &left, wchar_t const *right) { return left.Compare(right) < 0; }
inline bool operator<(wchar_t const *left, UnicodeString const &right) { return right.Compare(left) < 0; }

inline bool operator>(UnicodeString const &left, UnicodeString const &right) { return left.Compare(right) > 0; }
inline bool operator>(UnicodeString const &left, wchar_t const *right) { return left.Compare(right) > 0; }
inline bool operator>(wchar_t const *left, UnicodeString const &right) { return right.Compare(left) > 0; }

#endif // _UNICODESTRING_H_
