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
#include "aiplayer.h"
#include "behaviormodule.h"
#include "buildinfo.h"
#include "cavesystem.h"
#include "controlbar.h"
#include "drawable.h"
#include "gameclient.h"
#include "gameinfo.h"
#include "gamelogic.h"
#include "gametext.h"
#include "globaldata.h"
#include "namekeygenerator.h"
#include "opencontain.h"
#include "partitionmanager.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "radar.h"
#include "rankinfo.h"
#include "resourcegatheringmanager.h"
#include "scriptengine.h"
#include "simpleobjectiterator.h"
#include "squad.h"
#include "stealthupdate.h"
#include "team.h"
#include <algorithm>

// zh: 0x0044F7B0, wb: 0x0085960A
Player::Player(int32_t player_index) :
    m_playerIsPreorder(false),
    m_playerIsDead(false),
    m_playerIndex(player_index),
    m_upgradeList(nullptr),
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
    bool has_bonuses = m_battlePlanBonuses != nullptr;
    xfer->xferBool(&has_bonuses);
#ifdef GAME_DEBUG_STRUCTS
    // TODO unknown CRC stuff
#endif

    if (m_battlePlanBonuses != nullptr) {
#ifdef GAME_DEBUG_STRUCTS
        // TODO dumpBattlePlanBonuses
#endif
        xfer->xferReal(&m_battlePlanBonuses->m_armorBonus);
        xfer->xferReal(&m_battlePlanBonuses->m_sightBonus);
        xfer->xferInt(&m_battlePlanBonuses->m_bombardment);
        xfer->xferInt(&m_battlePlanBonuses->m_holdTheLine);
        xfer->xferInt(&m_battlePlanBonuses->m_searchAndDestroy);
        m_battlePlanBonuses->m_validKindOf.Xfer(xfer);
        m_battlePlanBonuses->m_invalidKindOf.Xfer(xfer);
    }

    xfer->xferInt(&m_currentSkillPoints);
    xfer->xferInt(&m_sciencePurchasePoints);
}

// zh: 0x00457D30, wb: 0x00861EB8
void Player::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 8;
    xfer->xferVersion(&version, 8);
    xfer->xferSnapshot(&m_money);
    unsigned short upgrade_count = 0;

    for (Upgrade *upgrade = m_upgradeList; upgrade != nullptr; upgrade = upgrade->Friend_Get_Next()) {
        upgrade_count++;
    }

    xfer->xferUnsignedShort(&upgrade_count);

    if (version >= 7) {
        xfer->xferBool(&m_playerIsPreorder);
    }

    if (version >= 8) {
        xfer->xferScienceVec(&m_disabledSciences);
        xfer->xferScienceVec(&m_hiddenSciences);
    }

    Utf8String str;

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (Upgrade *upgrade = m_upgradeList; upgrade != nullptr; upgrade = upgrade->Friend_Get_Next()) {
            str = upgrade->Get_Template()->Get_Name();
            xfer->xferAsciiString(&str);
            xfer->xferSnapshot(upgrade);
        }
    } else {
        for (unsigned short i = 0; i < upgrade_count; i++) {
            xfer->xferAsciiString(&str);
            UpgradeTemplate *tmplate = g_theUpgradeCenter->Find_Upgrade(str);
            captainslog_relassert(tmplate != nullptr, 6, "Player::xfer - Unable to find upgrade '%s'", str.Str());
            Upgrade *upgrade = Add_Upgrade(tmplate, UPGRADE_STATUS_INVALID);
            xfer->xferSnapshot(upgrade);
        }
    }

    xfer->xferInt(&m_radarCount);
    xfer->xferBool(&m_playerIsDead);
    xfer->xferInt(&m_disableProofRadarCount);
    xfer->xferBool(&m_radarDisabled);
    xfer->xferUpgradeMask(&m_upgradesInProgress);
    xfer->xferUpgradeMask(&m_upgradesCompleted);
    xfer->xferSnapshot(&m_energy);

    unsigned short team_count = static_cast<unsigned short>(m_playerTeamPrototypes.size());
    xfer->xferUnsignedShort(&team_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
            unsigned int id = (*i)->Get_ID();
            xfer->xferUser(&id, sizeof(id));
        }
    } else {
        m_playerTeamPrototypes.clear();

        for (unsigned short i = 0; i < team_count; i++) {
            unsigned int id;
            xfer->xferUser(&id, sizeof(id));
            TeamPrototype *prototype = g_theTeamFactory->Find_Team_Prototype_By_ID(id);
            captainslog_relassert(prototype != nullptr, 6, "Player::xfer - Unable to find team prototype by id");
            m_playerTeamPrototypes.push_back(prototype);
        }
    }

    unsigned short build_list_count = 0;

    for (BuildListInfo *build_list = m_buildListInfo; build_list != nullptr; build_list = build_list->Get_Next()) {
        build_list_count++;
    }

    xfer->xferUnsignedShort(&build_list_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (BuildListInfo *build_list = m_buildListInfo; build_list != nullptr; build_list = build_list->Get_Next()) {
            xfer->xferSnapshot(build_list);
        }
    } else {
        if (m_buildListInfo != nullptr) {
            m_buildListInfo->Delete_Instance();
            m_buildListInfo = nullptr;
        }

        for (unsigned short i = 0; i < build_list_count; i++) {
            BuildListInfo *build_list = new BuildListInfo();
            build_list->Set_Next(nullptr);

            if (m_buildListInfo != nullptr) {
                BuildListInfo *build_list2;

                for (build_list2 = m_buildListInfo; build_list2->Get_Next() != nullptr;
                     build_list2 = build_list2->Get_Next()) {
                }

                build_list2->Set_Next(build_list);
            } else {
                m_buildListInfo = build_list;
            }

            xfer->xferSnapshot(build_list);
        }
    }

    bool has_ai = m_ai != nullptr;
    xfer->xferBool(&has_ai);
    captainslog_relassert(
        (!has_ai && m_ai == nullptr) || (has_ai && m_ai != nullptr), 6, "Player::xfer - m_ai present/missing mismatch");

    if (m_ai != nullptr) {
        xfer->xferSnapshot(m_ai);
    }

    bool has_resource_manager = m_resourceGatheringManager != nullptr;
    xfer->xferBool(&has_resource_manager);
    captainslog_relassert((!has_resource_manager && m_resourceGatheringManager == nullptr)
            || (has_resource_manager && m_resourceGatheringManager != nullptr),
        6,
        "Player::xfer - m_resourceGatheringManager present/missing mismatch");

    if (m_resourceGatheringManager != nullptr) {
        xfer->xferSnapshot(m_resourceGatheringManager);
    }

    bool has_tunnel_system = m_tunnelSystem != nullptr;
    xfer->xferBool(&has_tunnel_system);

    captainslog_relassert(
        (!has_tunnel_system && m_tunnelSystem == nullptr) || (has_tunnel_system && m_tunnelSystem != nullptr),
        6,
        "Player::xfer - m_tunnelSystem present/missing mismatch");

    if (m_tunnelSystem != nullptr) {
        xfer->xferSnapshot(m_tunnelSystem);
    }

    int team_id;

    if (m_defaultTeam != nullptr) {
        team_id = m_defaultTeam->Get_Team_ID();
    } else {
        team_id = 0;
    }

    xfer->xferUser(&team_id, sizeof(team_id));

    if (xfer->Get_Mode() == XFER_LOAD) {
        m_defaultTeam = g_theTeamFactory->Find_Team_By_ID(team_id);
    }

    if (version < 5) {
        unsigned short science_count = static_cast<unsigned short>(m_sciences.size());
        xfer->xferUnsignedShort(&science_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto i = m_sciences.begin(); i != m_sciences.end(); i++) {
                ScienceType science = *i;
                xfer->xferUser(&science, sizeof(science));
            }
        } else {
            for (int i = 0; i < science_count; i++) {
                ScienceType science;
                xfer->xferUser(&science, sizeof(science));
                m_sciences.push_back(science);
            }
        }
    } else {
        if (xfer->Get_Mode() == XFER_LOAD) {
            m_sciences.clear();
        }

        xfer->xferScienceVec(&m_sciences);
    }

    xfer->xferInt(&m_rankLevel);
    xfer->xferInt(&m_currentSkillPoints);
    xfer->xferInt(&m_sciencePurchasePoints);
    xfer->xferInt(&m_skillPointsNeededForNextRank);
    xfer->xferInt(&m_rankProgress);
    xfer->xferUnicodeString(&m_scienceGeneralName);
    xfer->xferSnapshot(m_playerRelations);
    xfer->xferSnapshot(m_teamRelations);
    xfer->xferBool(&m_canBuildUnits);
    xfer->xferBool(&m_canBuildBase);
    xfer->xferBool(&m_playerIsObserver);

    if (version < 2) {
        m_skillPointsModifier = 1.0f;
    } else {
        xfer->xferReal(&m_skillPointsModifier);
    }

    if (version < 3) {
        m_listInScoreScreen = true;
    } else {
        xfer->xferBool(&m_listInScoreScreen);
    }

    xfer->xferUser(m_attackedByPlayer, sizeof(m_attackedByPlayer));
    xfer->xferReal(&m_bountyCostToBuild);
    xfer->xferSnapshot(&m_scoreKeeper);
    unsigned short change_list_count = static_cast<unsigned short>(m_kindOfPercentProductionChangeList.size());
    xfer->xferUnsignedShort(&change_list_count);

    if (xfer->Get_Mode() == XFER_SAVE) {
        for (auto i = m_kindOfPercentProductionChangeList.begin(); i != m_kindOfPercentProductionChangeList.end(); i++) {
            KindOfPercentProductionChange *k = *i;
            k->m_flags.Xfer(xfer);
            xfer->xferReal(&k->m_cost);
            xfer->xferUnsignedInt(&k->m_count);
        }
    } else {
        captainslog_relassert(m_kindOfPercentProductionChangeList.size() == 0,
            6,
            "Player::xfer - m_kindOfPercentProductionChangeList should be empty but is not");
        for (int i = 0; i < change_list_count; i++) {
            KindOfPercentProductionChange *k = new KindOfPercentProductionChange;
            k->m_flags.Xfer(xfer);
            xfer->xferReal(&k->m_cost);
            xfer->xferUnsignedInt(&k->m_count);
            m_kindOfPercentProductionChangeList.push_back(k);
        }
    }

    if (version > 4) {
        unsigned short special_power_timer_count = static_cast<unsigned short>(m_specialPowerReadyTimerList.size());
        xfer->xferUnsignedShort(&special_power_timer_count);

        if (xfer->Get_Mode() == XFER_SAVE) {
            for (auto i = m_specialPowerReadyTimerList.begin(); i != m_specialPowerReadyTimerList.end(); i++) {
                SpecialPowerReadyTimerType &timer = (*i);
                xfer->xferUnsignedInt(&timer.m_id);
                xfer->xferUnsignedInt(&timer.m_frame);
            }
        } else {
            captainslog_relassert(m_specialPowerReadyTimerList.size() == 0,
                6,
                "Player::xfer - m_specialPowerReadyTimerList should be empty but is not");
            for (int i = 0; i < special_power_timer_count; i++) {
                SpecialPowerReadyTimerType timer;
                xfer->xferUnsignedInt(&timer.m_id);
                xfer->xferUnsignedInt(&timer.m_frame);
                m_specialPowerReadyTimerList.push_back(timer);
            }
        }
    } else {
        m_specialPowerReadyTimerList.clear();
    }

    unsigned short squad_count = SQUAD_COUNT;
    xfer->xferUnsignedShort(&squad_count);
    captainslog_relassert(squad_count == SQUAD_COUNT, 6, "Player::xfer - size of m_squadCount array has changed");

    for (int i = 0; i < squad_count; i++) {
        captainslog_relassert(m_squads[i] != nullptr, 6, "Player::xfer - NULL squad at index '%d'", i);
        xfer->xferSnapshot(m_squads[i]);
    }

    bool has_ai_squad = m_aiSquad != nullptr;
    xfer->xferBool(&has_ai_squad);

    if (has_ai_squad) {
        if (m_aiSquad == nullptr && xfer->Get_Mode() == XFER_LOAD) {
            m_aiSquad = new Squad();
        }

        xfer->xferSnapshot(m_aiSquad);
    }

    bool has_battle_plan_bonuses = m_battlePlanBonuses != nullptr;
    xfer->xferBool(&has_battle_plan_bonuses);

    if (xfer->Get_Mode() == XFER_LOAD) {
        if (m_battlePlanBonuses != nullptr) {
            m_battlePlanBonuses->Delete_Instance();
            m_battlePlanBonuses = nullptr;
        }

        if (has_battle_plan_bonuses) {
            m_battlePlanBonuses = new BattlePlanBonuses();
        }
    }

    if (m_battlePlanBonuses != nullptr) {
        xfer->xferReal(&m_battlePlanBonuses->m_armorBonus);
        xfer->xferReal(&m_battlePlanBonuses->m_sightBonus);
        xfer->xferInt(&m_battlePlanBonuses->m_bombardment);
        xfer->xferInt(&m_battlePlanBonuses->m_holdTheLine);
        xfer->xferInt(&m_battlePlanBonuses->m_searchAndDestroy);
        m_battlePlanBonuses->m_validKindOf.Xfer(xfer);
        m_battlePlanBonuses->m_invalidKindOf.Xfer(xfer);
    }

    xfer->xferInt(&m_activeBattlePlans[0]);
    xfer->xferInt(&m_activeBattlePlans[1]);
    xfer->xferInt(&m_activeBattlePlans[2]);

    if (version < 6) {
        m_unitsShouldHunt = false;
    } else {
        xfer->xferBool(&m_unitsShouldHunt);
    }
}

// zh: 0x0044FDD0, wb: 0x00859AC9
void Player::Init(const PlayerTemplate *pt)
{
    captainslog_dbgassert(m_playerTeamPrototypes.size() == 0, "Player::m_playerTeamPrototypes is not empty at game start!");
    m_skillPointsModifier = 1.0f;
    m_lastAttackedByFrame = 0;
    m_playerIsPreorder = 0;
    m_playerIsDead = 0;
    m_radarCount = 0;
    m_disableProofRadarCount = 0;
    m_radarDisabled = 0;
    m_activeBattlePlans[0] = 0;
    m_activeBattlePlans[1] = 0;
    m_activeBattlePlans[2] = 0;

    if (m_battlePlanBonuses != nullptr) {
        m_battlePlanBonuses->Delete_Instance();
        m_battlePlanBonuses = nullptr;
    }

    Delete_Upgrade_List();
    m_energy.Init(this);
    m_missionStats.Init();

    if (m_buildListInfo != nullptr) {
        m_buildListInfo->Delete_Instance();
        m_buildListInfo = nullptr;
    }

    m_defaultTeam = nullptr;

    if (m_ai != nullptr) {
        m_ai->Delete_Instance();
        m_ai = nullptr;
    }

    if (m_resourceGatheringManager != nullptr) {
        m_resourceGatheringManager->Delete_Instance();
        m_resourceGatheringManager = nullptr;
    }

    for (int i = 0; i < SQUAD_COUNT; i++) {
        if (m_squads[i] != nullptr) {
            m_squads[i]->Delete_Instance();
            m_squads[i] = nullptr;
        }

        m_squads[i] = new Squad();
    }

    if (m_aiSquad != nullptr) {
        m_aiSquad->Delete_Instance();
        m_aiSquad = nullptr;
    }

    m_aiSquad = new Squad();

    if (m_tunnelSystem != nullptr) {
        m_tunnelSystem->Delete_Instance();
        m_tunnelSystem = nullptr;
    }

    m_canBuildBase = true;
    m_canBuildUnits = true;
    m_playerIsObserver = false;
    m_bountyCostToBuild = 0.0f;
    m_listInScoreScreen = true;
    m_unitsShouldHunt = false;
#ifdef GAME_DEBUG_STRUCTS
    m_ignorePrereqs = false;
    m_freeBuild = false;
    m_instantBuild = false;
#endif

    if (pt != nullptr) {
        m_side = pt->Get_Side_Name();
        m_baseSide = pt->Get_Base_Side();
        m_productionCostChanges = *pt->Get_Production_Cost_Changes();
        m_productionTimeChanges = *pt->Get_Production_Time_Changes();
        m_productionVeterancyLevels = *pt->Get_Production_Veterancy_Levels();
        m_playerColor = pt->Get_Preferred_Color()->Get_As_Int() | 0xFF000000;
        m_playerNightColor = m_playerColor;
        m_money = *pt->Get_Money();
        m_money.Set_Player_Index(Get_Player_Index());
        m_handicap = *pt->Get_Handicap();

        if (m_money.Get() == 0) {
            if (g_theGameInfo != nullptr) {
                m_money = *g_theGameInfo->Get_Money();
            } else {
                m_money = g_theWriteableGlobalData->m_defaultStartingCash;
            }
        }

        m_playerDisplayName.Clear();
        m_playerName.Clear();
        m_playerNameKey = NAMEKEY_INVALID;
        m_playerType = PLAYER_COMPUTER;
        m_playerIsObserver = pt->Is_Observer();
        m_playerIsDead = m_playerIsObserver;
    } else {
        m_side = "";
        m_baseSide = "";
        m_productionCostChanges.clear();
        m_productionTimeChanges.clear();
        m_productionVeterancyLevels.clear();
        m_playerColor = -1;
        m_playerNightColor = -1;
        m_money.Empty();
        m_handicap.Init();
        m_playerDisplayName = Utf16String::s_emptyString;
        m_playerName = Utf8String::s_emptyString;
        m_playerNameKey = g_theNameKeyGenerator->Name_To_Key(Utf8String::s_emptyString.Str());
        m_playerType = PLAYER_COMPUTER;
        Set_Player_Relationship(this, ALLIES);
    }

    m_scoreKeeper.Reset(m_playerIndex);
    m_playerTemplate = pt;
    Reset_Rank();
    m_disabledSciences.clear();
    m_hiddenSciences.clear();

    for (auto i = m_specialPowerReadyTimerList.begin(); i != m_specialPowerReadyTimerList.end();) {
        SpecialPowerReadyTimerType &timer = *i;
        i = m_specialPowerReadyTimerList.erase(i);
        timer.Reset();
    }

    for (auto i = m_kindOfPercentProductionChangeList.begin(); i != m_kindOfPercentProductionChangeList.end();) {
        KindOfPercentProductionChange *k = (*i);
        i = m_kindOfPercentProductionChangeList.erase(i);

        if (k != nullptr) {
            k->Delete_Instance();
        }
    }

    Get_Academy_Stats()->Init(this);
    m_retaliationModeEnabled = false;
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
    if (m_ai != nullptr) {
        m_ai->New_Map();
    }
}

// zh: 0x00452E40, wb: 0x0085C643
bool Player::Compute_Superweapon_Target(const SpecialPowerTemplate *sp_template, Coord3D *loc, int32_t unk1, float unk2)
{
    if (m_ai != nullptr) {
        return m_ai->Compute_Superweapon_Target(sp_template, loc, unk1, unk2);
    } else {
        return false;
    }
}

// zh: 0x004532F0, wb: 0x0085CE36
bool Player::Check_Bridges(Object *obj, Waypoint *waypoint)
{
    if (m_ai != nullptr) {
        return m_ai->Check_Bridges(obj, waypoint);
    } else {
        return false;
    }
}

// zh: 0x00453320, wb: 0x0085CE7C
bool Player::Get_AI_Base_Center(Coord3D *center)
{
    if (m_ai != nullptr) {
        return m_ai->Get_AI_Base_Center(center);
    } else {
        return false;
    }
}

// zh: 0x00453350, wb: 0x0085CEB5
void Player::Repair_Structure(ObjectID obj_id)
{
    if (m_ai != nullptr) {
        m_ai->Repair_Structure(obj_id);
    }
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
    if (m_teamRelations->m_relationships.empty()) {
        return false;
    }

    if (that == nullptr) {
        m_teamRelations->m_relationships.clear();
        return true;
    }

    auto it = m_teamRelations->m_relationships.find(that->Get_Team_ID());

    if (it != m_teamRelations->m_relationships.end()) {
        m_teamRelations->m_relationships.erase(it);
        return true;
    }

    return false;
}

// zh: 0x00450870, wb: 0x0085A4AA
Relationship Player::Get_Relationship(const Team *that) const
{
    if (that == nullptr) {
        return NEUTRAL;
    }

    if (!m_teamRelations->m_relationships.empty()) {
        auto it = m_teamRelations->m_relationships.find(that->Get_Team_ID());

        if (it != m_teamRelations->m_relationships.end()) {
            return (*it).second;
        }
    }

    if (!m_playerRelations->m_relationships.empty()) {
        Player *player = that->Get_Controlling_Player();

        if (player != nullptr) {
            auto i = m_playerRelations->m_relationships.find(player->Get_Player_Index());

            if (i != m_playerRelations->m_relationships.end()) {
                return (*i).second;
            }
        }
    }

    return NEUTRAL;
}

// zh: 0x00453A50, wb: 0x0085D941
void Player::Update_Team_States()
{
    for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
        (*i)->Update_State();
    }
}

int Player::Get_Squad_Number_For_Object(const Object *obj) const
{
    for (int i = 0; i < SQUAD_COUNT; i++) {
        if (m_squads[i]->Is_On_Squad(obj)) {
            return i;
        }
    }

    return -1;
}

// zh: 0x00452BA0, wb: 0x0085C3AA
void Player::Becoming_Local_Player(bool yes)
{
    if (yes) {
        if (g_theGameClient != nullptr) {
            RGBColor color;
            color.Set_From_Int(m_playerColor);
            g_theGameClient->Set_Team_Color(GameMath::Fast_To_Int_Truncate(color.red * 255.0f),
                GameMath::Fast_To_Int_Truncate(color.green * 255.0f),
                GameMath::Fast_To_Int_Truncate(color.blue * 255.0f));
        }

        if (g_thePartitionManager != nullptr) {
            SimpleObjectIterator *iterator = g_thePartitionManager->Iterate_All_Objects(nullptr);

            for (Object *obj = iterator->First(); obj != nullptr; obj = iterator->Next()) {
                ContainModuleInterface *contain = obj->Get_Contain();

                if (contain != nullptr) {
                    contain->Recalc_Apparent_Controlling_Player();
                    g_theRadar->Remove_Object(obj);
                    g_theRadar->Add_Object(obj);
                }

                if (obj->Is_KindOf(KINDOF_DISGUISER)) {
                    Drawable *drawable = obj->Get_Drawable();

                    if (drawable != nullptr) {
                        StealthUpdate *stealth = obj->Get_Stealth_Update();

                        if (stealth != nullptr) {
                            if (stealth->Has_Disguised_Template()) {
                                Player *player = g_thePlayerList->Get_Nth_Player(stealth->Get_Player_Index());
                                int indicator;

                                if (Get_Relationship(obj->Get_Team()) == ALLIES || !Is_Player_Active()) {
                                    if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
                                        indicator = obj->Get_Night_Indicator_Color();
                                    } else {
                                        indicator = obj->Get_Indicator_Color();
                                    }
                                } else if (g_theWriteableGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
                                    indicator = player->Get_Night_Color();
                                } else {
                                    indicator = player->Get_Color();
                                }

                                drawable->Set_Indicator_Color(indicator);
                                g_theRadar->Remove_Object(obj);
                                g_theRadar->Add_Object(obj);
                            }
                        }
                    }
                }
            }

            iterator->Delete_Instance();
        }

        if (g_theControlBar != nullptr) {
            g_theControlBar->Mark_UI_Dirty();
        }
    }
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

Upgrade *Player::Add_Upgrade(const UpgradeTemplate *upgrade_template, UpgradeStatusType status)
{
    Upgrade *upgrade = Find_Upgrade(upgrade_template);

    if (upgrade == nullptr) {
        upgrade = new Upgrade(upgrade_template);
        upgrade->Friend_Set_Prev(nullptr);
        upgrade->Friend_Set_Next(m_upgradeList);

        if (m_upgradeList != nullptr) {
            m_upgradeList->Friend_Set_Prev(upgrade);
        }

        m_upgradeList = upgrade;
    }

    upgrade->Set_Status(status);
    BitFlags<128> mask = upgrade_template->Get_Upgrade_Mask();

    if (status == UPGRADE_STATUS_IN_PRODUCTION) {
        m_upgradesInProgress.Set(mask);
    } else if (status == UPGRADE_STATUS_COMPLETE) {
        m_upgradesInProgress.Clear(mask);
        m_upgradesCompleted.Set(mask);
        On_Upgrade_Completed(upgrade_template);
    }

    if (g_thePlayerList->Get_Local_Player() == this) {
        g_theControlBar->Mark_UI_Dirty();
    }

    return upgrade;
}

Upgrade *Player::Find_Upgrade(const UpgradeTemplate *upgrade_template)
{
    for (Upgrade *upgrade = m_upgradeList; upgrade != nullptr; upgrade = upgrade->Friend_Get_Next()) {
        if (upgrade->Get_Template() == upgrade_template) {
            return upgrade;
        }
    }

    return nullptr;
}

void Player::On_Upgrade_Completed(const UpgradeTemplate *upgrade_template)
{
    for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
        DLINK_ITERATOR<Team> iter = (*i)->Iterate_Team_Instance_List();

        while (!iter.Done()) {
            Team *team = iter.Cur();

            if (team != nullptr) {
                DLINK_ITERATOR<Object> iter2 = team->Iterate_Team_Member_List();

                while (!iter2.Done()) {
                    Object *obj = iter2.Cur();

                    if (obj != nullptr) {
                        obj->Update_Upgrade_Modules();
                    }

                    iter2.Advance();
                }
            }

            iter.Advance();
        }
    }
}

void Player::Delete_Upgrade_List()
{
    while (m_upgradeList != nullptr) {
        Upgrade *upgrade = m_upgradeList->Friend_Get_Next();
        m_upgradeList->Delete_Instance();
        m_upgradeList = upgrade;
    }

    m_upgradesInProgress.Clear();
    m_upgradesCompleted.Clear();
}

void Player::Reset_Rank()
{
    m_rankLevel = 1;
    m_currentSkillPoints = 0;
    const RankInfo *next_rank_info = g_theRankInfoStore->Get_Rank_Info(m_rankLevel + 1);

    if (next_rank_info != nullptr) {
        m_skillPointsNeededForNextRank = next_rank_info->m_skillPointsNeeded;
    } else {
        m_skillPointsNeededForNextRank = 0x7FFFFFFF;
    }

    m_rankProgress = 0;
    m_sciences.clear();

    if (Get_Player_Template() != nullptr) {
        m_sciencePurchasePoints = Get_Player_Template()->Get_Intrinsic_Science_Purchase_Points();
    } else {
        m_sciencePurchasePoints = 0;
    }

    const RankInfo *cur_rank_info = g_theRankInfoStore->Get_Rank_Info(m_rankLevel);

    if (cur_rank_info != nullptr) {
        m_sciencePurchasePoints += cur_rank_info->m_sciencePurchasePointsGranted;
    }

    if (g_theGameText != nullptr) {
        m_scienceGeneralName = g_theGameText->Fetch("SCIENCE:GeneralName");
    } else {
        m_scienceGeneralName = Utf16String::s_emptyString;
    }

    Reset_Sciences();
}

void Player::Reset_Sciences()
{
    m_sciences.clear();

    if (Get_Player_Template() != nullptr) {
        m_sciences = *Get_Player_Template()->Get_Intrinsinc_Sciences();
    }

    for (int i = 1; i <= m_rankLevel; i++) {
        const RankInfo *info = g_theRankInfoStore->Get_Rank_Info(i);

        if (info != nullptr) {
            for (auto j = info->m_sciencesGranted.begin(); j != info->m_sciencesGranted.end(); j++) {
                Add_Science(*j);
            }
        }
    }

    for (auto i = m_sciences.begin(); i != m_sciences.end(); i++) {
        g_theScriptEngine->Notify_Of_Acquired_Science(Get_Player_Index(), *i);
    }
}

bool Player::Add_Science(ScienceType t)
{
    if (Has_Science(t)) {
        return false;
    }

    m_sciences.push_back(t);

    for (auto i = m_playerTeamPrototypes.begin(); i != m_playerTeamPrototypes.end(); i++) {
        DLINK_ITERATOR<Team> iter = (*i)->Iterate_Team_Instance_List();

        while (!iter.Done()) {
            Team *team = iter.Cur();

            if (team != nullptr) {
                DLINK_ITERATOR<Object> iter2 = team->Iterate_Team_Member_List();

                while (!iter2.Done()) {
                    Object *obj = iter2.Cur();

                    if (obj != nullptr) {
                        for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
                            SpecialPowerModuleInterface *interface = (*module)->Get_Special_Power();

                            if (interface != nullptr) {
                                if (interface->Get_Required_Science() == t) {
                                    interface->On_Special_Power_Creation();
                                    interface->Set_Ready_Frame(g_theGameLogic->Get_Frame());
                                }
                            }
                        }
                    }

                    iter2.Advance();
                }
            }

            iter.Advance();
        }

        g_theControlBar->Mark_UI_Dirty();
    }

    g_theScriptEngine->Notify_Of_Acquired_Science(Get_Player_Index(), t);
    return true;
}
