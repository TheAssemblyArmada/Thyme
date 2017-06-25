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
#include "stringex.h"
#include <stdio.h>

//#ifndef vsnwprintf
//#define vsnwprintf _vsnwprintf
//#endif

UnicodeString const UnicodeString::EmptyString;

UnicodeString::UnicodeString() :
    m_data(nullptr)
{
}

UnicodeString::UnicodeString(wchar_t const *s) :
    m_data(nullptr)
{
    if ( s != nullptr ) {
        size_t len = wcslen(s);

        if ( len > 0 ) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s, nullptr);
        }
    }
}

UnicodeString::UnicodeString(char16_t const *s) :
    m_data(nullptr)
{
    if ( s != nullptr ) {
        size_t len = strlen16(s);

        if ( len > 0 ) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, nullptr, nullptr);
        }

        Char16_To_WChar(Peek(), s);
    }
}

UnicodeString::UnicodeString(UnicodeString const &string) :
    m_data(string.m_data)
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if ( m_data != nullptr ) {
        ++m_data->ref_count;
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
    ASSERT_PRINT(m_data != nullptr, "null string ptr");

    //
    // Actual string data is stored immediately after the UnicodeStringData header.
    //
    return m_data->Peek();
}

void UnicodeString::Release_Buffer()
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if ( m_data != nullptr ) {
        if ( --m_data->ref_count == 0 ) {
            g_dynamicMemoryAllocator->Free_Bytes(m_data);
        }

        m_data = nullptr;
    }
}

void UnicodeString::Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data, wchar_t const *str_to_cpy, wchar_t const *str_to_cat)
{
    if ( m_data != nullptr && m_data->ref_count == 1 && m_data->num_chars_allocated >= chars_needed ) {
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

        int size = g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(sizeof(wchar_t) * chars_needed + sizeof(UnicodeStringData));
        UnicodeStringData *new_data = reinterpret_cast<UnicodeStringData *>(g_dynamicMemoryAllocator->Allocate_Bytes_No_Zero(size));

        new_data->ref_count = 1;
        new_data->num_chars_allocated = (size - sizeof(UnicodeStringData)) / sizeof(wchar_t);
    #ifdef GAME_DEBUG_STRUCTS
        new_data->debug_ptr = new_data->Peek();
    #endif

        if ( m_data != nullptr && keep_data ) {
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
        m_data = new_data;
    }
}

int UnicodeString::Get_Length() const
{
    if ( m_data != nullptr ) {
        return wcslen(m_data->Peek());
    }

    return 0;
}

void UnicodeString::Clear()
{
    Release_Buffer();
}

wchar_t UnicodeString::Get_Char(int index) const
{
    if ( m_data != nullptr ) {
        return m_data->Peek()[index];
    }

    return L'\0';
}

wchar_t const *UnicodeString::Str() const
{
    static wchar_t const *TheNullChr = L"";

    if ( m_data != nullptr ) {
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
    Ensure_Unique_Buffer_Of_Size(len + 1, false, nullptr, nullptr);

    return Peek();
}

void UnicodeString::Set(wchar_t const *s)
{
    if ( m_data != nullptr || s != m_data->Peek() ) {
        size_t len;

        if ( s && (len = wcslen(s) + 1, len != 1) ) {
            Ensure_Unique_Buffer_Of_Size(len, false, s, nullptr);
        } else {
            Release_Buffer();
        }
    }
}

void UnicodeString::Set(char16_t const *s)
{
    size_t len;

    if ( s && (len = strlen16(s) + 1, len != 1) ) {
        Ensure_Unique_Buffer_Of_Size(len, false, nullptr, nullptr);

        Char16_To_WChar(Peek(), s);
    }
}

void UnicodeString::Set(UnicodeString const &string)
{
    ScopedCriticalSectionClass cs(g_unicodeStringCriticalSection);

    if ( &string != this ) {
        Release_Buffer();
        m_data = string.m_data;

        if ( string.m_data != nullptr ) {
            ++m_data->ref_count;
        }
    }
}

void UnicodeString::Translate(AsciiString const &string)
{
    Release_Buffer();

    int str_len = string.Get_Length();

    for ( int i = 0; i < str_len; ++i ) {
        wchar_t c;

        if ( string.m_data != nullptr ) {
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

void UnicodeString::Concat(wchar_t const *s)
{
    size_t len = wcslen(s);

    if ( len > 0 ) {
        if ( m_data != nullptr ) {
            Ensure_Unique_Buffer_Of_Size(wcslen(Peek()) + len + 1, true, 0, s);
        } else {
            Set(s);
        }
    }
}

void UnicodeString::Trim()
{
    // No string, no Trim.
    if ( m_data == nullptr ) {
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
    if ( m_data == nullptr ) {
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

    if ( m_data == nullptr ) {
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
    if ( m_data == nullptr ) {
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

    ASSERT_THROW_PRINT(vswprintf(buf, sizeof(buf), format, args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

void UnicodeString::Format_VA(UnicodeString &format, char *args)
{
    wchar_t buf[MAX_FORMAT_BUF_LEN];

    ASSERT_THROW_PRINT(vswprintf(buf, sizeof(buf), format.Str(), args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

bool UnicodeString::Next_Token(UnicodeString *tok, UnicodeString delims)
{
    if ( m_data == nullptr ) {
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

// Naive copy of char16_t to wchar_t assuming only BMP characters
wchar_t *UnicodeString::Char16_To_WChar(wchar_t *dst, char16_t const *src)
{
    wchar_t *tmp = dst;

    while ( (*tmp++ = *src++) != 0 ) {

    }

    return dst;
}
