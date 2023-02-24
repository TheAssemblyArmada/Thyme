/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for tracking player information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "player.h"
#include "buildinfo.h"
#include "namekeygenerator.h"
#include "squad.h"
#include "team.h"
#include <algorithm>

// zh: 0x0044F7B0, wb: 0x0085960A
Player::Player(int32_t player_index) :
    m_playerIsPreorder(false),
    m_playerIsDead(false),
    m_playerIndex(player_index),
    m_upgradeList(0),
    m_buildListInfo(nullptr),
    m_ai(nullptr),
    m_resourceGatheringManager(nullptr),
    m_defaultTeam(nullptr),
    m_radarCount(0),
    m_disableProofRadarCount(0),
    m_radarDisabled(false),
    m_tunnelSystem(nullptr),
    m_playerTemplate(nullptr),
    m_battlePlanBonuses(nullptr),
    m_skillPointsModifier(1.0f),
    m_canBuildUnits(true),
    m_canBuildBase(true),
    m_bountyCostToBuild(0.0f),
    m_playerColor(0),
    m_aiSquad(nullptr),
    m_rankLevel(0),
    m_sciencePurchasePoints(0),
    m_currentSkillPoints(0),
    m_lastAttackedByFrame(0),
    m_unitsShouldHunt(0)
{
    m_playerRelations = new PlayerRelationMap();
    m_teamRelations = new TeamRelationMap();
    m_activeBattlePlans[0] = 0;
    m_activeBattlePlans[1] = 0;
    m_activeBattlePlans[2] = 0;
    m_side = nullptr;
    m_baseSide = nullptr;

    for (int i = 0; i < SQUAD_COUNT; i++) {
        m_squads[i] = nullptr;
    }

    for (int i = 0; i < MAX_PLAYER_COUNT; i++) {
        m_attackedByPlayer[i] = false;
    }

    Init(nullptr);
}

// zh: 0x00450550, wb: 0x0085A1F2
Player::~Player()
{
    m_defaultTeam = nullptr;
    m_playerTemplate = nullptr;

    for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
        (*i)->Set_Owning_Player(nullptr);
    }

    m_playerTeamPrototypes.clear();
    m_teamRelations->Delete_Instance();
    m_playerRelations->Delete_Instance();

    for (int i = 0; i < SQUAD_COUNT; i++) {
        if (m_squads[i] != nullptr) {
            m_squads[i]->Delete_Instance();
            m_squads[i] = nullptr;
        }
    }

    if (m_aiSquad != nullptr) {
        m_aiSquad->Delete_Instance();
        m_aiSquad = nullptr;
    }

    if (m_battlePlanBonuses != nullptr) {
        m_battlePlanBonuses->Delete_Instance();
    }
}

// zh: 0x00457C80, wb: 0x00861D13
void Player::CRC_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, Player, Xfer *>(PICK_ADDRESS(0x00457C80, 0x00861D13), this, xfer);
#endif
}

// zh: 0x00457D30, wb: 0x00861EB8
void Player::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, Player, Xfer *>(PICK_ADDRESS(0x00457D30, 0x00861EB8), this, xfer);
#endif
}

// zh: 0x0044FDD0, wb: 0x00859AC9
void Player::Init(const PlayerTemplate *pt)
{
#ifdef GAME_DLL
    Call_Method<void, Player, const PlayerTemplate *>(PICK_ADDRESS(0x0044FDD0, 0x00859AC9), this, pt);
#endif
}

// zh: 0x00450ED0, wb: 0x0085A9FF
void Player::Update()
{
#ifdef GAME_DLL
    Call_Method<void, Player>(PICK_ADDRESS(0x00450ED0, 0x0085A9FF), this);
#endif
}

// zh: 0x00451040, wb: 0x0085ABC5
void Player::New_Map()
{
#ifdef GAME_DLL
    Call_Method<void, Player>(PICK_ADDRESS(0x00451040, 0x0085ABC5), this);
#endif
}

// zh: 0x00452E40, wb: 0x0085C643
bool Player::Compute_Superweapon_Target(const SpecialPowerTemplate *sp_template, Coord3D *loc, int32_t unk1, float unk2)
{
#ifdef GAME_DLL
    return Call_Method<bool, Player, const SpecialPowerTemplate *, Coord3D *, int32_t, float>(
        PICK_ADDRESS(0x00452E40, 0x0085C643), this, sp_template, loc, unk1, unk2);
#else
    return false;
#endif
}

// zh: 0x004532F0, wb: 0x0085CE36
bool Player::Check_Bridges(Object *obj, Waypoint *waypoint)
{
#ifdef GAME_DLL
    return Call_Method<bool, Player, Object *, Waypoint *>(PICK_ADDRESS(0x004532F0, 0x0085CE36), this, obj, waypoint);
#else
    return false;
#endif
}

// zh: 0x00453320, wb: 0x0085CE7C
bool Player::Get_AI_Base_Center(Coord3D *center)
{
#ifdef GAME_DLL
    return Call_Method<bool, Player, Coord3D *>(PICK_ADDRESS(0x00453320, 0x0085CE7C), this, center);
#else
    return false;
#endif
}

// zh: 0x00453350, wb: 0x0085CEB5
void Player::Repair_Structure(ObjectID obj_id)
{
#ifdef GAME_DLL
    Call_Method<void, Player, ObjectID>(PICK_ADDRESS(0x00453350, 0x0085CEB5), this, obj_id);
#endif
}

NameKeyType Player::Get_Player_NameKey() const
{
    return m_playerNameKey;
}

int Player::Get_Player_Index() const
{
    return m_playerIndex;
}

uint16_t Player::Get_Player_Mask() const
{
    return 1ULL << m_playerIndex;
}

int Player::Get_Color() const
{
    return m_playerColor;
}

const Team *Player::Get_Default_Team() const
{
    captainslog_dbgassert(m_defaultTeam != nullptr, "Default team is null.");
    return m_defaultTeam;
}

Team *Player::Get_Default_Team()
{
    captainslog_dbgassert(m_defaultTeam != nullptr, "Default team is null.");
    return m_defaultTeam;
}

// zh: 0x00456820, wb: 0x0086048D
void Player::On_Power_Brown_Out_Change(bool change)
{
#ifdef GAME_DLL
    Call_Method<void, Player, bool>(PICK_ADDRESS(0x00456820, 0x0086048D), this, change);
#endif
}

// zh: 0x00450AA0, wb: 0x0085A70A
bool Player::Remove_Team_Relationship(const Team *that)
{
#ifdef GAME_DLL
    return Call_Method<bool, Player, const Team *>(PICK_ADDRESS(0x00450AA0, 0x0085A70A), this, that);
#else
    return false;
#endif
}

// zh: 0x00450870, wb: 0x0085A4AA
Relationship Player::Get_Relationship(const Team *that) const
{
#ifdef GAME_DLL
    return Call_Method<Relationship, const Player, const Team *>(PICK_ADDRESS(0x00450870, 0x0085A4AA), this, that);
#else
    return Relationship();
#endif
}

// zh: 0x00453A50, wb: 0x0085D941
void Player::Update_Team_States()
{
#ifdef GAME_DLL
    Call_Method<void, Player>(PICK_ADDRESS(0x00453A50, 0x0085D941), this);
#endif
}

int Player::Get_Squad_Number_For_Object(const Object *obj) const
{
#ifdef GAME_DLL
    return Call_Method<int, const Player, const Object *>(PICK_ADDRESS(0x00457650, 0x008616BA), this, obj);
#else
    return 0;
#endif
}

// zh: 0x00452BA0, wb: 0x0085C3AA
void Player::Becoming_Local_Player(bool yes)
{
#ifdef GAME_DLL
    Call_Method<void, Player, bool>(PICK_ADDRESS(0x00452BA0, 0x0085C3AA), this, yes);
#endif
}

void Player::Count_Objects_By_Thing_Template(int num_tmplates,
    ThingTemplate const *const *things,
    bool ignore_dead,
    int *counts,
    bool ignore_under_construction) const
{
#ifdef GAME_DLL
    Call_Method<void, Player const, int, ThingTemplate const *const *, bool, int *, bool>(
        PICK_ADDRESS(0x00453710, 0x0085D45D), this, num_tmplates, things, ignore_dead, counts, ignore_under_construction);
#endif
}

bool Player::Has_Science(ScienceType t) const
{
    return std::find(m_sciences.begin(), m_sciences.end(), t) != m_sciences.end();
}

float Player::Get_Production_Cost_Change_Percent(Utf8String build_template_name) const
{
    auto iter = m_productionCostChanges.find(g_theNameKeyGenerator->Name_To_Key(build_template_name.Str()));

    if (iter != m_productionCostChanges.end()) {
        return iter->second;
    }

    return 0.0f;
}

float Player::Get_Production_Time_Change_Percent(Utf8String build_template_name) const
{
    auto iter = m_productionTimeChanges.find(g_theNameKeyGenerator->Name_To_Key(build_template_name.Str()));

    if (iter != m_productionTimeChanges.end()) {
        return iter->second;
    }

    return 0.0f;
}

float Player::Get_Production_Cost_Change_Based_On_Kind_Of(BitFlags<KINDOF_COUNT> flags) const
{
#ifdef GAME_DLL
    return Call_Method<float, const Player, BitFlags<KINDOF_COUNT>>(PICK_ADDRESS(0x00457A50, 0x00861AC7), this, flags);
#else
    return 0.0f;
#endif
}

PlayerRelationMap::~PlayerRelationMap()
{
    m_relationships.clear();
}

void PlayerRelationMap::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    unsigned short size = static_cast<unsigned short>(m_relationships.size());
    xfer->xferUnsignedShort(&size);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto it = m_relationships.begin(); it != m_relationships.end(); ++it) {
            int id = (*it).first;
            xfer->xferInt(&id);
            Relationship r = (*it).second;
            xfer->xferUser(&r, sizeof(r));
        }
    } else {
        for (unsigned short i = 0; i < size; i++) {
            int id;
            xfer->xferInt(&id);
            Relationship r;
            xfer->xferUser(&r, sizeof(r));
            m_relationships[id] = r;
        }
    }
}

void Player::Pre_Team_Destroy(const Team *team)
{
#ifdef GAME_DLL
    Call_Method<void, Player, const Team *>(PICK_ADDRESS(0x00453430, 0x0085CFF0), this, team);
#endif
}

void Player::Add_Team_To_List(TeamPrototype *team)
{
#ifdef GAME_DLL
    Call_Method<void, Player, TeamPrototype *>(PICK_ADDRESS(0x00453620, 0x0085D291), this, team);
#endif
}

void Player::Remove_Team_From_List(TeamPrototype *team)
{
#ifdef GAME_DLL
    Call_Method<void, Player, TeamPrototype *>(PICK_ADDRESS(0x00453660, 0x0085D30E), this, team);
#endif
}

GameDifficulty Player::Get_Player_Difficulty() const
{
#ifdef GAME_DLL
    return Call_Method<GameDifficulty, const Player>(PICK_ADDRESS(0x004532D0, 0x0085CE04), this);
#else
    return DIFFICULTY_EASY;
#endif
}

void Player::Apply_Battle_Plan_Bonuses_For_Object(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Player, Object *>(PICK_ADDRESS(0x00456E00, 0x00860B23), this, obj);
#endif
}

void Player::Iterate_Objects(void (*func)(Object *, void *), void *data) const
{
    for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
        (*i)->Iterate_Objects(func, data);
    }
}

void Player::Init_From_Dict(const Dict *d)
{
#ifdef GAME_DLL
    Call_Method<void, Player, const Dict *>(PICK_ADDRESS(0x00451320, 0x0085ADE4), this, d);
#endif
}

void Player::Set_Build_List(BuildListInfo *build_list)
{
    if (m_buildListInfo != nullptr) {
        m_buildListInfo->Delete_Instance();
    }

    m_buildListInfo = build_list;
}

void Player::Set_Default_Team()
{
    Utf8String str;
    str.Set("team");
    str += m_playerName;
    Team *team = g_theTeamFactory->Find_Team(str);
    captainslog_dbgassert(team != nullptr, "no team");

    if (team != nullptr) {
        m_defaultTeam = team;
        team->Set_Active();
    }
}

void Player::Set_Player_Relationship(const Player *that, Relationship r)
{
    if (that != nullptr) {
        m_playerRelations->m_relationships[that->Get_Player_Index()] = r;
    }
}

void Player::Set_Player_Type(PlayerType t, bool is_skirmish)
{
#ifdef GAME_DLL
    Call_Method<void, Player, PlayerType, bool>(PICK_ADDRESS(0x00451050, 0x0085ABEC), this, t, is_skirmish);
#endif
}
