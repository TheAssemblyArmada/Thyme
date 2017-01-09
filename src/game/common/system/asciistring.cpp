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

AsciiString const AsciiString::EmptyString(nullptr);

AsciiString::AsciiString() :
    Data(nullptr)
{
}

AsciiString::AsciiString(char const *s) :
    Data(nullptr)
{
    if ( s != nullptr ) {
        int len = (int)strlen(s);
        if ( len > 0 ) {
            Ensure_Unique_Buffer_Of_Size(len + 1, false, s);
        }
    }
}

AsciiString::AsciiString(AsciiString const &string) :
    Data(string.Data)
{
    if ( Data != nullptr ) {
    #ifdef COMPILER_MSVC
        InterlockedIncrement16((volatile short*)&string.Data->RefCount);
    #elif defined COMPILER_GNUC || defined COMPILER_CLANG
        __sync_add_and_fetch(&string.Data->RefCount, 1);
    #endif
    }
}

void AsciiString::Validate()
{
    //TODO, doesnt seem to be implimented anywhere? It is called though...
}

char *AsciiString::Peek() const
{
    DEBUG_ASSERT_PRINT(Data != nullptr, "null string ptr");
    
    return Data->Peek();
}


void AsciiString::Release_Buffer()
{
    if ( Data != nullptr ) {
    #ifdef COMPILER_MSVC
        InterlockedDecrement16((volatile short*)&Data->RefCount);
    #elif defined COMPILER_GNUC || defined COMPILER_CLANG
        __sync_sub_and_fetch(&Data->RefCount, 1);
    #endif
        if ( Data->RefCount == 0 ) {
            Free_Bytes();
        }
        
        Data = nullptr;
    }
}

void AsciiString::Ensure_Unique_Buffer_Of_Size(int chars_needed, bool keep_data, char const *str_to_cpy, char const *str_to_cat)
{
    if ( Data != nullptr && Data->RefCount == 1 && Data->NumCharsAllocated >= chars_needed ) {
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

        int size = TheDynamicMemoryAllocator->Get_Actual_Allocation_Size(chars_needed + sizeof(AsciiStringData));
        AsciiStringData *new_data = reinterpret_cast<AsciiStringData *>(TheDynamicMemoryAllocator->Allocate_Bytes_No_Zero(size));
        
        new_data->RefCount = 1;
        new_data->NumCharsAllocated = size - sizeof(AsciiStringData);
        //new_data->NumCharsAllocated = numCharsNeeded;
    #ifdef GAME_DEBUG_STRUCTS
        new_data->DebugPtr = new_data->Peek();
    #endif

        if ( Data != nullptr && keep_data ) {
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
        Data = new_data;
    }
}

int AsciiString::Get_Length() const
{
    if ( Data != nullptr ) {
        int len = strlen(Str());
        DEBUG_ASSERT_PRINT(len > 0, "length of string is less than or equal to 0.");
        
        return len;
    }
    
    return 0;
}

char AsciiString::Get_Char(int index) const
{
    DEBUG_ASSERT_PRINT(index >= 0, "bad index in getCharAt.");
    DEBUG_ASSERT_PRINT(strlen(Peek()) > 0, "strlen returned less than or equal to 0 in getCharAt.");
    
    return Peek()[index];
}

const char *AsciiString::Str() const
{
    static char const TheNullChr[4] = "";

    if ( Data != nullptr ) {
        return Peek();
    }
    
    return TheNullChr;
}

char *AsciiString::Get_Buffer_For_Read(int len)
{
    DEBUG_ASSERT_PRINT(len > 0, "No need to allocate 0 len strings.");
    
    //
    // Generate buffer sufficient to read requested size into.
    //
    Ensure_Unique_Buffer_Of_Size(len + 1, 0, 0, 0);

    return Peek();
}

void AsciiString::Set(char const *s)
{
    if ( Data != nullptr || s != Data->Peek() ) {
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
        Data = string.Data;
        
        if ( string.Data != nullptr ) {
        #ifdef COMPILER_MSVC
            InterlockedIncrement16((volatile short*)&string.Data->RefCount);
        #elif defined COMPILER_GNUC || defined COMPILER_CLANG
            __sync_add_and_fetch(&string.Data->RefCount, 1);
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
        /*if ( v4 < 8 || (!stringSrc.Data ? (v6 = 0) : (v5 = stringSrc.Peek(), v6 = wcslen(v5)), v3 >= v6) )
        {
            if ( `UnicodeString::Get_Char'::`14'::allowCrash )
            {
                TheCurrentAllowCrashPtr = &`UnicodeString::Get_Char'::`14'::allowCrash;
                DebugCrash(aBadIndexInGetch);
                TheCurrentAllowCrashPtr = 0;
            }
        }*/
        wchar_t c;

        if ( string.Data != nullptr ) {
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

void AsciiString::Concat(char const *s)
{
    int len = strlen(s);

    if ( len > 0 ) {
        if ( Data != nullptr ) {
            Ensure_Unique_Buffer_Of_Size(strlen(Peek()) + len + 1, true, 0, s);
        } else {
            Set(s);
        }
    }
}

void AsciiString::Trim()
{
    // No string, no Trim.
    if ( Data == nullptr ) {
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
    if ( Data == nullptr ) {
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

    if ( Data == nullptr ) {
        return;
    }

    strcpy(buf, Peek());

    for ( char *c = buf; *c != '\0'; ++c ) {
        *c = tolower(*c);
    }

    Set(buf);
}

void AsciiString::Remove_Last_Char()
{
    if ( Data == nullptr ) {
        return;
    }

    int len = strlen(Peek());

    if ( len > 0 ) {
        Ensure_Unique_Buffer_Of_Size(len + 1, true);
        Peek()[len] = '\0';
    }
}

void AsciiString::Format(char const *format, ...)
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

void AsciiString::Format_VA(char const *format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];

    RELEASE_ASSERT(vsnprintf(buf, sizeof(buf), format, args) > 0);

    Set(buf);
}

void AsciiString::Format_VA(AsciiString &format, va_list args)
{
    char buf[MAX_FORMAT_BUF_LEN];

    RELEASE_ASSERT(vsnprintf(buf, sizeof(buf), format.Str(), args) > 0);
    
    Set(buf);
}

bool AsciiString::Starts_With(char const *p)
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = Data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncmp(Peek(), p, thatlen) == 0;
}

bool AsciiString::Ends_With(char const *p)
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = Data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

bool AsciiString::Starts_With_No_Case(char const *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = Data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncasecmp(Peek(), p, thatlen) == 0;
}

bool AsciiString::Ends_With_No_Case(char const *p) const
{
    if ( *p == '\0' ) {
        return true;
    }
    
    int thislen = Data != nullptr ? strlen(Peek()) : 0;
    int thatlen = strlen(p);
    
    if ( thislen < thatlen ) {
        return false;
    }
    
    return strncasecmp(Peek() + thislen - thatlen, p, thatlen) == 0;
}

bool AsciiString::Next_Token(AsciiString *tok, char const *delims)
{
    if ( Data == nullptr ) {
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
