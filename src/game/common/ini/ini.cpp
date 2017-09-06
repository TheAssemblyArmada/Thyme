/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Parser for SAGE engine configuration files.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ini.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "audiosettings.h"
#include "color.h"
#include "coord.h"
#include "file.h"
#include "filesystem.h"
#include "gamedebug.h"
#include "gamelod.h"
#include "gamemath.h"
#include "gametext.h"
#include "gametype.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "minmax.h"
#include "stringex.h"
#include "terrainroads.h"
#include "terraintypes.h"
#include "water.h"
#include "xfer.h"
#include <cctype>
#include <cstdio>

using GameMath::Ceil;

const float _SECONDS_PER_LOGICFRAME_REAL_74 = 1.0f / 30.0f;
const float _ANGLE_MULTIPLIER = 0.0174532925f;
const float _DURATION_MULT = 0.029999999f;

// Replace original function addresses as thyme implementations are written.
BlockParse TheTypeTable[] =
{
    { "AIData", (iniblockparse_t)(0x00518F00) /*&INI::parseAIDataDefinition*/ },
    { "Animation", (iniblockparse_t)(0x00518DB0)/*&INI::parseAnim2DDefinition*/ },
    { "Armor", (iniblockparse_t)(0x004B60A0)/*&INI::parseArmorDefinition*/ },
    { "AudioEvent", (iniblockparse_t)(0x0044ED70)/*&INI::parseAudioEventDefinition*/ },
    { "AudioSettings", &AudioSettings::Parse_Audio_Settings },
    { "Bridge", &TerrainRoadCollection::Parse_Terrain_Bridge_Definitions },
    { "Campaign", (iniblockparse_t)(0x00517490)/*&INI::parseCampaignDefinition*/ },
    { "ChallengeGenerals", (iniblockparse_t)(0x005170B0)/*&INI::parseChallengeModeDefinition*/ },
    { "CommandButton", (iniblockparse_t)(0x00516CE0)/*&INI::parseCommandButtonDefinition*/ },
    { "CommandMap", (iniblockparse_t)(0x00498480)/*&INI::parseMetaMapDefinition*/ },
    { "CommandSet", (iniblockparse_t)(0x00516CD0)/*&INI::parseCommandSetDefinition*/ },
    { "ControlBarScheme", (iniblockparse_t)(0x00516BA0)/*&INI::parseControlBarSchemeDefinition*/ },
    { "ControlBarResizer", (iniblockparse_t)(0x0062D610)/*&INI::parseControlBarResizerDefinition*/ },
    { "CrateData", (iniblockparse_t)(0x00516B90)/*&INI::parseCrateTemplateDefinition*/ },
    { "Credits", (iniblockparse_t)(0x00515A20)/*&INI::parseCredits*/ },
    { "WindowTransition", (iniblockparse_t)(0x005145F0)/*&INI::parseWindowTransitions*/ },
    { "DamageFX", (iniblockparse_t)(0x005145E0)/*&INI::parseDamageFXDefinition*/ },
    { "DialogEvent", (iniblockparse_t)(0x0044EFE0)/*&INI::parseDialogDefinition*/ },
    { "DrawGroupInfo", (iniblockparse_t)(0x005145B0)/*&INI::parseDrawGroupNumberDefinition*/ },
    { "EvaEvent", (iniblockparse_t)(0x00512BE0)/*&INI::parseEvaEvent*/ },
    { "FXList", (iniblockparse_t)(0x004CC260)/*&INI::parseFXListDefinition*/ },
    { "GameData", &GlobalData::Parse_Game_Data_Definitions },
    { "InGameUI", (iniblockparse_t)(0x00508440)/*&INI::parseInGameUIDefinition*/ },
    { "Locomotor", (iniblockparse_t)(0x004B8A70)/*&INI::parseLocomotorTemplateDefinition*/ },
    { "Language", &GlobalLanguage::Parse_Language_Defintions },
    { "MapCache", (iniblockparse_t)(0x00506760)/*&INI::parseMapCacheDefinition*/ },
    { "MapData", (iniblockparse_t)(0x0062D610)/*&INI::parseMapDataDefinition*/ },
    { "MappedImage", (iniblockparse_t)(0x00506510)/*&INI::parseMappedImageDefinition*/ },
    //{ "MiscAudio", (iniblockparse_t)(0x005064F0)/*&INI::parseMiscAudio*/ },
    { "MiscAudio", &MiscAudio::Parse_Misc_Audio },
    { "Mouse", (iniblockparse_t)(0x004041F0)/*&INI::parseMouseDefinition*/ },
    { "MouseCursor", (iniblockparse_t)(0x00404060)/*&INI::parseMouseCursorDefinition*/ },
    { "MultiplayerColor", (iniblockparse_t)(0x00504B10)/*&INI::parseMultiplayerColorDefinition*/ },
    { "MultiplayerStartingMoneyChoice", (iniblockparse_t)(0x00504C80)/*&INI::parseMultiplayerStartingMoneyChoiceDefinition*/ },
    { "OnlineChatColors", (iniblockparse_t)(0x00504D10)/*&INI::parseOnlineChatColorDefinition*/ },
    { "MultiplayerSettings", (iniblockparse_t)(0x00504A90)/*&INI::parseMultiplayerSettingsDefinition*/ },
    { "MusicTrack", (iniblockparse_t)(0x0044EAF0)/*&INI::parseMusicTrackDefinition*/ },
    { "Object", (iniblockparse_t)(0x005048E0)/*&INI::parseObjectDefinition*/ },
    { "ObjectCreationList", (iniblockparse_t)(0x004C1570)/*&INI::parseObjectCreationListDefinition*/ },
    { "ObjectReskin", (iniblockparse_t)(0x00504990)/*&INI::parseObjectReskinDefinition*/ },
    { "ParticleSystem", (iniblockparse_t)(0x005047E0)/*&INI::parseParticleSystemDefinition*/ },
    { "PlayerTemplate", (iniblockparse_t)(0x004D3DC0)/*&INI::parsePlayerTemplateDefinition*/ },
    { "Road", TerrainRoadCollection::Parse_Terrain_Road_Definitions },
    { "Science", (iniblockparse_t)(0x00488EA0)/*&INI::parseScienceDefinition*/ },
    { "Rank", (iniblockparse_t)(0x00489800)/*&INI::parseRankDefinition*/ },
    { "SpecialPower", (iniblockparse_t)(0x00504690)/*&INI::parseSpecialPowerDefinition*/ },
    { "ShellMenuScheme", (iniblockparse_t)(0x00503CE0)/*&INI::parseShellMenuSchemeDefinition*/ },
    { "Terrain", &TerrainTypeCollection::Parse_Terrain_Definition },
    { "Upgrade", (iniblockparse_t)(0x00503BA0)/*&INI::parseUpgradeDefinition*/ },
    { "Video", (iniblockparse_t)(0x005039D0)/*&INI::parseVideoDefinition*/ },
    { "WaterSet", &WaterSetting::Parse_Water_Setting },
    { "WaterTransparency", (iniblockparse_t)(0x00503390)/*&INI::parseWaterTransparencyDefinition*/ },
    { "Weather", (iniblockparse_t)(0x00502DF0)/*&INI::parseWeatherDefinition*/ },
    { "Weapon", (iniblockparse_t)(0x00502B30)/*&INI::parseWeaponTemplateDefinition*/ },
    { "WebpageURL", (iniblockparse_t)(0x005028F0)/*&INI::parseWebpageURL*/ },
    { "HeaderTemplate", (iniblockparse_t)(0x00501E50)/*&INI::parseHeaderTemplateDefinition*/ },
    { "StaticGameLOD", &GameLODManager::Parse_Static_LOD_Definitions },
    { "DynamicGameLOD", &GameLODManager::Parse_Dynamic_LOD_Definitions },
    { "LODPreset", &GameLODManager::Parse_LOD_Preset },
    { "BenchProfile", &GameLODManager::Parse_Bench_Profiles },
    { "ReallyLowMHz", &GameLODManager::Parse_Low_MHz },
    { "ScriptAction", (iniblockparse_t)(0x004221C0)/*&ScriptEngine::parseScriptAction*/ },
    { "ScriptCondition", (iniblockparse_t)(0x00422680)/*&ScriptEngine::parseScriptCondition*/ },
    { nullptr, nullptr }
};

// Helper function for Load
inline iniblockparse_t Find_Block_Parse(const char *token)
{
    // Iterate over the TypeTable to identify correct parsing function.
    for ( BlockParse *block = TheTypeTable; block->token != nullptr; ++block ) {
        if ( strcmp(block->token, token) == 0 ) {
            return block->parse_func;
        }
    }

    return nullptr;
}

// Helper function for Init_From_INI_Multi
inline inifieldparse_t Find_Field_Parse(FieldParse *table, const char *token, int &offset, const void *&data)
{
    FieldParse *tblptr;

    // Search the list for a matching FieldParse struct.
    for ( tblptr = table; tblptr->token != nullptr; ++tblptr ) {
        // If found, return the data and associated function.
        if ( strcmp(tblptr->token, token) == 0 ) {
            offset = tblptr->offset;
            data = tblptr->user_data;

            return tblptr->parse_func;
        }
    }

    // Didn't find matching token, but null token entry has a function
    if ( tblptr->parse_func != nullptr ) {
        offset = tblptr->offset;
        data = token;

        return tblptr->parse_func;
    }

    // Didn't find anything, sad times.
    return nullptr;
}

INI::INI() :
    m_backingFile(nullptr),
    m_bufferReadPos(0),
    m_bufferData(0),
    m_fileName("None"),
    m_loadType(INI_LOAD_INVALID),
    m_seps(" \n\r\t="),
    m_sepsPercent(" \n\r\t=%%"),
    m_sepsColon(" \n\r\t=:"),
    m_sepsQuote("\"\n="),
    m_endToken("END"),
    m_endOfFile(false)
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

    while ( !m_endOfFile ) {
        Read_Line();
        
        // Original seems to make an unused AsciiString from the
        // parsed block, possible leftover from debug code?
        //AsciiString block(m_currentBlock);

        char *token = crt_strtok(m_currentBlock, m_seps);
        
        if ( token != nullptr ) {
            iniblockparse_t parser = Find_Block_Parse(token);

            ASSERT_THROW_PRINT(
                parser != nullptr,
                0xDEAD0006,
                "[LINE: %d - FILE: '%s'] Unknown block '%s'\n",
                m_lineNumber,
                m_fileName.Str(),
                token
            );

            parser(this);
        }
    }

    Unprep_File();
}

void INI::Load_Directory(AsciiString dir, bool search_subdirs, INILoadType type, Xfer *xfer)
{
    ASSERT_THROW(!dir.Is_Empty(), 0xDEAD0006);

    std::set<AsciiString, rts::less_than_nocase<AsciiString> > files;
    dir += '/';

    g_theFileSystem->Get_File_List_From_Dir(dir, "*.ini", files, true);

    // Load everything from the top level directory first.
    for ( auto it = files.begin(); it != files.end(); ++it ) {
        // Create path string with initial dir stripped off.
        AsciiString path_check = &it->Str()[strlen(dir.Str())];
        
        if ( strchr(path_check.Str(), '\\') == nullptr && strchr(path_check.Str(), '/') == nullptr ) {
            Load(*it, type, xfer);
        }
    }

    // Now process everything from sub directories.
    for ( auto it = files.begin(); it != files.end(); ++it ) {
        // Create path string with initial dir stripped off.
        AsciiString path_check = &it->Str()[dir.Get_Length()];
        
        if ( strchr(path_check.Str(), '\\') != nullptr && strchr(path_check.Str(), '/') != nullptr ) {
            Load(*it, type, xfer);
        }
    }
}

void INI::Prep_File(AsciiString filename, INILoadType type)
{
    ASSERT_THROW_PRINT(
        m_backingFile == nullptr,
        0xDEAD0006,
        "Cannot open file %s, file already open.\n",
        filename.Str()
    );

    m_backingFile = g_theFileSystem->Open(filename.Str(), File::READ);

    ASSERT_THROW_PRINT(
        m_backingFile != nullptr,
        0xDEAD0006,
        "Could not open file %s.\n",
        filename.Str()
    );

    m_fileName = filename;
    m_loadType = type;
}

void INI::Unprep_File()
{
    m_backingFile->Close();
    m_backingFile = nullptr;
    m_bufferReadPos = 0;
    m_bufferData = 0;
    m_fileName = "None";
    m_loadType = INI_LOAD_INVALID;
    m_lineNumber = 0;
    m_endOfFile = false;
    SXfer = nullptr;
}

void INI::Init_From_INI(void *what, FieldParse *parse_table)
{
    MultiIniFieldParse p;

    p.Add(parse_table, 0);
    Init_From_INI_Multi(what, p);
}

void INI::Init_From_INI_Multi(void *what, const MultiIniFieldParse &parse_table_list)
{
    bool done = false;

    ASSERT_THROW_PRINT(what != nullptr, 0xDEAD0006, "Init_From_INI - Invalid parameters supplied.\n");

    while ( !done ) {
        ASSERT_THROW_PRINT(
            !m_endOfFile,
            0xDEAD0006,
            "Error parsing block '%s', in INI file '%s'.  Missing '%s' token\n",
            m_currentBlock,
            m_fileName.Str(),
            m_endToken
        );

        Read_Line();

        char *token = crt_strtok(m_currentBlock, m_seps);

        if ( token == nullptr ) {
            continue;
        }

        if ( strcasecmp(token, m_endToken) == 0 ) {
            done = true;
        } else {
            inifieldparse_t parsefunc;
            int offset;
            const void *data;
            int exoffset = 0;

            // Find an appropriate parser function from the parse table
            for ( int i = 0; ; ++i ) {
                ASSERT_THROW_PRINT(
                    i < parse_table_list.count,
                    0xDEAD0006,
                    "[LINE: %d - FILE: '%s'] Unknown field '%s' in block '%s'\n",
                    m_lineNumber,
                    m_fileName.Str(),
                    token,
                    m_currentBlock
                );

                parsefunc = Find_Field_Parse(parse_table_list.field_parsers[i], token, offset, data);
                
                if ( parsefunc != nullptr ) {
                    exoffset = parse_table_list.extra_offsets[i];

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
    ASSERT_PRINT(m_backingFile != nullptr, "Read_Line file pointer is nullptr.\n");
    
    if ( m_endOfFile ) {
        m_currentBlock[0] = '\0';
    } else {
        // Read into our current block buffer.
        char *cb;
        for ( cb = m_currentBlock; cb != &m_blockEnd; ++cb ) {
            // If the buffer is empty, refill it.
            if ( m_bufferReadPos == m_bufferData ) {
                m_bufferReadPos = 0;
                m_bufferData = m_backingFile->Read(m_buffer, sizeof(m_buffer));

                if ( m_bufferData == 0 ) {
                    m_endOfFile = true;

                    break;
                }
            }

            *cb = m_buffer[m_bufferReadPos++];

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
        ++m_lineNumber;
    }

    // If we have a transfer object assigned, do the transfer.
    if ( SXfer != nullptr ) {
        SXfer->xferImplementation(m_currentBlock, strlen(m_currentBlock));
    }
}

AsciiString INI::Get_Next_Quoted_Ascii_String()
{
    const char *token = Get_Next_Token_Or_Null();
    AsciiString next;
    char buffer[MAX_LINE_LENGTH];

    buffer[0] = '\0';

    if ( token != nullptr ) {
        if ( *token == '"' ) {
            if ( strlen(token) > 1 ) {
                strlcpy(buffer, token + 1, sizeof(buffer));

                if ( buffer[strlen(buffer) - 1] == '"' ) {
                    buffer[strlen(buffer) - 1] = '\0';

                    return buffer;
                }
            }

            const char *next_tok = Get_Next_Token(m_sepsQuote);

            if ( strlen(next_tok) > 1 && next_tok[1] != '\t' ) {
                strlcat(buffer, " ", sizeof(buffer));
                strlcat(buffer, next_tok, sizeof(buffer));
            }

            if ( buffer[strlen(buffer) - 1] == '"' ) {
                buffer[strlen(buffer) - 1] = '\0';
            }

            next = buffer;
        } else {
            next = token;
        }
    }

    return next;
}

int INI::Scan_Science(const char *token)
{
    //return TheScienceStore->Friend_Lookup_Science(token);
    return Call_Function<int, char const*>(0x0041D740, token); // INI::scanScience
}

float INI::Scan_PercentToReal(const char *token)
{
    float value;

    ASSERT_THROW(sscanf(token, "%f", &value) == 1, 0xDEAD0006);

    return (float)(value / 100.0f);
}

float INI::Scan_Real(const char *token)
{
    float value;

    ASSERT_THROW(sscanf(token, "%f", &value) == 1, 0xDEAD0006);

    return (float)value;
}

unsigned int INI::Scan_UnsignedInt(const char *token)
{
    unsigned int value;

    ASSERT_THROW(sscanf(token, "%u", &value) == 1, 0xDEAD0006);

    return value;
}

int INI::Scan_Int(const char *token)
{
    int value;

    ASSERT_THROW(sscanf(token, "%d", &value) == 1, 0xDEAD0006);

    return value;
}

bool INI::Scan_Bool(const char *token)
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

int INI::Scan_IndexList(const char *token, const char *const *list)
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

int INI::Scan_LookupList(const char *token, const LookupListRec *list)
{
    ASSERT_THROW_PRINT(
        list != nullptr && list->name != nullptr,
        0xDEAD0006,
        "Error, invalid list provided for Scan_LookupList\n"
    );

    int list_count = 0;

    while ( strcasecmp(list[list_count].name, token) != 0 ) {
        ASSERT_THROW_PRINT(
            list[++list_count].name != nullptr,
            0xDEAD0006,
            "Token %s not found in list\n",
            token
        );
    }

    return list[list_count].value;
}

// Field parsing functions.
void INI::Parse_Bool(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<bool*>(store) = Scan_Bool(ini->Get_Next_Token());
}

void INI::Parse_Byte(INI *ini, void *formal, void *store, const void *user_data)
{
    int tmp = Scan_Int(ini->Get_Next_Token());

    ASSERT_THROW_PRINT(tmp >= 0 && tmp < 256, 0xDEAD0001, "Value parsed outside range of a byte.\n");

    *static_cast<uint8_t*>(store) = tmp;
}

void INI::Parse_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<int*>(store) = Scan_Int(ini->Get_Next_Token());
}

void INI::Parse_Unsigned(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<unsigned int*>(store) = Scan_UnsignedInt(ini->Get_Next_Token());
}

void INI::Parse_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Positive_None_Zero_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    float tmp = Scan_Real(ini->Get_Next_Token());

    ASSERT_THROW_PRINT(tmp >= 0.0f, 0xDEAD0006, "Value parsed outside range of a positive none zero float.\n");

    *static_cast<float*>(store) = tmp;
}

void INI::Parse_Percent_To_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_PercentToReal(ini->Get_Next_Token(ini->m_sepsPercent));
}

void INI::Parse_Angle_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _ANGLE_MULTIPLIER;
}

void INI::Parse_Angular_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * (_SECONDS_PER_LOGICFRAME_REAL_74 * _ANGLE_MULTIPLIER);
}

void INI::Parse_AsciiString(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<AsciiString*>(store) = ini->Get_Next_Ascii_String();
}

void INI::Parse_Quoted_AsciiString(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<AsciiString*>(store) = ini->Get_Next_Quoted_Ascii_String();
}

void INI::Parse_AsciiString_Vector_Append(INI *ini, void *formal, void *store, const void *user_data)
{
    //DEBUG_LOG("Appending Vector for ini %s.\n", ini->m_fileName.Str());
    std::vector<AsciiString> *vec = static_cast<std::vector<AsciiString> *>(store);

    for ( const char *i = ini->Get_Next_Token_Or_Null(); i != nullptr; i = ini->Get_Next_Token_Or_Null() ) {
        vec->push_back(AsciiString(i));
    }
}

void INI::Parse_RGB_Color(INI *ini, void *formal, void *store, const void *user_data)
{
    int colors[3];
    const char *names[3] = { "R", "G", "B" };
    RGBColor *rgb = static_cast<RGBColor*>(store);

    for ( int i = 0; i < 3; ++i ) {
        const char *token = ini->Get_Next_Sub_Token(names[i]);
        colors[i] = Clamp(Scan_Int(token), 0, 255);
    }

    rgb->red = colors[0] / 255.0f;
    rgb->green = colors[1] / 255.0f;
    rgb->blue = colors[2] / 255.0f;
}

void INI::Parse_RGBA_Color_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    int colors[4];
    const char *names[4] = { "R", "G", "B", "A" };
    RGBAColorInt *rgba = static_cast<RGBAColorInt*>(store);

    for ( int i = 0; i < 4; ++i ) {
        const char *token = ini->Get_Next_Token_Or_Null(ini->m_sepsColon);

        if ( token != nullptr ) {
            ASSERT_THROW_PRINT(strcasecmp(token, names[i]) == 0, 0xDEAD0006, "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.\n", token);

            colors[i] = Clamp(Scan_Int(ini->Get_Next_Token(ini->m_sepsColon)), 0, 255);
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

void INI::Parse_Color_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    int colors[4];
    const char *names[4] = { "R", "G", "B", "A" };
    uint32_t *rgba = static_cast<uint32_t*>(store);

    for ( int i = 0; i < 4; ++i ) {
        const char *token = ini->Get_Next_Token_Or_Null(ini->m_sepsColon);

        if ( token != nullptr ) {
            ASSERT_THROW_PRINT(strcasecmp(token, names[i]) == 0, 0xDEAD0006, "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.\n", token);

            colors[i] = Clamp(Scan_Int(ini->Get_Next_Token(ini->m_sepsColon)), 0, 255);
        } else {
            ASSERT_THROW_PRINT(i == 3, 0xDEAD006, "Expected token missing, only 'A' token is optional for 'RBGA'.\n");
            colors[i] = 255;
        }
    }

    *rgba = Make_Color(colors[0], colors[1], colors[2], colors[3]);
}

void INI::Parse_Coord2D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord2D *coord = static_cast<Coord2D*>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
}

void INI::Parse_Coord3D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord3D *coord = static_cast<Coord3D*>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
    coord->z = Scan_Real(ini->Get_Next_Sub_Token("Z"));
}

void INI::Parse_Index_List(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<int*>(store) = Scan_IndexList(ini->Get_Next_Token(), static_cast<const char *const *>(user_data));
}

void INI::Parse_Duration_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = _DURATION_MULT * Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Duration_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<uint32_t*>(store) = Ceil(_DURATION_MULT * Scan_UnsignedInt(ini->Get_Next_Token()));
}

void INI::Parse_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Acceleration_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float*>(store) = Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74 * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Bit_In_Int32(INI *ini, void *formal, void *store, const void *user_data)
{
    uint32_t i = reinterpret_cast<uintptr_t>(user_data);

    if ( Scan_Bool(ini->Get_Next_Token()) ) {
        *static_cast<int32_t*>(store) |= i;
    } else {
        *static_cast<int32_t*>(store) &= ~i;
    }
}

void INI::Parse_And_Translate_Label(INI *ini, void *formal, void *store, const void *user_data)
{
    UnicodeString *str = static_cast<UnicodeString*>(store);
    *str = g_theGameText->Fetch(ini->Get_Next_Token());
}

void INI::Parse_Bitstring32(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *const *list = static_cast<const char *const *>(user_data);
    ASSERT_THROW_PRINT(list != nullptr && *list != nullptr, 0xDEAD0006, "No flag list provided.\n");
    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if ( token != nullptr ) {
        while ( strcasecmp(token, "NONE") != 0 ) {
            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if ( *token == '+' ) {
                ASSERT_THROW_PRINT(
                    !set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                );

                *static_cast<uint32_t*>(store) |= 1 << INI::Scan_IndexList(token + 1, list);
                adjust = true;
            } else if ( *token == '-' ) {
                ASSERT_THROW_PRINT(
                    !set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                ); 
                
                *static_cast<uint32_t*>(store) &= ~(1 << INI::Scan_IndexList(token + 1, list));
                adjust = true;
            } else {
                ASSERT_THROW_PRINT(
                    !adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                );

                // If we haven't started setting yet, 0 it out.
                if ( !set ) {
                    *static_cast<uint32_t*>(store) = 0;
                }

                *static_cast<uint32_t*>(store) |= 1 << INI::Scan_IndexList(token, list);
                set = true;
            }
            
            token = ini->Get_Next_Token_Or_Null();

            if ( token == nullptr ) {
                return;
            }
        }

        // If we encounter a "NONE" entry, set all bits to 0.
        // We should never reach here if something has already been set.
        ASSERT_THROW_PRINT(
            !adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.\n",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number()
        ); 
        
        *static_cast<uint32_t*>(store) = 0;
    }
}

void INI::Parse_Bitstring64(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *const *list = static_cast<const char *const *>(user_data);
    ASSERT_THROW_PRINT(list != nullptr && *list != nullptr, 0xDEAD0006, "No flag list provided.\n");
    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if ( token != nullptr ) {
        while ( strcasecmp(token, "NONE") != 0 ) {
            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if ( *token == '+' ) {
                ASSERT_THROW_PRINT(
                    !set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                );

                *static_cast<uint64_t*>(store) |= 1ll << INI::Scan_IndexList(token + 1, list);
                adjust = true;
            } else if ( *token == '-' ) {
                ASSERT_THROW_PRINT(
                    !set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                );

                *static_cast<uint64_t*>(store) &= ~(1ll << INI::Scan_IndexList(token + 1, list));
                adjust = true;
            } else {
                ASSERT_THROW_PRINT(
                    !adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.\n",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number()
                );

                // If we haven't started setting yet, 0 it out.
                if ( !set ) {
                    *static_cast<uint64_t*>(store) = 0;
                }

                *static_cast<uint64_t*>(store) |= 1ll << INI::Scan_IndexList(token, list);
                set = true;
            }

            token = ini->Get_Next_Token_Or_Null();

            if ( token == nullptr ) {
                return;
            }
        }

        // If we encounter a "NONE" entry, set all bits to 0.
        // We should never reach here if something has already been set.
        ASSERT_THROW_PRINT(
            !adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.\n",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number()
        ); 
        
        *static_cast<uint64_t*>(store) = 0;
    }
}

void INI::Parse_Speaker_Type(INI *ini, void *formal, void *store, const void *user_data)
{
    AsciiString speaker;
    Parse_AsciiString(ini, formal, &speaker, user_data);
    *static_cast<SpeakerType*>(store) = static_cast<SpeakerType>(g_theAudio->Translate_From_Speaker_Type(speaker));
}

void INI::Parse_Audio_Event_RTS(INI *ini, void *formal, void *store, const void *user_data)
{
    AudioEventRTS *ev = static_cast<AudioEventRTS *>(store);
    const char *token = ini->Get_Next_Token();

    if (strcasecmp(token, "NoSound") != 0) {
        ev->Set_Event_Name(token);
    }

    g_theAudio->Get_Info_For_Audio_Event(ev);
}
