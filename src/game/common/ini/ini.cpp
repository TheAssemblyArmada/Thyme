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
#include "ini.h"
#include "ai.h"
#include "anim2d.h"
#include "armor.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "audiosettings.h"
#include "campaignmanager.h"
#include "challengegenerals.h"
#include "color.h"
#include "colorspace.h"
#include "controlbar.h"
#include "coord.h"
#include "cratesystem.h"
#include "credits.h"
#include "damagefx.h"
#include "eva.h"
#include "file.h"
#include "filesystem.h"
#include "fpusetting.h"
#include "fxlist.h"
#include "gamelod.h"
#include "gamemath.h"
#include "gametext.h"
#include "gametype.h"
#include "gamewindowtransitions.h"
#include "globaldata.h"
#include "globallanguage.h"
#include "headertemplate.h"
#include "image.h"
#include "locomotor.h"
#include "maputil.h"
#include "metaevent.h"
#include "mouse.h"
#include "multiplayersettings.h"
#include "objectcreationlist.h"
#include "particlesysmanager.h"
#include "playertemplate.h"
#include "rankinfo.h"
#include "science.h"
#include "scriptengine.h"
#include "shellmenuscheme.h"
#include "snow.h"
#include "specialpower.h"
#include "terrainroads.h"
#include "terraintypes.h"
#include "thingfactory.h"
#include "upgrade.h"
#include "videoplayer.h"
#include "water.h"
#include "weapon.h"
#include "xfer.h"
#include <algorithm>
#include <cctype>
#include <cstdio>

using GameMath::Ceil;

#ifndef GAME_DLL
Xfer *g_sXfer = nullptr;
#else
#include "hooker.h"
#endif

const float _SECONDS_PER_LOGICFRAME_REAL_74 = 1.0f / 30.0f;
const float _ANGLE_MULTIPLIER = 0.0174532925f;
const float _DURATION_MULT = 0.029999999f;

// Replace original function addresses as thyme implementations are written.
// clang-format off
const BlockParse TheTypeTable[] = {
    {"AIData", AI::Parse_AI_Data_Definition},
    {"Animation", &Anim2DCollection::Parse_Anim2D_Definition},
    {"Armor", &ArmorStore::Parse_Armor_Definition},
    {"AudioEvent", &AudioEventInfo::Parse_Audio_Event_Definition},
    {"AudioSettings", &AudioSettings::Parse_Audio_Settings_Definition},
    {"Bridge", &TerrainRoadCollection::Parse_Terrain_Bridge_Definition},
    {"Campaign", &CampaignManager::Parse},
    {"ChallengeGenerals", &ChallengeGenerals::Parse_Challenge_Mode_Definition},
    {"CommandButton", &ControlBar::Parse_Command_Button_Definition},
    {"CommandMap", &MetaMap::Parse_Meta_Map},
    {"CommandSet", &ControlBar::Parse_Command_Set_Definition},
    {"ControlBarScheme", &ControlBar::Parse_Control_Bar_Scheme_Definition},
    {"ControlBarResizer", &INI::Parse_Control_Bar_Resizer_Definition},
    {"CrateData", &CrateSystem::Parse_Crate_Template_Definition},
    {"Credits", &CreditsManager::Parse},
    {"WindowTransition", &GameWindowTransitionsHandler::Parse_Window_Transitions},
    {"DamageFX", &DamageFXStore::Parse_Damage_FX_Definition},
    {"DialogEvent", &AudioEventInfo::Parse_Dialog_Definition},
    {"DrawGroupInfo", &INI::Parse_Draw_Group_Info },
    {"EvaEvent", &Eva::Parse},
    {"FXList", &FXListStore::Parse_FXList_Definition},
    {"GameData", &GlobalData::Parse_Game_Data_Definition},
    {"InGameUI", &InGameUI::Parse_In_Game_UI_Definition},
    {"Locomotor", &LocomotorStore::Parse_Locomotor_Template_Definition},
    {"Language", &GlobalLanguage::Parse_Language_Definition},
    {"MapCache", &MapCache::Parse_Map_Cache_Definition},
    {"MapData", &INI::Parse_Map_Data_Definition},
    {"MappedImage", &ImageCollection::Parse_Mapped_Image_Definition},
    {"MiscAudio", &MiscAudio::Parse_Misc_Audio},
    {"Mouse", &Mouse::Parse_Mouse_Definition},
    {"MouseCursor", &Mouse::Parse_Cursor_Definition},
    {"MultiplayerColor", &MultiplayerColorDefinition::Parse_Multiplayer_Color_Definition},
    {"MultiplayerStartingMoneyChoice",
        &MultiplayerSettings::Parse_Multiplayer_Starting_Money_Choice_Definition},
    {"OnlineChatColors", INI::Parse_Online_Chat_Color_Definition},
    {"MultiplayerSettings", &MultiplayerSettings::Parse_Multiplayer_Settings_Definition},
    {"MusicTrack", &AudioEventInfo::Parse_Music_Track_Definition},
    {"Object", &INI::Parse_Object_Definition},
    {"ObjectCreationList", &ObjectCreationListStore::Parse_Object_Creation_List_Definition },
    {"ObjectReskin", &INI::Parse_Object_Reskin_Definition},
    {"ParticleSystem", &ParticleSystemManager::Parse_Particle_System_Definition},
    {"PlayerTemplate", &PlayerTemplateStore::Parse_Player_Template_Definition},
    {"Road", &TerrainRoadCollection::Parse_Terrain_Road_Definition},
    {"Science", &ScienceStore::Parse_Science_Definition},
    {"Rank", &RankInfoStore::Parse_Rank_Definition},
    {"SpecialPower", &SpecialPowerStore::Parse_Special_Power_Definition},
    {"ShellMenuScheme", &ShellMenuSchemeManager::Parse},
    {"Terrain", &TerrainTypeCollection::Parse_Terrain_Definition},
    {"Upgrade", &UpgradeCenter::Parse_Upgrade_Definition},
    {"Video", &VideoPlayer::Parse},
    {"WaterSet", &WaterSetting::Parse_Water_Setting_Definition},
    {"WaterTransparency", &WaterTransparencySetting::Parse_Water_Transparency_Definition},
    {"Weather", &WeatherSetting::Parse_Weather_Definition},
    {"Weapon", &WeaponStore::Parse_Weapon_Template_Definition},
    {"WebpageURL", &INI::Parse_Webpage_URL_Definition},
    {"HeaderTemplate", &HeaderTemplateManager::Parse},
    {"StaticGameLOD", &GameLODManager::Parse_Static_LOD_Definition},
    {"DynamicGameLOD", &GameLODManager::Parse_Dynamic_LOD_Definition},
    {"LODPreset", &GameLODManager::Parse_LOD_Preset},
    {"BenchProfile", &GameLODManager::Parse_Bench_Profile},
    {"ReallyLowMHz", &GameLODManager::Parse_Really_Low_MHz},
    {"ScriptAction", &ScriptEngine::Parse_Script_Action},
    {"ScriptCondition", &ScriptEngine::Parse_Script_Condition},
    {nullptr, nullptr}
};
// clang-format on

// Helper function for Load
inline iniblockparse_t Find_Block_Parse(const char *token)
{
    // Iterate over the TypeTable to identify correct parsing function.
    for (const BlockParse *block = TheTypeTable; block->token != nullptr; ++block) {
        if (strcmp(block->token, token) == 0) {
            return block->parse_func;
        }
    }

    return nullptr;
}

// Helper function for Init_From_INI_Multi
inline inifieldparse_t Find_Field_Parse(const FieldParse *table, const char *token, int &offset, const void *&data)
{
    const FieldParse *tblptr;

    // Search the list for a matching FieldParse struct.
    for (tblptr = table; tblptr->token != nullptr; ++tblptr) {
        // If found, return the data and associated function.
        if (strcmp(tblptr->token, token) == 0) {
            offset = tblptr->offset;
            data = tblptr->user_data;

            return tblptr->parse_func;
        }
    }

    // Didn't find matching token, but null token entry has a function
    if (tblptr->parse_func != nullptr) {
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
    m_lineNumber(0),
    m_seps(" \n\r\t="),
    m_sepsPercent(" \n\r\t=%%"),
    m_sepsColon(" \n\r\t=:"),
    m_sepsQuote("\"\n="),
    m_endToken("END"),
    m_endOfFile(false)
{
    m_currentBlock[0] = '\0';
#ifdef GAME_DEBUG_STRUCTS
    m_curBlockStart[0] = '\0';
#endif
    // #BUGFIX Initialize all members
    m_buffer[0] = '\0';
}

INI::~INI() {}

void INI::Load(Utf8String filename, INILoadType type, Xfer *xfer)
{
    Set_FP_Mode(); // Ensure floating point mode is a consistent mode for loading.
    g_sXfer = xfer;
    Prep_File(filename, type);

    captainslog_dbgassert(!m_endOfFile, "INI::load, EOF at the beginning!");

    while (!m_endOfFile) {
        Read_Line();

        // Original seems to make an unused Utf8String from the
        // parsed block, possible leftover from debug code?
        // Utf8String block(m_currentBlock);

        char *token = strtok(m_currentBlock, m_seps);

        if (token != nullptr) {
            iniblockparse_t parser = Find_Block_Parse(token);

            if (parser != nullptr) {
#ifdef GAME_DEBUG_STRUCTS
                strcpy(m_curBlockStart, m_currentBlock);
#endif
                parser(this);
#ifdef GAME_DEBUG_STRUCTS
                strcpy(m_curBlockStart, "NO_BLOCK");
#endif
            } else {
                captainslog_relassert(
                    0, 0xDEAD0006, "[LINE: %d - FILE: '%s'] Unknown block '%s'", m_lineNumber, m_fileName.Str(), token);
            }
        }
    }

    Unprep_File();
}

void INI::Load_Directory(Utf8String dir, bool search_subdirs, INILoadType type, Xfer *xfer)
{
    captainslog_relassert(!dir.Is_Empty(), 0xDEAD0006, "No directory provided to load from.");

    std::set<Utf8String, rts::less_than_nocase<Utf8String>> files;
    dir += '/';

    g_theFileSystem->Get_File_List_In_Directory(dir, "*.ini", files, true);

    // Load everything from the top level directory first.
    for (auto it = files.begin(); it != files.end(); ++it) {
        // Create path string with initial dir stripped off.
        Utf8String path_check = &it->Str()[strlen(dir.Str())];

        if (strchr(path_check.Str(), '\\') == nullptr && strchr(path_check.Str(), '/') == nullptr) {
            Load(*it, type, xfer);
        }
    }

    // Now process everything from sub directories.
    for (auto it = files.begin(); it != files.end(); ++it) {
        // Create path string with initial dir stripped off.
        Utf8String path_check = &it->Str()[dir.Get_Length()];

        if (strchr(path_check.Str(), '\\') != nullptr || strchr(path_check.Str(), '/') != nullptr) {
            Load(*it, type, xfer);
        }
    }
}

void INI::Prep_File(Utf8String filename, INILoadType type)
{
    captainslog_relassert(m_backingFile == nullptr, 0xDEAD0006, "Cannot open file %s, file already open.", filename.Str());

    m_backingFile = g_theFileSystem->Open_File(filename.Str(), File::READ);

    captainslog_relassert(m_backingFile != nullptr, 0xDEAD0006, "Could not open file %s.", filename.Str());

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
    g_sXfer = nullptr;
}

void INI::Init_From_INI(void *what, const FieldParse *parse_table)
{
    MultiIniFieldParse p;

    p.Add(parse_table, 0);
    Init_From_INI_Multi(what, p);
}

void INI::Init_From_INI_Multi(void *what, const MultiIniFieldParse &parse_table_list)
{
    bool done = false;

    captainslog_relassert(what != nullptr, 0xDEAD0006, "Init_From_INI - Invalid parameters supplied.");

    while (!done) {
        captainslog_relassert(!Is_EOF(),
            0xDEAD0006,
            "Error parsing block '%s', in INI file '%s'.  Missing '%s' token",
            m_currentBlock,
            m_fileName.Str(),
            m_endToken);

        Read_Line();

        char *token = strtok(m_currentBlock, m_seps);

        if (token == nullptr) {
            continue;
        }

        if (strcasecmp(token, m_endToken) == 0) {
            done = true;
        } else {
            inifieldparse_t parsefunc;
            int offset;
            const void *data;
            int exoffset = 0;

            // Find an appropriate parser function from the parse table
            for (int i = 0;; ++i) {
                captainslog_relassert(i < parse_table_list.count,
                    0xDEAD0006,
                    "[LINE: %d - FILE: '%s'] Unknown field '%s' in block '%s'",
                    m_lineNumber,
                    m_fileName.Str(),
                    token,
                    m_currentBlock);

                parsefunc = Find_Field_Parse(parse_table_list.field_parsers[i], token, offset, data);

                if (parsefunc != nullptr) {
                    exoffset = parse_table_list.extra_offsets[i];

                    break;
                }
            }

            parsefunc(this, what, static_cast<char *>(what) + offset + exoffset, data);
        }
    }
}

void INI::Init_From_INI_Multi_Proc(void *what, void (*proc)(MultiIniFieldParse &))
{
    MultiIniFieldParse p;

    proc(p);
    Init_From_INI_Multi(what, p);
}

void INI::Read_Line()
{
    captainslog_dbgassert(m_backingFile != nullptr, "Read_Line file pointer is nullptr.");

    if (m_endOfFile) {
        m_currentBlock[0] = '\0';
    } else {
        // Read into our current block buffer.
        char *cb;
        for (cb = m_currentBlock; cb != &m_currentBlock[INI_MAX_CHARS_PER_LINE]; ++cb) {
            // If the buffer is empty, refill it.
            if (m_bufferReadPos == m_bufferData) {
                m_bufferReadPos = 0;
                m_bufferData = m_backingFile->Read(m_buffer, sizeof(m_buffer));

                if (m_bufferData == 0) {
                    m_endOfFile = true;

                    break;
                }
            }

            *cb = m_buffer[m_bufferReadPos++];

            // Reached end of line
            if (*cb == '\n') {
                break;
            }

            // Handle comment marker and none printing chars
            if (*cb == ';') {
                *cb = '\0';
            } else if (*cb > '\0' && *cb < ' ') {
                *cb = ' ';
            }
        }

        // Null terminate the buffer
        *cb = '\0';
        ++m_lineNumber;

        captainslog_dbgassert(cb != &m_currentBlock[INI_MAX_CHARS_PER_LINE],
            "Buffer too small (%d) and was truncated, increase INI_MAX_CHARS_PER_LINE",
            INI_MAX_CHARS_PER_LINE);
    }

    // If we have a transfer object assigned, do the transfer.
    if (g_sXfer != nullptr) {
        g_sXfer->xferImplementation(m_currentBlock, strlen(m_currentBlock));
    }
}

Utf8String INI::Get_Next_Quoted_Ascii_String() const
{
    const char *token = Get_Next_Token_Or_Null();
    Utf8String next;
    char buffer[INI_MAX_CHARS_PER_LINE];

    buffer[0] = '\0';

    if (token != nullptr) {
        if (*token == '"') {
            if (strlen(token) > 1) {
                strlcpy_tpl(buffer, token + 1);

                if (buffer[strlen(buffer) - 1] == '"') {
                    buffer[strlen(buffer) - 1] = '\0';

                    return buffer;
                }
            }

            const char *next_tok = Get_Next_Token(m_sepsQuote);

            if (strlen(next_tok) > 1 && next_tok[1] != '\t') {
                strlcat_tpl(buffer, " ");
                strlcat_tpl(buffer, next_tok);
            }

            if (buffer[strlen(buffer) - 1] == '"') {
                buffer[strlen(buffer) - 1] = '\0';
            }

            next = buffer;
        } else {
            next = token;
        }
    }

    return next;
}

ScienceType INI::Scan_Science(const char *token)
{
    return g_theScienceStore->Lookup_Science(token);
}

float INI::Scan_PercentToReal(const char *token)
{
    float value;
    int res = sscanf(token, "%f", &value);
    captainslog_relassert(res == 1, 0xDEAD0006, "Unable to parse percentage from token %s.", token);

    return (value / 100.0f);
}

float INI::Scan_Real(const char *token)
{
    float value;
    int res = sscanf(token, "%f", &value);
    captainslog_relassert(res == 1, 0xDEAD0006, "Unable to parse float from token %s.", token);

    return value;
}

unsigned int INI::Scan_UnsignedInt(const char *token)
{
    unsigned int value;
    int res = sscanf(token, "%u", &value);
    captainslog_relassert(res == 1, 0xDEAD0006, "Unable to parse unsigned int from token %s.", token);

    return value;
}

int INI::Scan_Int(const char *token)
{
    int value;
    int res = sscanf(token, "%d", &value);
    captainslog_relassert(res == 1, 0xDEAD0006, "Unable to parse int from token %s.", token);

    return value;
}

bool INI::Scan_Bool(const char *token)
{
    if (strcasecmp(token, "yes") == 0) {
        return true;
    } else {
        captainslog_relassert(strcasecmp(token, "no") == 0, 0xDEAD0006, "Invalid bool token %s, expected yes or no", token);
    }

    return false;
}

int INI::Scan_IndexList(const char *token, const char *const *list)
{
    captainslog_relassert(
        list != nullptr && *list != nullptr, 0xDEAD0006, "Error, invalid list provided for Scan_IndexList");

    auto *pstr = list;

    while (*pstr != nullptr) {
        if (strcasecmp(*pstr, token) == 0) {
            return pstr - list;
        }
        ++pstr;
    }

    captainslog_relassert(0, 0xDEAD0006, "Token %s not found in list", token);
    return pstr - list;
}

int INI::Scan_LookupList(const char *token, const LookupListRec *list)
{
    captainslog_relassert(
        list != nullptr && list->name != nullptr, 0xDEAD0006, "Error, invalid list provided for Scan_LookupList");

    int list_count = 0;

    while (strcasecmp(list[list_count].name, token) != 0) {
        ++list_count;
        captainslog_relassert(list[list_count].name != nullptr, 0xDEAD0006, "Token %s not found in list", token);
    }

    return list[list_count].value;
}

// Field parsing functions.
void INI::Parse_Bool(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<bool *>(store) = Scan_Bool(ini->Get_Next_Token());
}

void INI::Parse_Unsigned_Byte(INI *ini, void *formal, void *store, const void *user_data)
{
    int tmp = Scan_Int(ini->Get_Next_Token());

    captainslog_relassert(tmp >= 0 && tmp < 256, 0xDEAD0001, "Value parsed outside range of a byte.");

    *static_cast<uint8_t *>(store) = tmp;
}

void INI::Parse_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<int32_t *>(store) = Scan_Int(ini->Get_Next_Token());
}

void INI::Parse_Unsigned_Short(INI *ini, void *formal, void *store, const void *user_data)
{
    int tmp = Scan_Int(ini->Get_Next_Token());

    captainslog_relassert(tmp >= 0 && tmp < 65535, 0xDEAD0001, "Value parsed outside range of a short.");

    *static_cast<uint16_t *>(store) = tmp;
}

void INI::Parse_Unsigned_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<uint32_t *>(store) = Scan_UnsignedInt(ini->Get_Next_Token());
}

void INI::Parse_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Positive_Non_Zero_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    float tmp = Scan_Real(ini->Get_Next_Token());

    captainslog_relassert(tmp >= 0.0f, 0xDEAD0006, "Value parsed outside range of a positive none zero float.");

    *static_cast<float *>(store) = tmp;
}

void INI::Parse_Percent_To_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = Scan_PercentToReal(ini->Get_Next_Token(ini->m_sepsPercent));
}

void INI::Parse_Angle_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = Scan_Real(ini->Get_Next_Token()) * _ANGLE_MULTIPLIER;
}

void INI::Parse_Angular_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = Scan_Real(ini->Get_Next_Token()) * (_SECONDS_PER_LOGICFRAME_REAL_74 * _ANGLE_MULTIPLIER);
}

void INI::Parse_AsciiString(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<Utf8String *>(store) = ini->Get_Next_Ascii_String();
}

void INI::Parse_Quoted_AsciiString(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<Utf8String *>(store) = ini->Get_Next_Quoted_Ascii_String();
}

void INI::Parse_AsciiString_Vector(INI *ini, void *formal, void *store, const void *user_data)
{
    // DEBUG_LOG("Appending Vector for ini %s.", ini->m_fileName.Str());
    std::vector<Utf8String> *vec = static_cast<std::vector<Utf8String> *>(store);
    vec->clear();

    for (const char *i = ini->Get_Next_Token_Or_Null(); i != nullptr; i = ini->Get_Next_Token_Or_Null()) {
        vec->push_back(Utf8String(i));
    }
}

void INI::Parse_AsciiString_Vector_Append(INI *ini, void *formal, void *store, const void *user_data)
{
    // DEBUG_LOG("Appending Vector for ini %s.", ini->m_fileName.Str());
    std::vector<Utf8String> *vec = static_cast<std::vector<Utf8String> *>(store);

    for (const char *i = ini->Get_Next_Token_Or_Null(); i != nullptr; i = ini->Get_Next_Token_Or_Null()) {
        vec->push_back(Utf8String(i));
    }
}

void INI::Parse_RGB_Color(INI *ini, void *formal, void *store, const void *user_data)
{
    int colors[3];
    const char *names[3] = { "R", "G", "B" };
    RGBColor *rgb = static_cast<RGBColor *>(store);

    for (int i = 0; i < 3; ++i) {
        const char *token = ini->Get_Next_Sub_Token(names[i]);
        colors[i] = std::clamp(Scan_Int(token), 0, 255);
    }

    rgb->red = colors[0] / 255.0f;
    rgb->green = colors[1] / 255.0f;
    rgb->blue = colors[2] / 255.0f;
}

void INI::Parse_RGBA_Color_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    int colors[4];
    const char *names[4] = { "R", "G", "B", "A" };
    RGBAColorInt *rgba = static_cast<RGBAColorInt *>(store);

    for (int i = 0; i < 4; ++i) {
        const char *token = ini->Get_Next_Token_Or_Null(ini->m_sepsColon);

        if (token != nullptr) {
            captainslog_relassert(strcasecmp(token, names[i]) == 0,
                0xDEAD0006,
                "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.",
                token);

            colors[i] = std::clamp(Scan_Int(ini->Get_Next_Token(ini->m_sepsColon)), 0, 255);
        } else {
            captainslog_relassert(i == 3, 0xDEAD006, "Expected token missing, only 'A' token is optional for 'RBGA'.");
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
    uint32_t *rgba = static_cast<uint32_t *>(store);

    for (int i = 0; i < 4; ++i) {
        const char *token = ini->Get_Next_Token_Or_Null(ini->m_sepsColon);

        if (token != nullptr) {
            captainslog_relassert(strcasecmp(token, names[i]) == 0,
                0xDEAD0006,
                "Unexpected token '%s', expected one of 'R', 'G', 'B' or 'A'.",
                token);

            colors[i] = std::clamp(Scan_Int(ini->Get_Next_Token(ini->m_sepsColon)), 0, 255);
        } else {
            captainslog_relassert(i == 3, 0xDEAD006, "Expected token missing, only 'A' token is optional for 'RBGA'.");
            colors[i] = 255;
        }
    }

    *rgba = Make_Color(colors[0], colors[1], colors[2], colors[3]);
}

void INI::Parse_Coord2D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord2D *coord = static_cast<Coord2D *>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
}

void INI::Parse_Coord3D(INI *ini, void *formal, void *store, const void *user_data)
{
    Coord3D *coord = static_cast<Coord3D *>(store);

    coord->x = Scan_Real(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Real(ini->Get_Next_Sub_Token("Y"));
    coord->z = Scan_Real(ini->Get_Next_Sub_Token("Z"));
}

void INI::Parse_ICoord2D(INI *ini, void *formal, void *store, const void *user_data)
{
    ICoord2D *coord = static_cast<ICoord2D *>(store);

    coord->x = Scan_Int(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Int(ini->Get_Next_Sub_Token("Y"));
}

void INI::Parse_ICoord3D(INI *ini, void *formal, void *store, const void *user_data)
{
    ICoord3D *coord = static_cast<ICoord3D *>(store);

    coord->x = Scan_Int(ini->Get_Next_Sub_Token("X"));
    coord->y = Scan_Int(ini->Get_Next_Sub_Token("Y"));
    coord->z = Scan_Int(ini->Get_Next_Sub_Token("Z"));
}

void INI::Parse_Index_List(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<int *>(store) = Scan_IndexList(ini->Get_Next_Token(), static_cast<const char *const *>(user_data));
}

void INI::Parse_Byte_Sized_Index_List(INI *ini, void *formal, void *store, const void *user_data)
{
    int tmp = Scan_IndexList(ini->Get_Next_Token(), static_cast<const char *const *>(user_data));

    captainslog_relassert(tmp >= 0 && tmp < 256, 0xDEAD0001, "Value parsed outside range of a byte.");

    *static_cast<unsigned char *>(store) = tmp;
}
void INI::Parse_Duration_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = _DURATION_MULT * Scan_Real(ini->Get_Next_Token());
}

void INI::Parse_Duration_Unsigned_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<uint32_t *>(store) = Ceil(_DURATION_MULT * Scan_UnsignedInt(ini->Get_Next_Token()));
}

void INI::Parse_Duration_Unsigned_Short(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<uint16_t *>(store) = Ceil(_DURATION_MULT * Scan_UnsignedInt(ini->Get_Next_Token()));
}

void INI::Parse_Velocity_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) = Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Acceleration_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    *static_cast<float *>(store) =
        Scan_Real(ini->Get_Next_Token()) * _SECONDS_PER_LOGICFRAME_REAL_74 * _SECONDS_PER_LOGICFRAME_REAL_74;
}

void INI::Parse_Bit_In_Int32(INI *ini, void *formal, void *store, const void *user_data)
{
    uint32_t i = reinterpret_cast<uintptr_t>(user_data);

    if (Scan_Bool(ini->Get_Next_Token())) {
        *static_cast<int32_t *>(store) |= i;
    } else {
        *static_cast<int32_t *>(store) &= ~i;
    }
}

void INI::Parse_And_Translate_Label(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf16String *str = static_cast<Utf16String *>(store);
    *str = g_theGameText->Fetch(ini->Get_Next_Token());
}

void INI::Parse_Bitstring8(INI *ini, void *formal, void *store, const void *user_data)
{
    int bits;
    Parse_Bitstring32(ini, nullptr, &bits, user_data);

    captainslog_relassert((bits & ~255) == 0, CODE_01, "Bad bitstring list INI::parseBitString8");

    *static_cast<uint8_t *>(store) = bits;
}

void INI::Parse_Bitstring32(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *const *list = static_cast<const char *const *>(user_data);
    captainslog_relassert(list != nullptr && *list != nullptr, 0xDEAD0006, "No flag list provided.");
    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if (token != nullptr) {
        while (strcasecmp(token, "NONE") != 0) {
            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if (*token == '+') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                *static_cast<uint32_t *>(store) |= 1 << INI::Scan_IndexList(token + 1, list);
                adjust = true;
            } else if (*token == '-') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                *static_cast<uint32_t *>(store) &= ~(1 << INI::Scan_IndexList(token + 1, list));
                adjust = true;
            } else {
                captainslog_relassert(!adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                // If we haven't started setting yet, 0 it out.
                if (!set) {
                    *static_cast<uint32_t *>(store) = 0;
                }

                *static_cast<uint32_t *>(store) |= 1 << INI::Scan_IndexList(token, list);
                set = true;
            }

            token = ini->Get_Next_Token_Or_Null();

            if (token == nullptr) {
                return;
            }
        }

        // If we encounter a "NONE" entry, set all bits to 0.
        // We should never reach here if something has already been set.
        captainslog_relassert(!adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        *static_cast<uint32_t *>(store) = 0;
    }
}

void INI::Parse_Bitstring64(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *const *list = static_cast<const char *const *>(user_data);
    captainslog_relassert(list != nullptr && *list != nullptr, 0xDEAD0006, "No flag list provided.");
    const char *token = ini->Get_Next_Token_Or_Null();
    bool adjust = false;
    bool set = false;

    if (token != nullptr) {
        while (strcasecmp(token, "NONE") != 0) {
            // If we have a plus or minus as first char, then that decides if bit is set or cleared.
            // Otherwise just set.
            if (*token == '+') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                *static_cast<uint64_t *>(store) |= 1ll << INI::Scan_IndexList(token + 1, list);
                adjust = true;
            } else if (*token == '-') {
                captainslog_relassert(!set,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to adjust when we already set.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                *static_cast<uint64_t *>(store) &= ~(1ll << INI::Scan_IndexList(token + 1, list));
                adjust = true;
            } else {
                captainslog_relassert(!adjust,
                    0xDEAD0006,
                    "File: '%s', Line: %d Trying to set when we already adjusted.",
                    ini->Get_Filename().Str(),
                    ini->Get_Line_Number());

                // If we haven't started setting yet, 0 it out.
                if (!set) {
                    *static_cast<uint64_t *>(store) = 0;
                }

                *static_cast<uint64_t *>(store) |= 1ll << INI::Scan_IndexList(token, list);
                set = true;
            }

            token = ini->Get_Next_Token_Or_Null();

            if (token == nullptr) {
                return;
            }
        }

        // If we encounter a "NONE" entry, set all bits to 0.
        // We should never reach here if something has already been set.
        captainslog_relassert(!adjust && !set,
            0xDEAD0006,
            "File: '%s', Line: %d Trying to clear when we already set or adjusted.",
            ini->Get_Filename().Str(),
            ini->Get_Line_Number());

        *static_cast<uint64_t *>(store) = 0;
    }
}

// Was originally parseSpeakerType
void INI::Parse_Speaker_Type(INI *ini, void *formal, void *store, const void *user_data)
{
    Utf8String speaker;
    Parse_AsciiString(ini, formal, &speaker, user_data);
    *static_cast<SpeakerType *>(store) = static_cast<SpeakerType>(g_theAudio->Translate_From_Speaker_Type(speaker));
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

void INI::Parse_Sounds_List(INI *ini, void *formal, void *store, const void *user_data)
{
    std::vector<Utf8String> *sound_vec = static_cast<std::vector<Utf8String> *>(store);
    sound_vec->clear();

    for (const char *token = ini->Get_Next_Token_Or_Null(" \t,="); token != nullptr;
         token = ini->Get_Next_Token_Or_Null(" \t,=")) {
        sound_vec->push_back(token);
    }
}

void INI::Parse_Dynamic_Audio_Event_RTS(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *str = ini->Get_Next_Token();
    DynamicAudioEventRTS **audio = static_cast<DynamicAudioEventRTS **>(store);

    if (!strcasecmp(str, "NoSound")) {
        if (*audio) {
            (*audio)->Delete_Instance();
            *audio = nullptr;
        }
    } else {
        if (!*audio) {
            *audio = NEW_POOL_OBJ(DynamicAudioEventRTS);
        }

        (*audio)->m_event.Set_Event_Name(str);
    }

    if (*audio) {
        g_theAudio->Get_Info_For_Audio_Event(&(*audio)->m_event);
    }
}

void INI::Parse_Lookup_List(INI *ini, void *formal, void *store, const void *user_data)
{
    *reinterpret_cast<int *>(store) =
        ini->Scan_LookupList(ini->Get_Next_Token(), reinterpret_cast<const LookupListRec *>(user_data));
}

void INI::Parse_Object_Definition(INI *ini)
{
    Utf8String name = ini->Get_Next_Token();
    ThingFactory::Parse_Object_Definition(ini, name, Utf8String::s_emptyString);
}

void INI::Parse_Object_Reskin_Definition(INI *ini)
{
    Utf8String name = ini->Get_Next_Token();
    Utf8String reskin = ini->Get_Next_Token();
    ThingFactory::Parse_Object_Definition(ini, name, reskin);
}

bool INI::Is_Declaration_Of_Type(Utf8String block_type, Utf8String block_name, char *buffer_to_check)
{
    bool ret = true;

    if (buffer_to_check == nullptr || block_type.Is_Empty() || block_name.Is_Empty()) {
        return false;
    } else {
        char *buffer = buffer_to_check;
        int block_type_len = block_type.Get_Length();
        int block_name_len = block_name.Get_Length();

        while (isspace(*buffer)) {
            buffer++;
        }

        if (strlen(buffer) <= static_cast<size_t>(block_type_len)) {
            ret = false;
        } else {
            char c = buffer[block_type_len];
            buffer[block_type_len] = '\0';

            if (strcasecmp(block_type.Str(), buffer) != 0) {
                ret = false;
            }

            buffer[block_type_len] = c;
            buffer += block_type_len;
        }

        while (isspace(*buffer)) {
            buffer++;
        }

        if (strlen(buffer) <= static_cast<size_t>(block_name_len)) {
            ret = false;
        } else {
            char c = buffer[block_name_len];
            buffer[block_name_len] = '\0';

            if (strcasecmp(block_name.Str(), buffer) != 0) {
                ret = false;
            }

            buffer[block_name_len] = c;
            buffer += block_name_len;
        }

        while (strlen(buffer) != 0) {
            ret = ret && isspace(*buffer);
            buffer++;
        }

        return ret;
    }
}

bool INI::Is_End_Of_Block(char *buffer_to_check)
{
    static const char *end_string = "End";
    bool ret = false;

    if (buffer_to_check == nullptr) {
        return false;
    }

    size_t end_string_len = strlen(end_string);
    char *buffer = buffer_to_check;

    while (isspace(*buffer)) {
        buffer++;
    }

    if (strlen(buffer) <= end_string_len) {
        ret = false;
    } else {
        char c = buffer[end_string_len];
        buffer[end_string_len] = '\0';

        if (strcasecmp(end_string, buffer) != 0) {
            ret = false;
        }

        buffer[end_string_len] = c;
        buffer += end_string_len;
    }

    while (strlen(buffer) != 0) {
        ret = ret && isspace(*buffer);
        buffer++;
    }

    return ret;
}

void INI::Parse_Thing_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    captainslog_relassert(g_theThingFactory != nullptr, CODE_01, "TheThingFactory not inited yet");
    ThingTemplate **tmplate = static_cast<ThingTemplate **>(store);

    if (strcasecmp(name, "None") == 0) {
        *tmplate = nullptr;
    } else {
        ThingTemplate *thing_template = g_theThingFactory->Find_Template(name, true);
        captainslog_dbgassert(thing_template != nullptr, "ThingTemplate %s not found!", name);
        *tmplate = thing_template;
    }
}

void INI::Parse_Upgrade_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    captainslog_relassert(g_theUpgradeCenter != nullptr, CODE_01, "TheUpgradeCenter not inited yet");
    const UpgradeTemplate **tmplate = static_cast<const UpgradeTemplate **>(store);
    const UpgradeTemplate *upgrade_template = g_theUpgradeCenter->Find_Upgrade(name);
    captainslog_dbgassert(upgrade_template != nullptr || strcasecmp(name, "None") == 0, "Upgrade %s not found!", name);
    *tmplate = upgrade_template;
}

void INI::Parse_Special_Power_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    captainslog_relassert(g_theSpecialPowerStore != nullptr, CODE_01, "TheSpecialPowerStore not inited yet");
    const SpecialPowerTemplate **tmplate = static_cast<const SpecialPowerTemplate **>(store);
    const SpecialPowerTemplate *special_power_template = g_theSpecialPowerStore->Find_Special_Power_Template(name);
    captainslog_dbgassert(special_power_template != nullptr || strcasecmp(name, "None") == 0,
        "[LINE: %d in '%s'] Specialpower %s not found!",
        ini->Get_Line_Number(),
        ini->Get_Filename().Str(),
        name);
    *tmplate = special_power_template;
}

void INI::Parse_Webpage_URL_Definition(INI *ini)
{
    captainslog_relassert(false,
        0,
        "Web Browser class not implemented because it relies on old Internet Explorer code that isn't present in the "
        "versions of Windows we run on");
}

void INI::Parse_Veterancy_Level_Flags(INI *ini, void *formal, void *store, const void *user_data)
{
    int flags = -1;

    for (const char *flag = ini->Get_Next_Token(); flag != nullptr; flag = ini->Get_Next_Token_Or_Null()) {
        if (strcasecmp(flag, "ALL") == 0) {
            flags = -1;
        } else if (strcasecmp(flag, "NONE") == 0) {
            flags = 0;
        } else if (flag[0] == '+') {
            int vet_level = ini->Scan_IndexList(flag + 1, g_veterancyNames);
            flags |= (1 << (vet_level - 1));
        } else {
            if (flag[0] != '-') {
                throw CODE_06;
            }

            int vet_level = ini->Scan_IndexList(flag + 1, g_veterancyNames);
            flags &= ~(1 << (vet_level - 1));
        }
    }

    *static_cast<int *>(store) = flags;
}

void INI::Parse_Damage_Type_Flags(INI *ini, void *formal, void *store, const void *user_data)
{
    BitFlags<DAMAGE_NUM_TYPES> flags = DAMAGE_TYPE_FLAGS_NONE;
    flags.Flip();

    for (const char *flag = ini->Get_Next_Token(); flag != nullptr; flag = ini->Get_Next_Token_Or_Null()) {
        if (strcasecmp(flag, "ALL") == 0) {
            flags = DAMAGE_TYPE_FLAGS_NONE;
            flags.Flip();
        } else if (strcasecmp(flag, "NONE") == 0) {
            flags = DAMAGE_TYPE_FLAGS_NONE;
        } else if (flag[0] == '+') {
            flags =
                BitFlags<DAMAGE_NUM_TYPES>::Set_Bit(flags, BitFlags<DAMAGE_NUM_TYPES>::Get_Single_Bit_From_Name(flag + 1));
        } else {
            if (flag[0] != '-') {
                throw CODE_06;
            }

            flags =
                BitFlags<DAMAGE_NUM_TYPES>::Clear_Bit(flags, BitFlags<DAMAGE_NUM_TYPES>::Get_Single_Bit_From_Name(flag + 1));
        }
    }

    *static_cast<BitFlags<DAMAGE_NUM_TYPES> *>(store) = flags;
}

static const char *s_theDeathTypeNames[] = {
    "NORMAL",
    "NONE",
    "CRUSHED",
    "BURNED",
    "EXPLODED",
    "POISONED",
    "TOPPLED",
    "FLOODED",
    "SUICIDED",
    "LASERED",
    "DETONATED",
    "SPLATTED,",
    "POISONED_BETA",
    "EXTRA_2",
    "EXTRA_3",
    "EXTRA_4",
    "EXTRA_5",
    "EXTRA_6",
    "EXTRA_7",
    "EXTRA_8",
    "POISONED_GAMMA",
    nullptr,
};

void INI::Parse_Death_Type_Flags(INI *ini, void *formal, void *store, const void *user_data)
{
    int flags = -1;

    for (const char *flag = ini->Get_Next_Token(); flag != nullptr; flag = ini->Get_Next_Token_Or_Null()) {
        if (strcasecmp(flag, "ALL") == 0) {
            flags = -1;
        } else if (strcasecmp(flag, "NONE") == 0) {
            flags = 0;
        } else if (flag[0] == '+') {
            int death_type = ini->Scan_IndexList(flag + 1, s_theDeathTypeNames);
            flags |= (1 << (death_type - 1));
        } else {
            if (flag[0] != '-') {
                throw CODE_06;
            }

            int death_type = ini->Scan_IndexList(flag + 1, s_theDeathTypeNames);
            flags &= ~(1 << (death_type - 1));
        }
    }

    *static_cast<int *>(store) = flags;
}
