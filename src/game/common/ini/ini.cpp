////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: INI.CPP
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
#include "ini.h"
#include "color.h"
#include "coord.h"
#include "file.h"
#include "filesystem.h"
#include "gamedebug.h"
#include "minmax.h"
#include "xfer.h"
#include <cctype>
#include <cstdio>
#include <math.h>

const float _SECONDS_PER_LOGICFRAME_REAL_74 = 1.0f / 30.0f;
const float _ANGLE_MULTIPLIER = 0.0174532925f;
const float _DURATION_MULT = 0.029999999f;

// Parsing tables, remove hook when table can be populated correctly
// Can use Make_Function_Ptr for unimplemented.
#if 0
BlockParse TheTypeTable[] =
{
    { "AIData", &INI::parseAIDataDefinition },
    { "Animation", &INI::parseAnim2DDefinition },
    { "Armor", &INI::parseArmorDefinition },
    { "AudioEvent", &INI::parseAudioEventDefinition },
    { "AudioSettings", &INI::parseAudioSettingsDefinition },
    { "Bridge", &INI::parseTerrainBridgeDefinition },
    { "Campaign", &INI::parseCampaignDefinition },
    { "ChallengeGenerals", &INI::parseChallengeModeDefinition },
    { "CommandButton", &INI::parseCommandButtonDefinition },
    { "CommandMap", &INI::parseMetaMapDefinition },
    { "CommandSet", &INI::parseCommandSetDefinition },
    { "ControlBarScheme", &INI::parseControlBarSchemeDefinition },
    { "ControlBarResizer", &INI::parseControlBarResizerDefinition },
    { "CrateData", &INI::parseCrateTemplateDefinition },
    { "Credits", &INI::parseCredits },
    { "WindowTransition", &INI::parseWindowTransitions },
    { "DamageFX", &INI::parseDamageFXDefinition },
    { "DialogEvent", &INI::parseDialogDefinition },
    { "DrawGroupInfo", &INI::parseDrawGroupNumberDefinition },
    { "EvaEvent", &INI::parseEvaEvent },
    { "FXList", &INI::parseFXListDefinition },
    { "GameData", &INI::parseGameDataDefinition },
    { "InGameUI", &INI::parseInGameUIDefinition },
    { "Locomotor", &INI::parseLocomotorTemplateDefinition },
    { "Language", &INI::parseLanguageDefinition },
    { "MapCache", &INI::parseMapCacheDefinition },
    { "MapData", &INI::parseMapDataDefinition },
    { "MappedImage", &INI::parseMappedImageDefinition },
    { "MiscAudio", &INI::parseMiscAudio },
    { "Mouse", &INI::parseMouseDefinition },
    { "MouseCursor", &INI::parseMouseCursorDefinition },
    { "MultiplayerColor", &INI::parseMultiplayerColorDefinition },
    { "MultiplayerStartingMoneyChoice", &INI::parseMultiplayerStartingMoneyChoiceDefinition },
    { "OnlineChatColors", &INI::parseOnlineChatColorDefinition },
    { "MultiplayerSettings", &INI::parseMultiplayerSettingsDefinition },
    { "MusicTrack", &INI::parseMusicTrackDefinition },
    { "Object", &INI::parseObjectDefinition },
    { "ObjectCreationList", &INI::parseObjectCreationListDefinition },
    { "ObjectReskin", &INI::parseObjectReskinDefinition },
    { "ParticleSystem", &INI::parseParticleSystemDefinition },
    { "PlayerTemplate", &INI::parsePlayerTemplateDefinition },
    { "Road", &INI::parseTerrainRoadDefinition },
    { "Science", &INI::parseScienceDefinition },
    { "Rank", &INI::parseRankDefinition },
    { "SpecialPower", &INI::parseSpecialPowerDefinition },
    { "ShellMenuScheme", &INI::parseShellMenuSchemeDefinition },
    { "Terrain", &INI::parseTerrainDefinition },
    { "Upgrade", &INI::parseUpgradeDefinition },
    { "Video", &INI::parseVideoDefinition },
    { "WaterSet", &INI::parseWaterSettingDefinition },
    { "WaterTransparency", &INI::parseWaterTransparencyDefinition },
    { "Weather", &INI::parseWeatherDefinition },
    { "Weapon", &INI::parseWeaponTemplateDefinition },
    { "WebpageURL", &INI::parseWebpageURL },
    { "HeaderTemplate", &INI::parseHeaderTemplateDefinition },
    { "StaticGameLOD", &INI::parseStaticGameLODDefinition },
    { "DynamicGameLOD", &INI::parseDynamicGameLODDefinition },
    { "LODPreset", &INI::parseLODPreset },
    { "BenchProfile", &INI::parseBenchProfile },
    { "ReallyLowMHz", &parseReallyLowMHz },
    { "ScriptAction", &ScriptEngine::parseScriptAction },
    { "ScriptCondition", &ScriptEngine::parseScriptCondition },
    { nullptr, nullptr }
};
#else
#define TheTypeTable (Make_Global<BlockParse>(0x0093B670))
#endif


// Helper function for Load
inline iniblockparse_t Find_Block_Parse(char const *token)
{
    // Iterate over the TypeTable to identify correct parsing function.
    for ( BlockParse *block = &TheTypeTable; block->Token != nullptr; ++block ) {
        if ( strcmp(block->Token, token) == 0 ) {
            return block->ParseFunc;
        }
    }

    return nullptr;
}

// Helper function for Init_From_INI_Multi
inline inifieldparse_t Find_Field_Parse(FieldParse *table, char const *token, int &offset, void const *&data)
{
    FieldParse *tblptr;

    // Search the list for a matching FieldParse struct.
    for ( tblptr = table; tblptr->Token != nullptr; ++tblptr ) {
        // If found, return the data and associated function.
        if ( strcmp(tblptr->Token, token) == 0 ) {
            offset = tblptr->Offset;
            data = tblptr->UserData;

            return tblptr->ParseFunc;
        }
    }

    // Didn't find matching token, but null token entry has a function
    if ( tblptr->ParseFunc != nullptr ) {
        offset = tblptr->Offset;
        data = token;

        return tblptr->ParseFunc;
    }

    // Didn't find anything, sad times.
    return nullptr;
}

void MultiIniFieldParse::Add(FieldParse *field_parse, unsigned int extra_offset)
{
    ASSERT_THROW(Count < MAX_MULTI_FIELDS, 0xDEAD0001);

    FieldParsers[Count] = field_parse;
    ExtraOffsets[Count] = extra_offset;
    ++Count;
}

INI::INI() :
    BackingFile(nullptr),
    BufferReadPos(0),
    BufferData(0),
    FileName("None"),
    LoadType(INI_LOAD_INVALID),
    Seps(" \n\r\t="),
    SepsPercent(" \n\r\t=%%"),
    SepsColon(" \n\r\t=:"),
    SepsQuote(" \"\n="),
    EndToken("END"),
    EndOfFile(false)
{
}

INI::~INI()
{
}

void INI::Load(AsciiString filename, INILoadType type, Xfer *xfer)
{
    Call_Function<void>(0x004A1E40); // setFPMode()
    SXfer = xfer;
    Prep_File(filename, type);

    while ( !EndOfFile ) {
        Read_Line();
        
        // Original seems to make an unused AsciiString from the
        // parsed block, possible leftover from debug code?
        //AsciiString block(CurrentBlock);

        char *token = crt_strtok(CurrentBlock, Seps);
        
        if ( token != nullptr ) {
            iniblockparse_t parser = Find_Block_Parse(token);

            ASSERT_THROW_PRINT(
                parser != nullptr,
                0xDEAD0006,
                "[LINE: %d - FILE: '%s'] Unknown block '%s'\n",
                LineNumber,
                FileName.Str(),
                token
            );

            parser(this);
        }
    }

    Unprep_File();
}

void INI::Load_Directory(AsciiString dir, bool search_subdirs, INILoadType type, Xfer *xfer)
{
    // TODO
    // Calls FileSystem::Get_File_List_From_Dir
    // Passes around a std::list, all calls to FileSystem::Get_File_List_From_Dir need to
    // be implemented at once due to ABI issues.
    Call_Method<void, INI, AsciiString, bool, INILoadType, Xfer*>(0x0041A1C0, this, dir, search_subdirs,type, xfer);
}

void INI::Prep_File(AsciiString filename, INILoadType type)
{
    ASSERT_THROW_PRINT(
        BackingFile == nullptr,
        0xDEAD0006,
        "Cannot open file %s, file already open.\n",
        filename.Str()
    );

    BackingFile = TheFileSystem->Open(filename.Str(), File::READ);

    ASSERT_THROW_PRINT(
        BackingFile != nullptr,
        0xDEAD0006,
        "Could not open file %s.\n",
        filename.Str()
    );

    FileName = filename;
    LoadType = type;
}

void INI::Unprep_File()
{
    BackingFile->Close();
    BackingFile = nullptr;
    BufferReadPos = 0;
    BufferData = 0;
    FileName = "None";
    LoadType = INI_LOAD_INVALID;
    LineNumber = 0;
    EndOfFile = false;
    SXfer = nullptr;
}

void INI::Init_From_INI(void *what, FieldParse *parse_table)
{
    MultiIniFieldParse p;

    p.Add(parse_table, 0);
    Init_From_INI_Multi(what, p);
}

void INI::Init_From_INI_Multi(void *what, MultiIniFieldParse const &parse_table_list)
{
    bool done = false;

    ASSERT_THROW_PRINT(what != nullptr, 0xDEAD0006, "Init_From_INI - Invalid parameters supplied.\n");

    while ( !done ) {
        ASSERT_THROW_PRINT(
            !EndOfFile,
            0xDEAD0006,
            "Error parsing block '%s', in INI file '%s'.  Missing '%s' token\n",
            CurrentBlock,
            FileName.Str(),
            EndToken
        );

        Read_Line();

        char *token = crt_strtok(CurrentBlock, Seps);

        if ( token == nullptr ) {
            continue;
        }

        if ( strcasecmp(token, EndToken) == 0 ) {
            done = true;
        } else {
            inifieldparse_t parsefunc;
            int offset;
            void const *data;
            int exoffset = 0;

            // Find an appropriate parser function from the parse table
            for ( int i = 0; ; ++i ) {
                ASSERT_THROW_PRINT(
                    i < parse_table_list.Count,
                    0xDEAD0006,
                    "[LINE: %d - FILE: '%s'] Unknown field '%s' in block '%s'\n",
                    LineNumber,
                    FileName.Str(),
                    token,
                    CurrentBlock
                );

                parsefunc = Find_Field_Parse(parse_table_list.FieldParsers[i], token, offset, data);
                
                if ( parsefunc != nullptr ) {
                    exoffset = parse_table_list.ExtraOffsets[i];

                    break;
                }
            }

            parsefunc(this, what, static_cast<char*>(what) + offset + exoffset, data);
        }
    }
}

void INI::Init_From_INI_Multi_Proc(void *what, void(*proc)(MultiIniFieldParse *))
{
    MultiIniFieldParse p;

    proc(&p);
    Init_From_INI_Multi(what, p);
}

void INI::Read_Line()
{
    ASSERT_PRINT(BackingFile != nullptr, "Read_Line file pointer is nullptr.\n");
    
    if ( EndOfFile ) {
        CurrentBlock[0] = '\0';
    } else {
        // Read into our current block buffer.
        char *cb;
        for ( cb = CurrentBlock; cb != &BlockEnd; ++cb ) {
            // If the buffer is empty, refill it.
            if ( BufferReadPos == BufferData ) {
                BufferReadPos = 0;
                BufferData = BackingFile->Read(Buffer, sizeof(Buffer));

                if ( BufferData == 0 ) {
                    EndOfFile = true;

                    break;
                }
            }

            *cb = Buffer[BufferReadPos++];

            // Reached end of line
            if ( *cb == '\n' ) {
                break;
            }

            // Handle comment marker and none printing chars
            if ( *cb == ';' ) {
                *cb = '\0';
            } else if ( *cb > '\0' && *cb < ' ' ) {
                *cb = ' ';
            }
        }

        // Null terminate the buffer
        *cb = '\0';
        ++LineNumber;
    }

    // If we have a transfer object assigned, do the transfer.
    if ( SXfer != nullptr ) {
        SXfer->xferImplementation(CurrentBlock, strlen(CurrentBlock));
    }
}

int INI::Scan_Science(char const *token)
{
    //return TheScienceStore->Friend_Lookup_Science(token);
    return Call_Function<int, char const*>(0x0041D740, token); // INI::scanScience
}

float INI::Scan_PercentToReal(char const *token)
{
    float value;

    ASSERT_THROW(sscanf(token, "%f", &value) == 1, 0xDEAD0006);

    return (float)(value / 100.0f);
}

float INI::Scan_Real(char const *token)
{
    float value;

    ASSERT_THROW(sscanf(token, "%f", &value) == 1, 0xDEAD0006);

    return (float)value;
}

uint32_t INI::Scan_UnsignedInt(char const *token)
{
    uint32_t value;

    ASSERT_THROW(sscanf(token, "%u", &value) == 1, 0xDEAD0006);

    return value;
}

int32_t INI::Scan_Int(char const *token)
{
    int32_t value;

    ASSERT_THROW(sscanf(token, "%d", &value) == 1, 0xDEAD0006);

    return value;
}

bool INI::Scan_Bool(char const *token)
{
    if ( strcasecmp(token, "yes") == 0 ) {
        return true;
    } else {
        ASSERT_THROW_PRINT(
            strcasecmp(token, "no") == 0,
            0xDEAD0006,
            "Invalid bool token %s, expected yes or no\n",
            token
        );
    }

    return false;
}

int INI::Scan_IndexList(char const *token, char const *const *list)
{
    ASSERT_THROW_PRINT(
        list != nullptr && *list != nullptr,
        0xDEAD0006,
        "Error, invalid list provided for Scan_IndexList\n"
    );

    int list_count = 0;

    while ( strcasecmp(list[list_count], token) != 0  ) {
        ASSERT_THROW_PRINT(
            list[++list_count] != nullptr,
            0xDEAD0006,
            "Token %s not found in list\n",
            token
        );
    }

    return list_count;
}

int INI::Scan_LookupList(char const *token, LookupListRec const *list)
{
    ASSERT_THROW_PRINT(
        list != nullptr && list->Name != nullptr,
        0xDEAD0006,
        "Error, invalid list provided for Scan_LookupList\n"
    );

    int list_count = 0;

    while ( strcasecmp(list[list_count].Name, token) != 0 ) {
        ASSERT_THROW_PRINT(
            list[++list_count].Name != nullptr,
            0xDEAD0006,
            "Token %s not found in list\n",
            token
        );
    }

    return list[list_count].Value;
}

// Field parsing functions.
void INI::Parse_Bool(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<bool*>(store) = Scan_Bool(ini->Get_Next_Token());
}

void INI::Parse_Byte(INI *ini, void *formal, void *store, void const *user_data)
{
    int tmp = Scan_Int(ini->Get_Next_Token());

    ASSERT_THROW_PRINT(tmp >= 0 && tmp < 256, 0xDEAD0001, "Value parsed outside range of a byte.\n");

    *static_cast<uint8_t*>(store) = tmp;
}

void INI::Parse_Int(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<int32_t*>(store) = Scan_Int(ini->Get_Next_Token());
}

void INI::Parse_Unsigned(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<uint32_t*>(store) = Scan_UnsignedInt(ini->Get_Next_Token());
}

void INI::Parse_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Positive_None_Zero_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    float tmp = Scan_Real(ini->Get_Next_Token());

    ASSERT_THROW_PRINT(tmp >= 0.0f, 0xDEAD0006, "Value parsed outside range of a positive none zero float.\n");

    *static_cast<float*>(store) = tmp;
}

void INI::Parse_Percent_To_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token(ini->SepsPercent));
}

void INI::Parse_Angle_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _ANGLE_MULTIPLIER;
}

void INI::Parse_Angular_Velocity_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * (_SECONDS_PER_LOGICFRAME_REAL_74 * _ANGLE_MULTIPLIER);
}

void INI::Parse_AsciiString(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<AsciiString*>(store) = ini->Get_Next_Ascii_String();
}

void INI::Parse_AsciiString_Vector_Append(INI *ini, void *formal, void *store, void const *user_data)
{
    DEBUG_LOG("Appending Vector for ini %s.\n", ini->FileName.Str());
    std::vector<AsciiString> *vec = static_cast<std::vector<AsciiString> *>(store);

    for ( char *i = ini->Get_Next_Token_Or_Null(); i != nullptr; i = ini->Get_Next_Token_Or_Null() ) {
        vec->push_back(AsciiString(i));
    }
}

void INI::Parse_RGB_Color(INI *ini, void *formal, void *store, void const *user_data)
{
    int colors[3];
    char const *names[3] = { "R", "G", "B" };
    RGBColor *rgb = static_cast<RGBColor*>(store);

    for ( int i = 0; i < 3; ++i ) {
        char *token = ini->Get_Next_Sub_Token(names[i]);
        colors[i] = Clamp(Scan_Int(token), 0, 255);
    }

    rgb->red = colors[0] / 255.0f;
    rgb->green = colors[1] / 255.0f;
    rgb->blue = colors[2] / 255.0f;
}

void INI::Parse_RGBA_Color_Int(INI *ini, void *formal, void *store, void const *user_data)
{
    int colors[4];
    char const *names[4] = { "R", "G", "B", "A" };
    RGBAColorInt *rgba = static_cast<RGBAColorInt*>(store);

    for ( int i = 0; i < 4; ++i ) {
        char *token = ini->Get_Next_Token_Or_Null(ini->SepsColon);

        if ( token != nullptr ) {
            ASSERT_THROW_PRINT(strcasecmp(token, names[i]) == 0, 0xDEAD0006, "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.\n", token);

            colors[i] = Clamp(Scan_Int(ini->Get_Next_Token(ini->SepsColon)), 0, 255);
        } else {
            ASSERT_THROW_PRINT(i == 3, 0xDEAD006, "Expected token missing, only 'A' token is optional for 'RBGA'.\n");
            colors[i] = 255;
        }
    }

    rgba->red = colors[0];
    rgba->green = colors[1];
    rgba->blue = colors[2];
    rgba->alpha = colors[3];
}

void INI::Parse_Color_Int(INI *ini, void *formal, void *store, void const *user_data)
{
    int colors[4];
    char const *names[4] = { "R", "G", "B", "A" };
    uint32_t *rgba = static_cast<uint32_t*>(store);

    for ( int i = 0; i < 4; ++i ) {
        char *token = ini->Get_Next_Token_Or_Null(ini->SepsColon);

        if ( token != nullptr ) {
            ASSERT_THROW_PRINT(strcasecmp(token, names[i]) == 0, 0xDEAD0006, "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.\n", token);

            colors[i] = Clamp(Scan_Int(ini->Get_Next_Token(ini->SepsColon)), 0, 255);
        } else {
            ASSERT_THROW_PRINT(i == 3, 0xDEAD006, "Expected token missing, only 'A' token is optional for 'RBGA'.\n");
            colors[i] = 255;
        }
    }

    *rgba = Make_Color(colors[0], colors[1], colors[2], colors[3]);
}

void INI::Parse_Coord2D(INI *ini, void *formal, void *store, void const *user_data)
{
    Coord2D *coord = static_cast<Coord2D*>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
}

void INI::Parse_Coord3D(INI *ini, void *formal, void *store, void const *user_data)
{
    Coord3D *coord = static_cast<Coord3D*>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
    coord->z = Scan_Real(ini->Get_Next_Sub_Token("Z"));
}

void INI::Parse_Index_List(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<int*>(store) = Scan_IndexList(ini->Get_Next_Token(), static_cast<const char *const *>(user_data));
}

void INI::Parse_Duration_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = _DURATION_MULT * Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Duration_Int(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<uint32_t*>(store) = ceilf(_DURATION_MULT * Scan_UnsignedInt(ini->Get_Next_Token()));
}

void INI::Parse_Velocity_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Acceleration_Real(INI *ini, void *formal, void *store, void const *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74 * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Bit_In_Int32(INI *ini, void *formal, void *store, void const *user_data)
{
    uint32_t i = reinterpret_cast<uintptr_t>(user_data);

    if ( Scan_Bool(ini->Get_Next_Token()) ) {
        *static_cast<int32_t*>(store) |= i;
    } else {
        *static_cast<int32_t*>(store) &= ~i;
    }
}
