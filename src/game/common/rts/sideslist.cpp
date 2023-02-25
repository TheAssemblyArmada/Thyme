/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Classes for managing sides.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "sideslist.h"
#include "staticnamekey.h"
#include "unicodestring.h"
#include "xfer.h"
#include <captainslog.h>

#ifdef GAME_DLL
#else
SidesList *g_theSidesList = nullptr;
#endif

/**
 * @brief Resets this subsystem.
 *
 * 0x004D6100
 */
void SidesList::Reset()
{
    Clear();
    m_teamRec.Clear();
    m_skirmishTeamsRec.Clear();
}

/**
 * @brief Uses the passed Xfer object to perform transfer of this sides list.
 *
 * 0x004D85F0
 */
void SidesList::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    int32_t sides = m_numSides;
    xfer->xferInt(&sides);
    captainslog_relassert(
        sides == m_numSides, 6, "Transferred sides %d count does not match m_numSides %d.", sides, m_numSides);

    for (int i = 0; i < sides; ++i) {
        captainslog_relassert(
            i < m_numSides, 0xDEAD0003, "Attempting to process a side with a higher value than m_numSides.");
        bool has_scripts = m_sides[i].Get_ScriptList() != nullptr;
        xfer->xferBool(&has_scripts);

        if (m_sides[i].Get_ScriptList() != nullptr) {
            captainslog_relassert(has_scripts, 6, "We have a script list, but has_scripts is not set.");
            // Transfer script class.
            xfer->xferSnapshot(m_sides[i].Get_ScriptList());
        }
    }
}

/**
 * @brief Clears the existing data.
 *
 * 0x004D77A0
 */
void SidesList::Clear()
{
    m_numSides = 0;
    m_numSkirmishSides = 0;

    for (int i = 0; i < MAX_SIDE_COUNT; ++i) {
        m_sides[i].Init();
        m_skirmishSides[i].Init();
    }
}

/**
 * @brief Adds a side to the list.
 */
void SidesList::Add_Side(const Dict *dict)
{
    if (m_numSides < MAX_SIDE_COUNT) {
        m_sides[m_numSides++].Init(dict);
    }
}

/**
 * @brief Empties all teams from the list.
 */
void SidesList::Empty_Teams()
{
    m_teamRec.Clear();
    m_skirmishTeamsRec.Clear();
}

/**
 * @brief Empties all sides from the list.
 */
void SidesList::Empty_Sides()
{
    for (int i = 0; i < MAX_SIDE_COUNT; ++i) {
        m_sides[i].Init();
        m_skirmishSides[i].Init();
    }
}

/**
 * @brief Validates sides in the list.
 *
 * 0x004D7F70
 */
bool SidesList::Validate_Sides()
{
    bool modified = false;

    int index;
    for (index = 0; index < m_numSides; ++index) {
        SidesInfo *info = Get_Sides_Info(index);
        if (info->Get_Dict().Get_AsciiString(g_playerNameKey).Is_Empty()) {
            break;
        }
    }

    if (index >= m_numSides) {
        Add_Player_By_Template("");
        modified = true;
    }

    // Validates side info.
    for (index = 0; index < m_numSides; ++index) {
        SidesInfo *info = Get_Sides_Info(index);
        Utf8String player_name = info->Get_Dict().Get_AsciiString(g_playerNameKey);
        Utf8String team_name = "team";
        team_name += player_name;
        TeamsInfo *team_info = Find_Team_Info(team_name, nullptr);

        if (team_info != nullptr) {
            if (team_info->Get_Dict()->Get_AsciiString(g_teamOwnerKey) != player_name) {
                team_info->Get_Dict()->Set_AsciiString(g_teamOwnerKey, player_name);
                modified = true;
            }

            if (!team_info->Get_Dict()->Get_Bool(g_teamIsSingletonKey)) {
                team_info->Get_Dict()->Set_Bool(g_teamIsSingletonKey, true);
                modified = true;
            }
        } else {
            Dict dict;
            dict.Set_AsciiString(g_teamNameKey, team_name);
            dict.Set_AsciiString(g_teamOwnerKey, player_name);
            dict.Set_Bool(g_teamIsSingletonKey, true);
            Add_Team(&dict);
            modified = true;
        }

        Utf8String allies = info->Get_Dict().Get_AsciiString(g_playerAlliesKey);
        Utf8String enemies = info->Get_Dict().Get_AsciiString(g_playerEnemiesKey);

        // Allies and enemies are whitespace separated lists.
        if (Validate_Ally_Enemy_List(player_name, allies)) {
            info->Get_Dict().Set_AsciiString(g_playerAlliesKey, allies);
        }

        if (Validate_Ally_Enemy_List(player_name, enemies)) {
            info->Get_Dict().Set_AsciiString(g_playerAlliesKey, enemies);
        }
    }

    // Validates team names.
    for (index = 0; index < m_teamRec.Get_Num_Teams(); ++index) {
        TeamsInfo *tinfo = m_teamRec.Get_Team_Info(index);
        Utf8String team_name = tinfo->Get_Dict()->Get_AsciiString(g_teamNameKey);

        if (Find_Side_Info(team_name) != nullptr) {
            captainslog_warn("Duplicate name '%s' between player and team, removing...", team_name.Str());
            Remove_Team(index);
            modified = true;

            // Restart the loop from the beginning.
            index = -1;
            continue;
        } else {
            // TODO
        }
    }

    // Validates team owner
    for (index = 0; index < m_teamRec.Get_Num_Teams(); ++index) {
        TeamsInfo *tinfo = m_teamRec.Get_Team_Info(index);
        Utf8String team_name = tinfo->Get_Dict()->Get_AsciiString(g_teamNameKey);
        Utf8String team_owner = tinfo->Get_Dict()->Get_AsciiString(g_teamOwnerKey);

        if (Find_Side_Info(team_owner) == nullptr || team_name == team_owner) {
            captainslog_warn("Bad owner '%s', reset to neutral.", team_owner.Str());
            tinfo->Get_Dict()->Set_AsciiString(g_teamOwnerKey, Utf8String::s_emptyString);
            modified = true;
        }
    }

    return modified;
}

/**
 * @brief Validates an ally or enemy list.
 *
 * 0x004D79A0
 */
bool SidesList::Validate_Ally_Enemy_List(const Utf8String &team, Utf8String &allies)
{
    bool modified = false;
    Utf8String str = allies;
    Utf8String token;
    Utf8String new_str;

    // Goes through string constructing a new space separated list from the old one, skipping ones that don't match existing
    // side info.
    while (str.Next_Token(&token)) {
        if (team != token && Find_Side_Info(token) != nullptr) {
            if (!new_str.Is_Empty()) {
                new_str += " ";
            }

            new_str += token;
        } else {
            modified = true;
        }
    }

    allies = new_str;

    return modified;
}

/**
 * @brief Adds a template player.
 *
 * 0x004D7BC0
 */
void SidesList::Add_Player_By_Template(Utf8String template_name)
{
    Utf8String player_name;
    Utf16String display_name;
    bool is_human;

    if (template_name.Is_Empty()) {
        player_name = "";
        display_name = U_CHAR("Neutral");
        is_human = false;
    } else {
        player_name = "Plyr";

        if (template_name.Starts_With("Faction")) {
            player_name += template_name.Str() + strlen("Faction");
        } else {
            player_name += template_name;
        }

        display_name = player_name;
        is_human = player_name == "PlyrCivilian" ? false : true;
    }

    Dict dict;
    dict.Set_AsciiString(g_playerNameKey, player_name);
    dict.Set_Bool(g_playerIsHumanKey, is_human);
    dict.Set_UnicodeString(g_playerDisplayNameKey, display_name);
    dict.Set_AsciiString(g_playerFactionKey, template_name);
    dict.Set_AsciiString(g_playerAlliesKey, "");
    dict.Set_AsciiString(g_playerEnemiesKey, "");
    Add_Side(&dict);
    dict.Clear();

    Utf8String team_name = "team";
    team_name += player_name;
    dict.Set_AsciiString(g_teamNameKey, team_name);
    dict.Set_AsciiString(g_teamOwnerKey, player_name);
    dict.Set_Bool(g_teamIsSingletonKey, true);
    Add_Team(&dict);
}

/**
 * @brief Finds side info matching the provided name.
 *
 * 0x004D6A60
 */
SidesInfo *SidesList::Find_Side_Info(Utf8String name, int *index)
{
    for (int i = 0; i < m_numSides; ++i) {
        if (m_sides[i].Get_Dict().Get_AsciiString(g_playerNameKey) == name) {
            if (index != nullptr) {
                *index = i;
            }

            return &m_sides[i];
        }
    }

    return nullptr;
}

/**
 * @brief Finds skirmish side info matching the provided name.
 *
 * 0x004D6BD0
 */
SidesInfo *SidesList::Find_Skirmish_Side_Info(Utf8String name, int *index)
{
    for (int i = 0; i < m_numSkirmishSides; ++i) {
        if (m_skirmishSides[i].Get_Dict().Get_AsciiString(g_playerNameKey) == name) {
            if (index != nullptr) {
                *index = i;
            }

            return &m_skirmishSides[i];
        }
    }

    return nullptr;
}

/**
 * @brief Retrieves side info at the given index.
 */
SidesInfo *SidesList::Get_Sides_Info(int index)
{
    captainslog_relassert(index >= 0 && index < m_numSides, 0xDEAD0003, "Index out of bounds for side info.");

    return &m_sides[index];
}

/**
 * @brief Parses a sides list chunk from a data chunk stream.
 *
 * 0x004D62A0
 */
bool SidesList::Parse_Sides_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data)
{
    if (g_theSidesList == nullptr) {
        return false;
    }

    g_theSidesList->Reset();
    g_theSidesList->Empty_Sides();
    int side_count = input.Read_Int32();

    for (int i = 0; i < side_count; ++i) {
        Dict read_dict = input.Read_Dict();
        g_theSidesList->Add_Side(&read_dict);

        int build_info_count = input.Read_Int32();

        for (int j = 0; j < build_info_count; ++j) {
            BuildListInfo *build_info = NEW_POOL_OBJ(BuildListInfo);
            build_info->Parse_Data_Chunk(input, info);
            g_theSidesList->m_sides[i].Add_To_BuildList(build_info, j);
        }
    }

    if (info->version >= 2) {
        int team_count = input.Read_Int32();
        g_theSidesList->Empty_Teams();

        for (int i = 0; i < team_count; ++i) {
            Dict team_dict = input.Read_Dict();
            g_theSidesList->m_teamRec.Add_Team(&team_dict);
        }
    }

    input.Register_Parser("PlayerScriptsList", info->label, ScriptList::Parse_Scripts_Chunk, nullptr);
    bool tmp = input.Parse(nullptr);
    captainslog_relassert(tmp, 0xDEAD0005, "Parsing script chunk failed.");

    ScriptList *scripts[MAX_LIST_COUNT];
    int script_count = ScriptList::Get_Read_Scripts(scripts);

    for (int i = 0; i < script_count; ++i) {
        if (i >= g_theSidesList->m_numSides) {
            scripts[i]->Delete_Instance();
            scripts[i] = nullptr;
        } else {
            if (g_theSidesList->m_sides[i].Get_ScriptList() != nullptr) {
                g_theSidesList->m_sides[i].Get_ScriptList()->Delete_Instance();
            }

            g_theSidesList->m_sides[i].Set_ScriptList(scripts[i]);
        }
    }

    g_theSidesList->Validate_Sides();

    return true;
}
