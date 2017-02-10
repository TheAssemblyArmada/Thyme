////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: UNICODESTRING.CPP
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


////////////////////////////////////////////////////////////////////////////////
//  Includes
////////////////////////////////////////////////////////////////////////////////
#include "unicodestring.h"
#include "asciistring.h"
#include "critsection.h"
#include "gamedebug.h"
#include <stdio.h>

#ifndef vsnwprintf
#define vsnwprintf _vsnwprintf
#endif

UnicodeString const UnicodeString::EmptyString(nullptr);

UnicodeString::UnicodeString() :
    Data(nullptr)
{
}

UnicodeString::UnicodeString(wchar_t const *s) :
    Data(nullptr)
{
    if ( s != nullptr ) {
        size_t len = wcslen(s);

        if ( len > 0 ) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s, nullptr);
        }
    }
}

UnicodeString::UnicodeString(UnicodeString const &string) :
    Data(string.Data)
{
    ScopedCriticalSectionClass cs(UnicodeStringCriticalSection);

    if ( Data != nullptr ) {
        ++Data->RefCount;
    }
}

UnicodeString::~UnicodeString()
{
    Release_Buffer();
}

void UnicodeString::Validate()
{
}

wchar_t *UnicodeString::Peek() const
{
    ASSERT_PRINT(Data != nullptr, "null string ptr");

    //
    // Actual string data is stored immediately after the UnicodeStringData header.
    //
    return Data->Peek();
}

void UnicodeString::Release_Buffer()
{
    ScopedCriticalSectionClass cs(UnicodeStringCriticalSection);

    if ( Data != nullptr ) {
        if ( --Data->RefCount == 0 ) {
            TheDynamicMemoryAllocator->Free_Bytes(Data);
        }

        Data = nullptr;
    }
}

void UnicodeString::Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data, wchar_t const *str_to_cpy, wchar_t const *str_to_cat)
{
    if ( Data != nullptr && Data->RefCount == 1 && Data->NumCharsAllocated >= chars_needed ) {
        if ( str_to_cpy != nullptr ) {
            wcscpy(Peek(), str_to_cpy);
        }

        if ( str_to_cat != nullptr ) {
            wcscat(Peek(), str_to_cat);
        }

    } else {

        //this block would have been a macro like DEBUG_CRASH(numCharsNeeded + 8 > MAX_LEN, THROW_02);
        //if ( numCharsNeeded + 8 > MAX_LEN ) {
        // *&preserveData = 0xDEAD0002;
        //throw(&preserveData, &_TI1_AW4ErrorCode__);
        //}

        int size = TheDynamicMemoryAllocator->Get_Actual_Allocation_Size(sizeof(wchar_t) * chars_needed + sizeof(UnicodeStringData));
        UnicodeStringData *new_data = reinterpret_cast<UnicodeStringData *>(TheDynamicMemoryAllocator->Allocate_Bytes_No_Zero(size));

        new_data->RefCount = 1;
        new_data->NumCharsAllocated = (size - sizeof(UnicodeStringData)) / sizeof(wchar_t);
    #ifdef GAME_DEBUG_STRUCTS
        new_data->DebugPtr = new_data->Peek();
    #endif

        if ( Data != nullptr && keep_data ) {
            wcscpy(new_data->Peek(), Peek());
        } else {
            *new_data->Peek() = L'\0';
        }

        if ( str_to_cpy != nullptr ) {
            wcscpy(new_data->Peek(), str_to_cpy);
        }

        if ( str_to_cat != nullptr ) {
            wcscat(new_data->Peek(), str_to_cat);
        }

        Release_Buffer();
        Data = new_data;
    }
}

size_t UnicodeString::Get_Length() const
{
    if ( Data != nullptr ) {
        return wcslen(Data->Peek());
    }

    return 0;
}

void UnicodeString::Clear()
{
    Release_Buffer();
}

wchar_t UnicodeString::Get_Char(int index) const
{
    if ( Data != nullptr ) {
        return Data->Peek()[index];
    }

    return L'\0';
}

wchar_t const *UnicodeString::Str()
{
    static wchar_t const *TheNullChr = L"";

    if ( Data != nullptr ) {
        return Peek();
    }

    return TheNullChr;
}

wchar_t *UnicodeString::Get_Buffer_For_Read(int len)
{
    ASSERT_PRINT(len > 0, "No need to allocate 0 len strings.");

    //
    // 
    //
    Ensure_Unique_Buffer_Of_Size(len + 1, 0, 0, 0);

    return Peek();
}

void UnicodeString::Set(wchar_t const *s)
{
    if ( Data != nullptr || s != Data->Peek() ) {
        size_t len;

        if ( s && (len = wcslen(s) + 1, len != 1) ) {
            Ensure_Unique_Buffer_Of_Size(len, false, s, nullptr);
        } else {
            Release_Buffer();
        }
    }
}

void UnicodeString::Set(UnicodeString const &string)
{
    ScopedCriticalSectionClass cs(UnicodeStringCriticalSection);

    if ( &string != this ) {
        Release_Buffer();
        Data = string.Data;

        if ( string.Data != nullptr ) {
            ++Data->RefCount;
        }
    }
}

void UnicodeString::Translate(AsciiString const &string)
{
    Release_Buffer();

    int str_len = string.Get_Length();

    for ( int i = 0; i < str_len; ++i ) {
        wchar_t c;

        if ( string.Data != nullptr ) {
            c = string.Get_Char(i);
        } else {
            c = L'\0';
        }

        Concat(c);
    }
}

void UnicodeString::Concat(wchar_t c)
{
    wchar_t str[2];

    str[0] = c;
    str[1] = L'\0';
    Concat(str);
}

void UnicodeString::Concat(wchar_t *s)
{
    size_t len = wcslen(s);

    if ( len > 0 ) {
        if ( Data != nullptr ) {
            Ensure_Unique_Buffer_Of_Size(wcslen(Peek()) + len + 1, true, 0, s);
        } else {
            Set(s);
        }
    }
}

void UnicodeString::Trim()
{
    // No string, no Trim.
    if ( Data == nullptr ) {
        return;
    }

    wchar_t *str = Peek();

    // Find first none space in string if not the first.
    for ( char i = *str; i != '\0'; i = *(++str) ) {
        if ( !iswspace(i) ) {
            break;
        }
    }

    // If we had some spaces and moved Str, Set string to that position.
    if ( str != Peek() ) {
        Set(str);
    }

    // Oops, Set call broke the string.
    if ( Data == nullptr ) {
        return;
    }

    for ( int i = wcslen(Peek()) - 1; i >= 0; --i ) {
        if ( !iswspace(Get_Char(i)) ) {
            break;
        }

        Remove_Last_Char();
    }
}

void UnicodeString::To_Lower()
{
    wchar_t buf[MAX_FORMAT_BUF_LEN];

    if ( Data == nullptr ) {
        return;
    }

    wcscpy(buf, Peek());

    for ( wchar_t *c = buf; *c != L'\0'; ++c ) {
        *c = towlower(*c);
    }

    Set(buf);
}

void UnicodeString::Remove_Last_Char()
{
    if ( Data == nullptr ) {
        return;
    }

    int len = wcslen(Peek());

    if ( len > 0 ) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len] = L'\0';
    }
}

void UnicodeString::Format(wchar_t const *format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void UnicodeString::Format(UnicodeString format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void UnicodeString::Format_VA(wchar_t const *format, char *args)
{
    wchar_t buf[MAX_FORMAT_BUF_LEN];

    RELEASE_ASSERT(vsnwprintf(buf, sizeof(buf), format, args) > 0);

    Set(buf);
}

void UnicodeString::Format_VA(UnicodeString &format, char *args)
{
    wchar_t buf[MAX_FORMAT_BUF_LEN];

    RELEASE_ASSERT(vsnwprintf(buf, sizeof(buf), format.Str(), args) > 0);

    Set(buf);
}

bool UnicodeString::Next_Token(UnicodeString *tok, UnicodeString delims)
{
    if ( Data == nullptr ) {
        return false;
    }

    if ( *Peek() == L'\0' || this == tok ) {
        return false;
    }

    //
    // If no separators provided, default to white space.
    //
    if ( delims == nullptr ) {
        delims = L" \n\r\t";
    }
    
    wchar_t const *start = Str();

    size_t pos = wcscspn(Peek(), delims.Str());

    //
    // Check if the position of the next token is not the start of data anyway.
    //
    if ( &(Peek()[pos]) > Peek() ) {
        wchar_t *read_buffer = tok->Get_Buffer_For_Read(pos + 1);
        memcpy(read_buffer, Peek(), pos);
        read_buffer[pos] = L'\0';
        Set(&(Peek()[pos]));

        return true;
    }

    return false;
}
