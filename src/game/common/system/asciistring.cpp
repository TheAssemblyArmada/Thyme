////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: ASCIISTRING.CPP
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
#include "asciistring.h"
#include "unicodestring.h"
#include "gamedebug.h"
#include <cctype>
#include <cstdio>

AsciiString const AsciiString::s_emptyString(nullptr);

AsciiString::AsciiString() :
    m_data(nullptr)
{
}

AsciiString::AsciiString(const char *s) :
    m_data(nullptr)
{
    if ( s != nullptr ) {
        int len = (int)strlen(s);
        if ( len > 0 ) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s);
        }
    }
}

AsciiString::AsciiString(AsciiString const &string) :
    m_data(string.m_data)
{
    if ( m_data != nullptr ) {
    #ifdef COMPILER_MSVC
        InterlockedIncrement16((volatile short*)&string.m_data->ref_count);
    #elif defined COMPILER_GNUC || defined COMPILER_CLANG
        __sync_add_and_fetch(&string.m_data->ref_count, 1);
    #endif
    }
}

void AsciiString::Validate()
{
    //TODO, doesnt seem to be implimented anywhere? It is called though...
}

char *AsciiString::Peek() const
{
    ASSERT_PRINT(m_data != nullptr, "null string ptr");
    
    return m_data->Peek();
}


void AsciiString::Release_Buffer()
{
    if ( m_data != nullptr ) {
    #ifdef COMPILER_MSVC
        InterlockedDecrement16((volatile short*)&m_data->ref_count);
    #elif defined COMPILER_GNUC || defined COMPILER_CLANG
        __sync_sub_and_fetch(&m_data->ref_count, 1);
    #endif
        if ( m_data->ref_count == 0 ) {
            g_dynamicMemoryAllocator->Free_Bytes(m_data);
        }
        
        m_data = nullptr;
    }
}

void AsciiString::Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data, const char *str_to_cpy, const char *str_to_cat)
{
    if ( m_data != nullptr && m_data->ref_count == 1 && m_data->num_chars_allocated >= chars_needed ) {
        if ( str_to_cpy != nullptr ) {
            strcpy(Peek(), str_to_cpy);
        }

        if ( str_to_cat != nullptr ) {
            strcpy(Peek() + strlen(Peek()), str_to_cat);
        }

    } else {

        //this block would have been a macro like DEBUG_CRASH(numCharsNeeded + 8 > MAX_LEN, THROW_02); (see cl_debug.h)
        //if ( numCharsNeeded + 8 > MAX_LEN ) {
            // *&preserveData = 0xDEAD0002;
            //throw(&preserveData, &_TI1_AW4ErrorCode__);
        //}

        int size = g_dynamicMemoryAllocator->Get_Actual_Allocation_Size(chars_needed + sizeof(AsciiStringData));
        AsciiStringData *new_data = reinterpret_cast<AsciiStringData *>(g_dynamicMemoryAllocator->Allocate_Bytes_No_Zero(size));
        
        new_data->ref_count = 1;
        new_data->num_chars_allocated = size - sizeof(AsciiStringData);
        //new_data->num_chars_allocated = numCharsNeeded;
    #ifdef GAME_DEBUG_STRUCTS
        new_data->debug_ptr = new_data->Peek();
    #endif

        if ( m_data != nullptr && keep_data ) {
            strcpy(new_data->Peek(), Peek());
        } else {
            *new_data->Peek() = '\0';
        }

        if ( str_to_cpy != nullptr ) {
            strcpy(new_data->Peek(), str_to_cpy);
        }

        if ( str_to_cat != nullptr ) {
            strcat(new_data->Peek(), str_to_cat);
        }

        Release_Buffer();
        m_data = new_data;
    }
}

int AsciiString::Get_Length() const
{
    if ( m_data != nullptr ) {
        int len = strlen(Str());
        ASSERT_PRINT(len > 0, "length of string is less than or equal to 0.");
        
        return len;
    }
    
    return 0;
}

char AsciiString::Get_Char(int index) const
{
    ASSERT_PRINT(index >= 0, "bad index in getCharAt.");
    ASSERT_PRINT(Get_Length() > 0, "strlen returned less than or equal to 0 in getCharAt.");
    
    return Peek()[index];
}

const char *AsciiString::Str() const
{
    static char const TheNullChr[4] = "";

    if ( m_data != nullptr ) {
        return Peek();
    }
    
    return TheNullChr;
}

char *AsciiString::Get_Buffer_For_Read(int len)
{
    ASSERT_PRINT(len > 0, "No need to allocate 0 len strings.");
    
    //
    // Generate buffer sufficient to read requested size into.
    //
    Ensure_Unique_Buffer_Of_Size(len + 1, 0, 0, 0);

    return Peek();
}

void AsciiString::Set(const char *s)
{
    if ( m_data == nullptr || s != m_data->Peek() ) {
        size_t len;
        
        if ( s && (len = strlen(s) + 1, len != 1) ) {
            Ensure_Unique_Buffer_Of_Size(len, false, s, nullptr);
        } else {
            Release_Buffer();
        }
    }
}

void AsciiString::Set(AsciiString const &string)
{
    if ( &string != this ) {
        Release_Buffer();
        m_data = string.m_data;
        
        if ( string.m_data != nullptr ) {
        #ifdef COMPILER_MSVC
            InterlockedIncrement16((volatile short*)&string.m_data->ref_count);
        #elif defined COMPILER_GNUC || defined COMPILER_CLANG
            __sync_add_and_fetch(&string.m_data->ref_count, 1);
        #endif
        }
    }
}

void AsciiString::Translate(UnicodeString const &string)
{
    Release_Buffer();
    
    int str_len = string.Get_Length();

    for ( int i = 0; i < str_len; ++i ) {
        //This is a debug assert from the look of it.
        /*if ( v4 < 8 || (!stringSrc.m_data ? (v6 = 0) : (v5 = stringSrc.Peek(), v6 = wcslen(v5)), v3 >= v6) )
        {
            if ( `UnicodeString::Get_Char'::`14'::allowCrash )
            {
                TheCurrentAllowCrashPtr = &`UnicodeString::Get_Char'::`14'::allowCrash;
                DebugCrash(aBadIndexInGetch);
                TheCurrentAllowCrashPtr = 0;
            }
        }*/
        wchar_t c;

        if ( string.m_data != nullptr ) {
            c = string.Get_Char(i);
        } else {
            c = L'\0';
        }
        
        // null out the second byte so Concat only concatenates the first byte.
        // prevents issues if unicode string contains none ascii chars.
        // This will have endian issues on big endian.
        c &= 0xFF;
        Concat(reinterpret_cast<char *>(&c));
    }
}

void AsciiString::Concat(char c)
{
    char str[2];
 
    str[0] = c;
    str[1] = '\0';
    Concat(str);
}

void AsciiString::Concat(const char *s)
{
    int len = strlen(s);

    if ( len > 0 ) {
        if ( m_data != nullptr ) {
            Ensure_Unique_Buffer_Of_Size(strlen(Peek()) + len + 1, true, 0, s);
        } else {
            Set(s);
        }
    }
}

void AsciiString::Trim()
{
    // No string, no Trim.
    if ( m_data == nullptr ) {
        return;
    }

    char *str = Peek();

    // Find first none space in string if not the first.
    for ( char i = *str; i != '\0'; i = *(++str) ) {
        if ( !isspace(i) ) {
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

    for ( int i = strlen(Peek()) - 1; i >= 0; --i ) {
        if ( !isspace(Get_Char(i)) ) {
            break;
        }

        Remove_Last_Char();
    }
}

void AsciiString::To_Lower()
{
    char buf[MAX_FORMAT_BUF_LEN];

    if ( m_data == nullptr ) {
        return;
    }

    strcpy(buf, Peek());

    for ( char *c = buf; *c != '\0'; ++c ) {
        *c = tolower(*c);
    }

    Set(buf);
}

// Sanitize the path to use all forward slashes for cross platform
// compat. Ideally we call this immediately on reading a path from the OS
// or config file and everything else assumes it, but until we control all
// path inputs, we need to do this everywhere paths are compared.
void AsciiString::Fix_Path()
{
    char buf[MAX_FORMAT_BUF_LEN];

    if ( m_data == nullptr ) {
        return;
    }

    strcpy(buf, Peek());

    for ( char *c = buf; *c != '\0'; ++c ) {
        if ( *c == '\\' ) {
            *c = '/';
        }
    }

    Set(buf);
}

void AsciiString::Remove_Last_Char()
{
    if ( m_data == nullptr ) {
        return;
    }

    int len = strlen(Peek());

    if ( len > 0 ) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len - 1] = '\0';
    }
}

void AsciiString::Format(const char *format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void AsciiString::Format(AsciiString format, ...)
{
    va_list va;

    va_start(va, format);
    Format_VA(format, va);
}

void AsciiString::Format_VA(const char *format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];

    ASSERT_THROW_PRINT(vsnprintf(buf, sizeof(buf), format, args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

void AsciiString::Format_VA(AsciiString &format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];

    ASSERT_THROW_PRINT(vsnprintf(buf, sizeof(buf), format.Str(), args) > 0, 0xDEAD0002, "Unable to format buffer");

    Set(buf);
}

bool AsciiString::Starts_With(const char *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = m_data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncmp(Peek(), p, thatlen) == 0;
}

bool AsciiString::Ends_With(const char *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = m_data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

bool AsciiString::Starts_With_No_Case(const char *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = m_data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncasecmp(Peek(), p, thatlen) == 0;
}

bool AsciiString::Ends_With_No_Case(const char *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = m_data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncasecmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

bool AsciiString::Next_Token(AsciiString *tok, const char *delims)
{
    if ( m_data == nullptr ) {
        return false;
    }
    
    if ( *Peek() == '\0' || this == tok ) {
        return false;
    }
    
    //
    // If no separators provided, default to white space.
    //
    if ( delims == nullptr ) {
        delims = " \n\r\t";
    }
    
    char *start = Peek();
    
    //
    // Find next instance of token or end of string
    //
    for ( char c = *start; c != '\0'; c = *(++start) ) {
        if ( strchr(delims, c) == nullptr ) {
            break;
        }
    }
    
    if ( *start == '\0' ) {
        Release_Buffer();
        tok->Release_Buffer();
        
        return false;
    }
    
    char *end = start;
    
    //
    // Find next instance of token or end of string.
    //
    for ( char c = *end; c != '\0'; c = *(++end) ) {
        if ( strchr(delims, c) != nullptr ) {
            break;
        }
    }
    
    if ( end <= start ) {
        Release_Buffer();
        tok->Release_Buffer();
        
        return false;
    }
    
    //
    // Copy found region into provided AsciiString, then move this string
    // to start of next section.
    //
    char *tokstr = tok->Get_Buffer_For_Read(end - start + 1);
    memcpy(tokstr, start, end - start);
    tokstr[end - start] = '\0';
    Set(end);
    
    return true;
}

#ifdef GAME_DEBUG
void AsciiString::Debug_Ignore_Leaks()
{
    //TODO, doesnt seem to be implimented anywhere? It is called though...
}
#endif // GAME_DEBUG
