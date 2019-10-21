/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Parser for SAGE engine configuration files.
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
#include "asciistring.h"
#include <captnassert.h>

class File;
class Xfer;
class INI;

extern const float _SECONDS_PER_LOGICFRAME_REAL_74;
extern const float _ANGLE_MULTIPLIER;

enum INILoadType
{
    INI_LOAD_INVALID,
    INI_LOAD_OVERWRITE,
    INI_LOAD_CREATE_OVERRIDES,
    INI_LOAD_UNK,
};

// Function pointer type for the field parser functions
typedef void (*inifieldparse_t)(INI *, void *, void *, const void *);
typedef void (*iniblockparse_t)(INI *);

struct LookupListRec
{
    const char *name;
    int value;
};

struct BlockParse
{
    const char *token;
    iniblockparse_t parse_func;
};

struct FieldParse
{
    const char *token;
    inifieldparse_t parse_func;
    const void *user_data;
    size_t offset;
};

struct MultiIniFieldParse
{
    MultiIniFieldParse() : count(0) { memset(extra_offsets, 0, sizeof(extra_offsets)); }

    void Add(FieldParse *field_parse, unsigned int extra_offset)
    {
        captain_assert(count < MAX_MULTI_FIELDS, 0xDEAD0001, "Cannot add additional field parsers, max exceeded.");

        field_parsers[count] = field_parse;
        extra_offsets[count] = extra_offset;
        ++count;
    }

    enum
    {
        MAX_MULTI_FIELDS = 16
    };

    FieldParse *field_parsers[MAX_MULTI_FIELDS];
    unsigned int extra_offsets[MAX_MULTI_FIELDS];
    int count;
};

class INI
{
    ALLOW_HOOKING
public:
    enum
    {
        MAX_LINE_LENGTH = 1028,
        MAX_BUFFER_SIZE = 8192,
    };

    INI();
    ~INI();

    void Load(Utf8String filename, INILoadType type, Xfer *xfer);
    void Load_Directory(Utf8String dir, bool search_subdirs, INILoadType type, Xfer *xfer);

    void Init_From_INI(void *what, FieldParse *parse_table);
    void Init_From_INI_Multi(void *what, const MultiIniFieldParse &parse_table_list);
    void Init_From_INI_Multi_Proc(void *what, void (*proc)(MultiIniFieldParse *));

    const char *Get_Next_Token_Or_Null(const char *seps = nullptr);
    const char *Get_Next_Token(const char *seps = nullptr);
    const char *Get_Next_Sub_Token(const char *expected);
    Utf8String Get_Next_Ascii_String();
    Utf8String Get_Next_Quoted_Ascii_String();
    Utf8String Get_Filename() { return m_fileName; }
    INILoadType Get_Load_Type() { return m_loadType; }
    int Get_Line_Number() { return m_lineNumber; }

    // Scan functions
    static int Scan_Science(const char *token);
    static float Scan_PercentToReal(const char *token);
    static float Scan_Real(const char *token);
    static unsigned int Scan_UnsignedInt(const char *token);
    static int Scan_Int(const char *token);
    static bool Scan_Bool(const char *token);
    static int Scan_IndexList(const char *token, const char *const *list);
    static int Scan_LookupList(const char *token, const LookupListRec *list);

    // Field parsing functions
    static void Parse_Bool(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Byte(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Int(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Unsigned(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Positive_None_Zero_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Percent_To_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Angle_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Angular_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_AsciiString(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Quoted_AsciiString(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_AsciiString_Vector_Append(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_RGB_Color(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_RGBA_Color_Int(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Color_Int(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Coord2D(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Coord3D(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_ICoord2D(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_ICoord3D(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Index_List(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Duration_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Duration_Int(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Acceleration_Real(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Bit_In_Int32(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_And_Translate_Label(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Bitstring32(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Bitstring64(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Speaker_Type(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Audio_Event_RTS(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Science_Vector(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Sounds_Vector(INI *ini, void *formal, void *store, const void *user_data);

    // Block Parser functions
    static void Parse_Draw_Group_Info(INI *ini);

private:
    void Read_Line();
    void Prep_File(Utf8String filename, INILoadType type);
    void Unprep_File();

    File *m_backingFile;
    char m_buffer[MAX_BUFFER_SIZE];
    int m_bufferReadPos;
    int m_bufferData;
    Utf8String m_fileName;
    INILoadType m_loadType;
    int m_lineNumber;
    char m_currentBlock[MAX_LINE_LENGTH];
    char m_blockEnd;
    const char *m_seps;
    const char *m_sepsPercent;
    const char *m_sepsColon;
    const char *m_sepsQuote;
    const char *m_endToken;
    bool m_endOfFile;
};

#ifdef GAME_DLL
#include "hookcrt.h"
extern Xfer *&g_sXfer;
#else
extern Xfer *g_sXfer;
#endif

// Functions for inlining, neater than including in class declaration
inline const char *INI::Get_Next_Token_Or_Null(const char *seps)
{
    return strtok(0, seps != nullptr ? seps : m_seps);
}

inline const char *INI::Get_Next_Token(const char *seps)
{
    char *ret = strtok(0, seps != nullptr ? seps : m_seps);
    captain_assert(
        ret != nullptr, 0xDEAD0006, "Expected further tokens in '%s', line %d", m_fileName.Str(), m_lineNumber);

    return ret;
}

inline const char *INI::Get_Next_Sub_Token(const char *expected)
{
    const char *next = Get_Next_Token(m_sepsColon);
    captain_dbgassert(strcasecmp(next, expected) == 0, "Did not get expected token");
    return Get_Next_Token(m_sepsColon);
}

inline Utf8String INI::Get_Next_Ascii_String()
{
    static char _buffer[MAX_LINE_LENGTH];
    Utf8String next;

    const char *token = Get_Next_Token_Or_Null();

    if (token != nullptr) {
        if (*token == '"') { // Handle quoted tokens.
            _buffer[0] = '\0';

            if (strlen(token) > 1) {
                strlcpy(_buffer, token + 1, sizeof(_buffer));
            }

            const char *next_token = Get_Next_Token_Or_Null(m_sepsQuote);

            if (next_token != nullptr) { // If we have another token, parse that too.
                if (strlen(next_token) > 1 && next_token[1] != '\t') {
                    strlcat(_buffer, " ", sizeof(_buffer));
                }

                strlcat(_buffer, next_token, sizeof(_buffer));
            } else { // Handle trailing ".
                size_t length = strlen(_buffer);

                if (length > 1 && _buffer[length - 1] == '"') {
                    _buffer[length - 1] = '\0';
                }
            }

            next = _buffer;
        } else {
            next = token;
        }
    }

    return next;
}