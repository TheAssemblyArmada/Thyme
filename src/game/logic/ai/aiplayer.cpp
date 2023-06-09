/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI Player
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aiplayer.h"
#include "ai.h"
#include "gamelogic.h"
#include "player.h"
#include "scriptengine.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void Delete_Queue(TeamInQueue *team)
{
    if (team != nullptr) {
        team->Delete_Instance();
    }
}

AIPlayer::AIPlayer(Player *p) :
    m_player(p),
    m_readyToBuildTeam(false),
    m_readyToBuildStructure(false),
    m_teamTimer(2),
    m_structureTimer(2),
    m_teamDelaySeconds(10),
    m_buildDelay(0),
    m_teamDelay(0),
    m_skillsetSelector(INVALID_SKILLSET_SELECTION),
    m_baseRadius(0.0f),
    m_dozerID(INVALID_OBJECT_ID),
    m_structureRepairCount(0),
    m_dozerQueued(false),
    m_dozerRepairing(false),
    m_bridgeRepairTimer(0),
    m_supplySourceFrame(0),
    m_supplySourceID(INVALID_OBJECT_ID),
    m_supplyCenterID(INVALID_OBJECT_ID)
{
    m_frameLastBuildingBuilt = g_theGameLogic->Get_Frame();
    p->Set_Can_Build_Units(false);

    for (int i = 0; i < 2; i++) {
        m_structureRepairQueue[i] = INVALID_OBJECT_ID;
    }

    m_dozerPosition.Zero();
    m_baseCenter.Zero();
    m_baseCenterValid = false;
    m_difficulty = g_theScriptEngine->Get_Difficulty();
    m_teamDelaySeconds = g_theAI->Get_AI_Data()->m_teamSeconds;
}

AIPlayer::~AIPlayer()
{
    Clear_Teams_In_Queue();
}

void AIPlayer::Clear_Teams_In_Queue()
{
    Remove_All_Team_Build_Queue(Delete_Queue);
    Remove_All_Team_Ready_Queue(Delete_Queue);
}

bool AIPlayer::Compute_Superweapon_Target(const SpecialPowerTemplate *special_power, Coord3D *pos, int player_index, float f)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, const SpecialPowerTemplate *, Coord3D *, int, float>(
        PICK_ADDRESS(0x00580E10, 0x00A46775), this, special_power, pos, player_index, f);
#else
    return false;
#endif
}

void AIPlayer::Update()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585DB0, 0x00A4B0E7), this);
#endif
}

void AIPlayer::New_Map()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585DE0, 0x00A4B131), this);
#endif
}

void AIPlayer::On_Unit_Produced(Object *factory, Object *unit)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, Object *, Object *>(PICK_ADDRESS(0x005809E0, 0x00A46400), this, factory, unit);
#endif
}

void AIPlayer::On_Structure_Produced(Object *factory, Object *bldg)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, Object *, Object *>(PICK_ADDRESS(0x0057E770, 0x00A43BCC), this, factory, bldg);
#endif
}

void AIPlayer::Build_Specific_AI_Team(TeamPrototype *team_proto, bool priority_build)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, TeamPrototype *, bool>(
        PICK_ADDRESS(0x005840D0, 0x00A4998D), this, team_proto, priority_build);
#endif
}

void AIPlayer::Build_AI_Base_Defense(bool b)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, bool>(PICK_ADDRESS(0x00583CC0, 0x00A493CF), this, b);
#endif
}

void AIPlayer::Build_AI_Base_Defense_Structure(const Utf8String &name, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, const Utf8String &, bool>(PICK_ADDRESS(0x00583D60, 0x00A49431), this, name, b);
#endif
}

void AIPlayer::Build_Specific_AI_Building(const Utf8String &name)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, const Utf8String &>(PICK_ADDRESS(0x005824F0, 0x00A47F22), this, name);
#endif
}

void AIPlayer::Recruit_Specific_AI_Team(TeamPrototype *team_proto, float recruit_radius)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, TeamPrototype *, float>(
        PICK_ADDRESS(0x00584890, 0x00A49F72), this, team_proto, recruit_radius);
#endif
}

void AIPlayer::Repair_Structure(ObjectID id)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, ObjectID>(PICK_ADDRESS(0x00583E00, 0x00A49493), this, id);
#endif
}

void AIPlayer::Select_Skillset(int skillset)
{
    captainslog_dbgassert(m_skillsetSelector != INVALID_SKILLSET_SELECTION,
        "Selecting a skill set (%d) after one has already been chosen (%d) means some points have been incorrectly spent.",
        skillset + 1,
        m_skillsetSelector + 1);
    m_skillsetSelector = skillset;
}

void AIPlayer::Do_Base_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585320, 0x00A4A6FD), this);
#endif
}

void AIPlayer::Check_Ready_Teams()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585380, 0x00A4A793), this);
#endif
}

void AIPlayer::Check_Queued_Teams()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585690, 0x00A4AAA4), this);
#endif
}

void AIPlayer::Do_Team_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585920, 0x00A4AD2E), this);
#endif
}

void AIPlayer::Do_Upgrades_And_Skills()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00585980, 0x00A4ADC3), this);
#endif
}

Object *AIPlayer::Find_Dozer(const Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<Object *, AIPlayer, const Coord3D *>(PICK_ADDRESS(0x005864B0, 0x00A4B817), this, pos);
#else
    return nullptr;
#endif
}

void AIPlayer::Queue_Dozer()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x005861B0, 0x00A4B5BB), this);
#endif
}

bool AIPlayer::Select_Team_To_Build()
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer>(PICK_ADDRESS(0x005820B0, 0x00A47A9D), this);
#else
    return false;
#endif
}

bool AIPlayer::Select_Team_To_Reinforce(int min_priority)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, int>(PICK_ADDRESS(0x005819E0, 0x00A474D4), this, min_priority);
#else
    return false;
#endif
}

bool AIPlayer::Start_Training(WorkOrder *order, bool busy_ok, Utf8String team_name)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, WorkOrder *, bool, Utf8String>(
        PICK_ADDRESS(0x00581420, 0x00A46F3B), this, order, busy_ok, team_name);
#else
    return false;
#endif
}

bool AIPlayer::Is_A_Good_Idea_To_Build_Team(TeamPrototype *team_proto)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, TeamPrototype *>(PICK_ADDRESS(0x00581800, 0x00A47301), this, team_proto);
#else
    return false;
#endif
}

void AIPlayer::Process_Base_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00580070, 0x00A458E1), this);
#endif
}

void AIPlayer::Process_Team_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer>(PICK_ADDRESS(0x00584F50, 0x00A4A447), this);
#endif
}

void AIPlayer::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x00586620, 0x00A4BA2B), this, xfer);
#endif
}

AISkirmishPlayer::AISkirmishPlayer(Player *p) :
    AIPlayer(p),
    m_flags1(0),
    m_flags2(0),
    m_angle1(0.0f),
    m_angle2(0.0f),
    m_angle3(0.0f),
    m_angle4(0.0f),
    m_angle5(0.0f),
    m_angle6(0.0f),
    m_nextGetEnemyFrame(0),
    m_enemyPlayer(nullptr)
{
    p->Set_Can_Build_Units(true);
}

AISkirmishPlayer::~AISkirmishPlayer()
{
    Clear_Teams_In_Queue();
}

bool AISkirmishPlayer::Compute_Superweapon_Target(
    const SpecialPowerTemplate *special_power, Coord3D *pos, int player_index, float f)
{
#ifdef GAME_DLL
    return Call_Method<bool, AISkirmishPlayer, const SpecialPowerTemplate *, Coord3D *, int, float>(
        PICK_ADDRESS(0x0057E0F0, 0x00A50E2C), this, special_power, pos, player_index, f);
#else
    return false;
#endif
}

void AISkirmishPlayer::Update()
{
    AIPlayer::Update();
}

void AISkirmishPlayer::New_Map()
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer>(PICK_ADDRESS(0x0057DF10, 0x00A50C21), this);
#endif
}

void AISkirmishPlayer::On_Unit_Produced(Object *factory, Object *unit)
{
    AIPlayer::On_Unit_Produced(factory, unit);
}

void AISkirmishPlayer::Build_Specific_AI_Team(TeamPrototype *team_proto, bool priority_build)
{
    AIPlayer::Build_Specific_AI_Team(team_proto, priority_build);
}

void AISkirmishPlayer::Build_AI_Base_Defense(bool b)
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer, bool>(PICK_ADDRESS(0x0057CC70, 0x00A4F96B), this, b);
#endif
}

void AISkirmishPlayer::Build_AI_Base_Defense_Structure(const Utf8String &name, bool b)
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer, const Utf8String &, bool>(PICK_ADDRESS(0x0057CDE0, 0x00A4FA5E), this, name, b);
#endif
}

void AISkirmishPlayer::Build_Specific_AI_Building(const Utf8String &name)
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer, const Utf8String &>(PICK_ADDRESS(0x0057C5A0, 0x00A4F285), this, name);
#endif
}

void AISkirmishPlayer::Recruit_Specific_AI_Team(TeamPrototype *team_proto, float recruit_radius)
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer, TeamPrototype *, float>(
        PICK_ADDRESS(0x0057D2F0, 0x00A5013A), this, team_proto, recruit_radius);
#endif
}

Player *AISkirmishPlayer::Get_Ai_Enemy()
{
#ifdef GAME_DLL
    return Call_Method<Player *, AISkirmishPlayer>(PICK_ADDRESS(0x0057CC40, 0x00A4F920), this);
#else
    return nullptr;
#endif
}

bool AISkirmishPlayer::Check_Bridges(Object *obj, Waypoint *waypoint)
{
#ifdef GAME_DLL
    return Call_Method<bool, AISkirmishPlayer, Object *, Waypoint *>(
        PICK_ADDRESS(0x0057D220, 0x00A50038), this, obj, waypoint);
#else
    return false;
#endif
}

void AISkirmishPlayer::Do_Base_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer>(PICK_ADDRESS(0x0057DAD0, 0x00A50622), this);
#endif
}

void AISkirmishPlayer::Check_Ready_Teams()
{
    AIPlayer::Check_Ready_Teams();
}

void AISkirmishPlayer::Check_Queued_Teams()
{
    AIPlayer::Check_Queued_Teams();
}

void AISkirmishPlayer::Do_Team_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer>(PICK_ADDRESS(0x0057DB60, 0x00A506F5), this);
#endif
}

Object *AISkirmishPlayer::Find_Dozer(const Coord3D *pos)
{
    return AIPlayer::Find_Dozer(pos);
}

void AISkirmishPlayer::Queue_Dozer()
{
    AIPlayer::Queue_Dozer();
}

bool AISkirmishPlayer::Select_Team_To_Build()
{
    return AIPlayer::Select_Team_To_Build();
}

bool AISkirmishPlayer::Select_Team_To_Reinforce(int min_priority)
{
    return AIPlayer::Select_Team_To_Reinforce(min_priority);
}

bool AISkirmishPlayer::Start_Training(WorkOrder *order, bool busy_ok, Utf8String team_name)
{
#ifdef GAME_DLL
    return Call_Method<bool, AISkirmishPlayer, WorkOrder *, bool, Utf8String>(
        PICK_ADDRESS(0x0057C1C0, 0x00A4EF49), this, order, busy_ok, team_name);
#else
    return false;
#endif
}

bool AISkirmishPlayer::Is_A_Good_Idea_To_Build_Team(TeamPrototype *team_proto)
{
#ifdef GAME_DLL
    return Call_Method<bool, AISkirmishPlayer, TeamPrototype *>(PICK_ADDRESS(0x0057C3A0, 0x00A4F086), this, team_proto);
#else
    return false;
#endif
}

void AISkirmishPlayer::Process_Base_Building()
{
#ifdef GAME_DLL
    Call_Method<void, AISkirmishPlayer>(PICK_ADDRESS(0x0057BBC0, 0x00A4E781), this);
#endif
}

void AISkirmishPlayer::Process_Team_Building()
{
    AIPlayer::Process_Team_Building();
}

void AISkirmishPlayer::Xfer_Snapshot(Xfer *xfer)
{
#ifdef GAME_DLL
    Call_Method<void, SnapShot, Xfer *>(PICK_ADDRESS(0x0057E3B0, 0x00A510A8), this, xfer);
#endif
}

bool AIPlayer::Is_Supply_Source_Safe(int source)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, int>(PICK_ADDRESS(0x005807E0, 0x00A46205), this, source);
#else
    return false;
#endif
}

bool AIPlayer::Is_Supply_Source_Attacked()
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer>(PICK_ADDRESS(0x005806C0, 0x00A46005), this);
#else
    return false;
#endif
}

void AIPlayer::AI_Pre_Team_Destroy(const Team *team)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, const Team *>(PICK_ADDRESS(0x00580440, 0x00A45DC7), this, team);
#endif
}

void AIPlayer::Guard_Supply_Center(Team *team, int center)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, Team *, int>(PICK_ADDRESS(0x00580580, 0x00A45EC2), this, team, center);
#endif
}

void AIPlayer::Build_By_Supplies(int supplies, const Utf8String &name)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, int, const Utf8String &>(PICK_ADDRESS(0x00582E30, 0x00A48497), this, supplies, name);
#endif
}

void AIPlayer::Build_Specific_Building_Nearest_Team(const Utf8String &name, const Team *team)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, const Utf8String &, const Team *>(PICK_ADDRESS(0x00583680, 0x00A48D85), this, name, team);
#endif
}

void AIPlayer::Build_Upgrade(const Utf8String &name)
{
#ifdef GAME_DLL
    Call_Method<void, AIPlayer, const Utf8String &>(PICK_ADDRESS(0x00582660, 0x00A47F9D), this, name);
#endif
}

bool AIPlayer::Calc_Closest_Construction_Zone_Location(const ThingTemplate *tmplate, Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<bool, AIPlayer, const ThingTemplate *, Coord3D *>(
        PICK_ADDRESS(0x005832C0, 0x00A48A63), this, tmplate, pos);
#else
    return false;
#endif
}
