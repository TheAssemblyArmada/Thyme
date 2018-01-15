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
#include "gamedebug.h"
#include "staticnamekey.h"
#include "unicodestring.h"
#include "xfer.h"

#ifndef THYME_STNADALONE
SidesList *&g_theSidesList = Make_Global<SidesList *>(0x00A2BE3C);
#else
SidesList *g_theSidesList = nullptr;
#endif

BuildListInfo::BuildListInfo() :
    m_buildingName(),
    m_templateName(),
    m_location{0.0f, 0.0f, 0.0f},
    m_rallyPointOffset{0.0f, 0.0f},
    m_angle(0.0f),
    m_isInitiallyBuilt(false),
    m_numRebuilds(0),
    m_nextBuildList(nullptr),
    m_script(),
    m_health(100),
    m_whiner(true),
    m_repairable(false),
    m_sellable(true),
    m_autoBuild(true),
    m_renderObj(nullptr),
    m_shadowObj(nullptr),
    m_selected(false),
    m_objectID(OBJECT_UNK),
    m_objectTimestamp(0),
    m_underConstruction(false),
    m_unkbool3(false),
    m_unkint1(0),
    m_unkint2(0),
    m_unkbool4(false)
{
    memset(m_unkArray, 0, sizeof(m_unkArray));
}

BuildListInfo::~BuildListInfo()
{
    for (BuildListInfo *next = m_nextBuildList, *saved = nullptr; next != nullptr; next = saved) {
        saved = next->m_nextBuildList;
        Delete_Instance(next);
        next = saved;
    }
}

void BuildListInfo::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 2;
    xfer->xferVersion(&version, 2);
    xfer->xferAsciiString(&m_buildingName);
    xfer->xferAsciiString(&m_templateName);
    xfer->xferCoord3D(&m_location);
    xfer->xferCoord2D(&m_rallyPointOffset);
    xfer->xferReal(&m_angle);
    xfer->xferBool(&m_isInitiallyBuilt);
    xfer->xferUnsignedInt(&m_numRebuilds);
    xfer->xferAsciiString(&m_script);
    xfer->xferInt(&m_health);
    xfer->xferBool(&m_whiner);
    xfer->xferBool(&m_repairable);
    xfer->xferBool(&m_sellable);
    xfer->xferBool(&m_autoBuild);
    xfer->xferObjectID(&m_objectID);
    xfer->xferUnsignedInt(&m_objectTimestamp);
    xfer->xferBool(&m_underConstruction);
    xfer->xferUser(m_unkArray, sizeof(m_unkArray));
    xfer->xferBool(&m_unkbool3);
    xfer->xferInt(&m_unkint1);
    xfer->xferBool(&m_unkbool4);

    if (version >= 2) {
        xfer->xferInt(&m_unkint2);
    }
}

BuildListInfo &BuildListInfo::operator=(const BuildListInfo &that)
{
    if (this != &that) {
        m_buildingName = that.m_buildingName;
        m_templateName = that.m_templateName;
        m_location = that.m_location;
        m_rallyPointOffset = that.m_rallyPointOffset;
        m_angle = that.m_angle;
        m_isInitiallyBuilt = that.m_isInitiallyBuilt;
        m_numRebuilds = that.m_numRebuilds;
        m_nextBuildList = that.m_nextBuildList;
        m_script = that.m_script;
        m_health = that.m_health;
        m_whiner = that.m_whiner;
        m_repairable = that.m_repairable;
        m_sellable = that.m_sellable;
        m_autoBuild = that.m_autoBuild;
        m_renderObj = that.m_renderObj;
        m_shadowObj = that.m_shadowObj;
        m_selected = that.m_selected;
        m_objectID = that.m_objectID;
        m_objectTimestamp = that.m_objectTimestamp;
        m_underConstruction = that.m_underConstruction;
        memcpy(m_unkArray, that.m_unkArray, sizeof(m_unkArray));
        m_unkbool3 = that.m_unkbool3;
        m_unkint1 = that.m_unkint1;
        m_unkint2 = that.m_unkint2;
        m_unkbool4 = that.m_unkbool4;
    }

    return *this;
}

void BuildListInfo::Parse_Data_Chunk(DataChunkInput &input, DataChunkInfo *info)
{
    m_buildingName = input.Read_AsciiString();
    m_templateName = input.Read_AsciiString();
    m_location.x = input.Read_Real32();
    m_location.y = input.Read_Real32();
    m_location.z = input.Read_Real32();
    m_angle = input.Read_Real32();
    m_isInitiallyBuilt = input.Read_Byte();
    m_numRebuilds = input.Read_Int32();

    if (info->version >= 3) {
        m_script = input.Read_AsciiString();
        m_health = input.Read_Int32();
        m_whiner = input.Read_Byte();
        m_repairable = input.Read_Byte();
        m_sellable = input.Read_Byte();
    }
}

SidesInfo::SidesInfo(const SidesInfo &that) : m_buildList(nullptr), m_dict(0), m_scripts(nullptr)
{
    *this = that;
}

SidesInfo::~SidesInfo()
{
    Init();
    m_dict.Release_Data();
}

void SidesInfo::Init(const Dict *dict)
{
    Delete_Instance(m_buildList);
    m_buildList = nullptr;
    m_dict.Init(dict);
    Delete_Instance(m_scripts);
    m_scripts = nullptr;
}

void SidesInfo::Add_To_BuildList(BuildListInfo *list, int pos)
{
    BuildListInfo *position = nullptr;
    BuildListInfo *build_list = m_buildList;

    for (int i = pos; i > 0; --i) {
        if (build_list == nullptr) {
            break;
        }

        position = build_list;
        build_list = build_list->Get_Next();
    }

    if (position != nullptr) {
        list->Set_Next(position->Get_Next());
        position->Set_Next(list);
    } else {
        list->Set_Next(m_buildList);
        m_buildList = list;
    }
}

int SidesInfo::Remove_From_BuildList(BuildListInfo *list)
{
    if (list == nullptr) {
        return 0;
    }

    if (list == m_buildList) {
        m_buildList = list->Get_Next();
        list->Set_Next(nullptr);

        return 0;
    }

    int pos = 1;

    for (BuildListInfo *next = m_buildList; next != nullptr; next = next->Get_Next()) {
        if (list == next->Get_Next()) {
            next->Set_Next(list->Get_Next());

            break;
        }

        ++pos;
    }

    list->Set_Next(nullptr);

    return pos;
}

void SidesInfo::Reorder_In_BuildList(BuildListInfo *list, int pos)
{
    Remove_From_BuildList(list);
    Add_To_BuildList(list, pos);
}

SidesInfo &SidesInfo::operator=(const SidesInfo &that)
{
    if (this != &that) {
        Init();
        m_dict = that.m_dict;

        // Copy the build list.
        for (BuildListInfo *next = that.m_buildList, *last = nullptr; next != nullptr; next = next->Get_Next()) {
            BuildListInfo *new_list = new BuildListInfo;
            *new_list = *next;

            if (last == nullptr) {
                m_buildList = new_list;
            } else {
                last->Set_Next(new_list);
            }

            last = new_list;
        }

        if (that.m_scripts != nullptr) {
            m_scripts = that.m_scripts->Duplicate();
        }
    }

    return *this;
}

void SidesList::Reset()
{
    Clear();
    m_teamRec.Clear();
    m_skirmishTeamsRec.Clear();
}

void SidesList::Xfer_Snapshot(Xfer *xfer)
{
    uint8_t version = 1;
    xfer->xferVersion(&version, 1);
    int32_t sides = m_numSides;
    xfer->xferInt(&sides);
    ASSERT_THROW(sides == m_numSides, 6);

    for (int i = 0; i < sides; ++i) {
        ASSERT_THROW(i < m_numSides, 0xDEAD0003);
        bool has_scripts = m_sides[i].Get_ScriptList() != nullptr;
        xfer->xferBool(&has_scripts);

        if (m_sides[i].Get_ScriptList() != nullptr) {
            ASSERT_THROW(has_scripts, 6);
            // Transfer script class.
            xfer->xferSnapshot(m_sides[i].Get_ScriptList()->Get_Scripts());
        }
    }
}

void SidesList::Clear()
{
    m_numSides = 0;
    m_numSkirmishSides = 0;

    for (int i = 0; i < MAX_SIDE_COUNT; ++i) {
        m_sides[i].Init();
        m_skirmishSides[i].Init();
    }
}

void SidesList::Add_Side(const Dict *dict)
{
    if (m_numSides < MAX_SIDE_COUNT) {
        m_sides[m_numSides++].Init(dict);
    }
}

void SidesList::Empty_Teams()
{
    m_teamRec.Clear();
    m_skirmishTeamsRec.Clear();
}

void SidesList::Empty_Sides()
{
    for (int i = 0; i < MAX_SIDE_COUNT; ++i) {
        m_sides[i].Init();
        m_skirmishSides[i].Init();
    }
}

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
        AsciiString player_name = info->Get_Dict().Get_AsciiString(g_playerNameKey);
        AsciiString team_name = "team";
        team_name += player_name;
        TeamsInfo *team_info = Find_Team_Info(team_name, nullptr);

        if (team_info != nullptr) {
            if (team_info->dict.Get_AsciiString(g_teamOwnerKey) != player_name) {
                team_info->dict.Set_AsciiString(g_teamOwnerKey, player_name);
                modified = true;
            }

            if (!team_info->dict.Get_Bool(g_teamIsSingletonKey)) {
                team_info->dict.Set_Bool(g_teamIsSingletonKey, true);
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

        AsciiString allies = info->Get_Dict().Get_AsciiString(g_playerAlliesKey);
        AsciiString enemies = info->Get_Dict().Get_AsciiString(g_playerEnemiesKey);

        // Allies and enemies are whitespace separated lists.
        if (Validate_Ally_Enemy_List(player_name, allies)) {
            info->Get_Dict().Set_AsciiString(g_playerAlliesKey, allies);
        }

        if (Validate_Ally_Enemy_List(player_name, enemies)) {
            info->Get_Dict().Set_AsciiString(g_playerAlliesKey, enemies);
        }
    }

    // Validates team names.
    for (index = 0; index < m_teamRec.Count(); ++index) {
        TeamsInfo *tinfo = m_teamRec.Get_Team_Info(index);
        AsciiString team_name = tinfo->dict.Get_AsciiString(g_teamNameKey);

        if (Find_Side_Info(team_name) == nullptr) {
            DEBUG_LOG("Duplicate name '%s' between player and team, removing...\n", team_name.Str());
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
    for (index = 0; index < m_teamRec.Count(); ++index) {
        TeamsInfo *tinfo = m_teamRec.Get_Team_Info(index);
        AsciiString team_name = tinfo->dict.Get_AsciiString(g_teamNameKey);
        AsciiString team_owner = tinfo->dict.Get_AsciiString(g_teamOwnerKey);

        if (!Find_Side_Info(team_owner) || team_name == team_owner) {
            DEBUG_LOG("Bad owner '%s', reset to neutral.\n", team_owner.Str());
            tinfo->dict.Set_AsciiString(g_teamOwnerKey, "");
            modified = true;
        }
    }

    return modified;
}

bool SidesList::Validate_Ally_Enemy_List(const AsciiString &team, AsciiString &allies)
{
    bool modified = false;
    AsciiString str = allies;
    AsciiString token;
    AsciiString new_str;

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

void SidesList::Add_Player_By_Template(AsciiString template_name)
{
    AsciiString player_name;
    UnicodeString display_name;
    bool is_human;

    if (template_name.Is_Empty()) {
        player_name = "";
        display_name = L"Neutral";
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

    AsciiString team_name = "team";
    team_name += player_name;
    dict.Set_AsciiString(g_teamNameKey, team_name);
    dict.Set_AsciiString(g_teamOwnerKey, player_name);
    dict.Set_Bool(g_teamIsSingletonKey, true);
    Add_Team(&dict);
}

SidesInfo *SidesList::Find_Side_Info(AsciiString name, int *index)
{
    for (int i = 0; i < m_numSides; ++i) {
        if (m_sides[i].Get_Dict().Get_AsciiString(g_playerNameKey) == name) {
            if (index != nullptr) {
                *index = i;

                return &m_sides[i];
            }
        }
    }

    return nullptr;
}

SidesInfo *SidesList::Find_Skirmish_Side_Info(AsciiString name, int *index)
{
    for (int i = 0; i < m_numSkirmishSides; ++i) {
        if (m_skirmishSides[i].Get_Dict().Get_AsciiString(g_playerNameKey) == name) {
            if (index != nullptr) {
                *index = i;

                return &m_skirmishSides[i];
            }
        }
    }

    return nullptr;
}

SidesInfo *SidesList::Get_Sides_Info(int index)
{
    ASSERT_THROW_PRINT(index >= 0 && index < m_numSides, 0xDEAD0003, "Index out of bounds for side info.");

    return &m_sides[index];
}

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
            BuildListInfo *build_info = new BuildListInfo;
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
    ASSERT_THROW_PRINT(input.Parse(nullptr), 0xDEAD0005, "Parsing script chunk failed.\n");

    ScriptList *scripts[MAX_LIST_COUNT];
    int script_count = ScriptList::Get_Read_Scripts(scripts);

    for (int i = 0; i < script_count; ++i) {
        if (i >= g_theSidesList->m_numSides) {
            Delete_Instance(scripts[i]);
            scripts[i] = nullptr;
        } else {
            if (g_theSidesList->m_sides[i].Get_ScriptList() != nullptr) {
                Delete_Instance(g_theSidesList->m_sides[i].Get_ScriptList());
            }

            g_theSidesList->m_sides[i].Set_ScriptList(scripts[i]);
        }
    }

    g_theSidesList->Validate_Sides();

    return true;
}
