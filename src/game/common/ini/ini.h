////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: INI.H
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Parser for SAGE engine configuration files.
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

#ifndef _INI_H_
#define _INI_H_

#include "asciistring.h"
#include "gamedebug.h"
#include "hookcrt.h"
#include "hooker.h"

class File;
class Xfer;
class INI;

#define SXfer (Make_Global<Xfer*>(0x00A2A6B8))

enum INILoadType
{
    INI_LOAD_INVALID          = 0,
    INI_LOAD_OVERWRITE        = 1,
    INI_LOAD_CREATE_OVERRIDES = 2,
};

// Function pointer type for the field parser functions
typedef void(*inifieldparse_t)(INI *, void *, void *, void const *);
typedef void(__cdecl *iniblockparse_t)(INI *);

struct LookupListRec
{
    char const *Name;
    int Value;
};

struct BlockParse
{
    char const *Token;
    iniblockparse_t ParseFunc;
};

struct FieldParse
{
    char const *Token;
    inifieldparse_t ParseFunc;
    void const *UserData;
    int Offset;
};

struct MultiIniFieldParse
{
    MultiIniFieldParse() : Count(0) { memset(ExtraOffsets, 0, sizeof(ExtraOffsets)); }

    void Add(FieldParse *field_parse, unsigned int extra_offset);

    enum { MAX_MULTI_FIELDS = 16 };

    FieldParse *FieldParsers[MAX_MULTI_FIELDS];
    unsigned int ExtraOffsets[MAX_MULTI_FIELDS];
    int Count;
};

class INI
{
    public:
        enum
        {
            MAX_LINE_LENGTH = 1028,
            MAX_BUFFER_SIZE = 8192,
        };

        INI();
        ~INI();

        void Load(AsciiString filename, INILoadType type, Xfer *xfer);
        void Load_Directory(AsciiString dir, bool search_subdirs, INILoadType type, Xfer *xfer);

        void Prep_File(AsciiString filename, INILoadType type);
        void Unprep_File();

        void Init_From_INI(void *what, FieldParse *parse_table);
        void Init_From_INI_Multi(void *what, MultiIniFieldParse const &parse_table_list);
        void Init_From_INI_Multi_Proc(void *what, void (*proc)(MultiIniFieldParse *));

        void Read_Line();

        char *Get_Next_Token_Or_Null(char const *seps = nullptr);
        char *Get_Next_Token(char const *seps = nullptr);
        char *Get_Next_Sub_Token(char const *expected);
        AsciiString Get_Next_Ascii_String();
        AsciiString Get_Filename() { return FileName; }

        int Scan_Science(char const *token);
        float Scan_PercentToReal(char const *token);
        float Scan_Real(char const *token);
        uint32_t Scan_UnsignedInt(char const *token);
        int32_t Scan_Int(char const *token);
        bool Scan_Bool(char const *token);
        int Scan_IndexList(char const *token, char const* const* list);
        int Scan_LookupList(char const *token, LookupListRec const *list);

    private:
        File *BackingFile;
        char Buffer[MAX_BUFFER_SIZE];
        int BufferReadPos;
        int BufferData;
        AsciiString FileName;
        INILoadType LoadType;
        int LineNumber;
        char CurrentBlock[MAX_LINE_LENGTH];
        char BlockEnd;
        char const *Seps;
        char const *SepsPercent;
        char const *SepsColon;
        char const *SepsQuote;
        char const *EndToken;
        bool EndOfFile;

        //static Xfer *SXfer;
};

// Functions for inlining, neater than including in class declaration
inline char *INI::Get_Next_Token_Or_Null(char const *seps)
{
    return crt_strtok(0, seps != nullptr ? seps : Seps);
}

inline char *INI::Get_Next_Token(char const *seps)
{
    DEBUG_LOG("INI Getting next token from file %s.\n", FileName.Str());
    char *ret = crt_strtok(0, seps != nullptr ? seps : Seps);
    ASSERT_THROW_PRINT(ret != nullptr, 0xDEAD0006, "Expected further tokens\n");

    return ret;
}

inline char *INI::Get_Next_Sub_Token(char const *expected)
{
    ASSERT_PRINT(strcasecmp(Get_Next_Token(SepsColon), expected) == 0, "Did not get expected token\n" )
    return Get_Next_Token(SepsColon);
}

inline AsciiString INI::Get_Next_Ascii_String()
{
    static char _buffer[1024];
    AsciiString next;

    char *token = Get_Next_Token_Or_Null();

    if ( token != nullptr ) {
        if ( *token == '"' ) {
            if ( strlen(token) > 1 ) {
                strcpy(_buffer, token + 1);
            }

            char *ntoken = Get_Next_Token(SepsQuote);

            if ( strlen(ntoken) > 1 && ntoken[1] != '\t' ) {
                strcat(_buffer, " ");
            }

            strcat(_buffer, ntoken);
            next = _buffer;
        } else {
            next = token;
        }
    }

    return next;
}

#endif // _INI_H_
