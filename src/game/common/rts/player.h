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
#pragma once

#include "energy.h"
#include "handicap.h"
#include "missionstats.h"
#include "money.h"
#include "scorekeeper.h"
#include "snapshot.h"
#include "unicodestring.h"
#include <list>
#include <map>

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class PlayerTemplate;
class SpecialPowerTemplate;
class UpgradeTemplate;
class Object;
class Waypoint;
class AIPlayer;
class AIGroup;
class Squad;
class Team;
class TeamPrototype;
class GameMessage;
class BuildListInfo;
class BattlePlanBonuses;
class ResourceGatheringManager;
class PlayerRelationMap;
class TeamRelationMap;
class TunnelTracker;
class KindOfPercentProductionChange;

struct SpecialPowerReadyTimerType // #TODO Implement and move elsewhere
{
    int m_unk1; // possibly m_id
    unsigned int m_unk2; // possibly m_frame
};

class AcademyStats // #TODO Implement and move elsewhere
{
public:
    char unk[0x9C];
    unsigned int m_minesClearedCount;
    char unk3[0x14];

    void Increment_Mines_Disarmed() { m_minesClearedCount++; }
};

enum KindOfType : int32_t;
enum NameKeyType : int32_t;
enum ScienceType : int32_t;
enum VeterancyLevel;
enum Relationship;
// enum GameDifficulty;
// enum SpecialPowerType;
// enum ScienceAvailabilityType;
// enum BattlePlanStatus;
// enum UpgradeStatusType;

// #TODO this is just a skeleton to support other objects at the moment.

class Player : public SnapShot
{
public:
    enum PlayerType
    {
        PLAYER_HUMAN = 0,
        PLAYER_COMPUTER = 1,
    };

    Player(int32_t player_index);
    virtual ~Player();

    // SnapShot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    //~SnapShot

    void Init(const PlayerTemplate *player_template);
    // void Init_From_Dict(Dict const *);
    void Update();
    void New_Map();

    virtual bool Compute_Superweapon_Target(const SpecialPowerTemplate *sp_template, Coord3D *loc, int32_t unk1, float unk2);
    virtual bool Check_Bridges(Object *obj, Waypoint *waypoint);
    virtual bool Get_AI_Base_Center(Coord3D *center);
    virtual void Repair_Structure(ObjectID obj_id);

    NameKeyType Get_Player_NameKey() const;
    int Get_Player_Index() const;
    uint16_t Get_Player_Mask() const;
    int Get_Color() const;
    Team *Get_Default_Team();
    const Team *Get_Default_Team() const;

    const Handicap *Get_Handicap() const { return &m_handicap; }
    const Energy *Get_Energy() const { return &m_energy; }
    PlayerType Get_Player_Type() const { return m_playerType; }
    const PlayerTemplate *Get_Player_Template() const { return m_playerTemplate; }
    AcademyStats *Get_Academy_Stats() { return &m_academy; }

    int Get_Total_Battle_Plan_Count() const
    {
        return m_activeBattlePlans[2] + m_activeBattlePlans[1] + m_activeBattlePlans[0];
    }

    // Player *Get_Current_Enemy();
    GameDifficulty Get_Player_Difficulty() const;

    bool Is_Player_Active() const { return !m_playerIsObserver && !m_playerIsDead; }
    // bool Is_Skirmish_AI_Player() const;
    // bool Is_Player_Observer() const;
    // bool Is_Player_Dead() const;
    // bool Is_Playable_Side() const;
    // bool Is_Local_Player() const;
    // bool Has_Any_Shortcut_Special_Power() const;
    // bool Has_Any_Buildings() const;
    // bool Has_Any_Buildings(BitFlags<KINDOF_COUNT>) const;
    // bool Has_Any_Units() const;
    // bool Has_Any_Objects() const;
    // bool Has_Any_Build_Facility() const;

    // void On_Unit_Created(Object *, Object *);
    // void On_Structure_Created(Object *, Object *);
    // void On_Structure_Undone(Object *);
    // void On_Structure_Construction_Complete(Object *, Object *, bool);
    // void On_Upgrade_Completed(const UpgradeTemplate *);
    void On_Power_Brown_Out_Change(bool b);

    // bool Is_Supply_Source_Safe(int) const;
    // bool Is_Supply_Source_Attacked() const;
    // void Guard_Supply_Center(Team *, int);
    // unsigned int Get_Supply_Box_Value() const;

    // void Set_Team_Delay_Seconds(int);
    void Pre_Team_Destroy(const Team *team);

    void Add_Team_To_List(TeamPrototype *team);
    void Remove_Team_From_List(TeamPrototype *team);
    // void Set_Default_Team();
    // void Becoming_Team_Member(Object *, bool);
    // void Set_Player_Relationship(const Player *, Relationship);
    // bool Remove_Player_Relationship(const Player *);
    // void Set_Team_Relationship(const Team *, Relationship);
    bool Remove_Team_Relationship(const Team *team);
    Relationship Get_Relationship(const Team *team) const;
    void Update_Team_States();

    // void Process_Create_Team_GameMessage(int, GameMessage *);
    // void Process_Add_Team_GameMessage(int, GameMessage *);
    // void Process_Select_Team_GameMessage(int, GameMessage *);

    // bool Allowed_To_Build(const ThingTemplate *) const;
    // bool Can_Afford_Build(ThingTemplate const *) const;
    // bool Can_Build(const ThingTemplate *) const;
    // bool Can_Build_More_Of_Type(const ThingTemplate *) const;
    // void Build_Specific_Team(TeamPrototype *);
    // void Recruit_Specific_Team(TeamPrototype *, float);
    // void Build_Base_Defense(bool);
    // void Build_Base_Defense_Structure(const Utf8String &, bool);
    // void Build_Specific_Building(const Utf8String &);
    // void Build_By_Supplies(const Utf8String &);
    // void Build_Specific_Building_Nearest_Team(const Utf8String &, const Team *);

    // void Set_Build_List(BuildListInfo *);
    // void Add_ToPriority_Build_List(Utf8String, Coord3D *, float);
    // void Add_To_Build_List(Object *);

    // Upgrade *Add_Upgrade(UpgradeTemplate const *, UpgradeStatusType);
    // void Remove_Upgrade(UpgradeTemplate const *);
    // void Delete_Upgrade_List();
    // void Build_Upgrade(const Utf8String &);
    // Upgrade *Find_Upgrade(const UpgradeTemplate *);
    // bool Has_Upgrade_In_Production(UpgradeTemplate const *) const;
    // bool Has_Upgrade_Complete(BitFlags<128>) const;
    // bool Has_Upgrade_Complete(const UpgradeTemplate *) const;

    // void Add_Kind_Of_Production_Cost_Change(BitFlags<KINDOF_COUNT>, float);
    // void Remove_Kind_Of_Production_Cost_Change(BitFlags<KINDOF_COUNT>, float);
    float Get_Production_Cost_Change_Based_On_Kind_Of(BitFlags<KINDOF_COUNT> flags) const;
    // VeterancyLevel Get_Production_Veterancy_Level(Utf8String) const;
    float Get_Production_Cost_Change_Percent(Utf8String build_template_name) const;
    float Get_Production_Time_Change_Percent(Utf8String build_template_name) const;

    // bool Calc_Closest_Construction_Zone_Location(ThingTemplate const *, Coord3D *) const;

    // void Set_List_In_Score_Screen(bool);
    // bool Get_List_In_Score_Screen() const;

    // void Add_Radar(bool);
    // void Remove_Radar(bool);
    // void Enable_Radar();
    // void Disable_Radar();
    // bool Has_Radar() const;
    // bool Ok_To_Play_Radar_Edge_Sound() const;

    // void Add_AIGroup_To_Current_Selection(AIGroup *);
    // void Set_Currently_Selected_AIGroup(AIGroup *);
    // void Get_Current_Selection_As_AI_Group(AIGroup *) const;

    // Squad *Get_Hotkey_Squad(int);
    int Get_Squad_Number_For_Object(const Object *obj) const;
    // void Remove_Object_From_Hotkey_Squad(Object *);

    // int Count_Buildings() const;
    // int Count_Objects(BitFlags<KINDOF_COUNT>, BitFlags<KINDOF_COUNT>) const;
    // int Count_Ready_Shortcut_SpecialPowers_Of_Type(SpecialPowerType) const;
    void Count_Objects_By_ThingTemplate(
        int num_tmplates, ThingTemplate const *const *things, bool b1, int *counts, bool b2) const;
    void Iterate_Objects(void (*func)(Object *, void *), void *data) const;

    // Object *Find_Any_Existing_Object_With_ThingTemplate(ThingTemplate const *);
    // Object *Find_Most_Ready_Shortcut_SpecialPower_For_Thing(ThingTemplate const *, unsigned int &);
    // Object *Find_Most_Ready_Shortcut_Weapon_For_Thing(ThingTemplate const *, unsigned int &);
    // Object *Find_Most_Ready_Shortcut_SpecialPower_Of_Type(SpecialPowerType);
    // Object *Find_Natural_CommandCenter();
    // Object *Find_Closest_By_Kind_Of(Object *, BitFlags<KINDOF_COUNT>, BitFlags<KINDOF_COUNT>);

    // void Set_Player_Type(PlayerType, bool);
    // void Set_Attacked_By(int);
    // bool Get_Attacked_By(int) const;
    // void Set_Objects_Enabled(Utf8String, bool);
    // void Set_Units_Should_Idle_Or_Resume(bool);
    // void Set_Units_Vision_Spied(bool, BitFlags<KINDOF_COUNT>, int);
    // void Set_Units_Should_Hunt(bool, CommandSourceType);
    // void Garrison_All_Units(CommandSourceType);
    // void Ungarrison_All_Units(CommandSourceType);
    // void Do_Bounty_For_Kill(Object const *, Object const *);

    // void Heal_All_Objects();
    // void Sell_Everything_Under_The_Sun();
    // void Transfer_Assets_From_That(Player *);
    void Becoming_Local_Player(bool b);
    // void Kill_Player();

    // void Add_New_Shared_SpecialPower_Timer(const SpecialPowerTemplate *, unsigned int);
    // unsigned int Get_Or_Start_SpecialPower_Ready_Frame(const SpecialPowerTemplate *) const;
    // void Express_SpecialPower_Ready_Frame(const SpecialPowerTemplate *, unsigned int);
    // void Reset_Or_Start_SpecialPower_Ready_Frame(const SpecialPowerTemplate *);

    // bool Does_Object_Qualify_For_Battle_Plan(Object *) const;
    // int Get_Battle_Plans_Active_Specific(BattlePlanStatus) const;
    void Apply_Battle_Plan_Bonuses_For_Object(Object *obj);
    // void Apply_Battle_Plan_Bonuses_For_Player_Objects(const BattlePlanBonuses *);
    // void Change_Battle_Plan(BattlePlanStatus, int, BattlePlanBonuses *);
    // void Remove_Battle_Plan_Bonuses_For_Object(Object *);

    // void Friend_Apply_Difficulty_Bonuses_For_Object(Object *, bool);

    bool Has_Science(ScienceType t) const;
    // bool Has_Prereqs_For_Science(ScienceType) const;
    // bool Is_Capable_Of_Purchasing_Science(ScienceType) const;
    // bool Is_Science_Hidden(ScienceType) const;
    // bool Is_Science_Disabled(ScienceType) const;
    // void Add_Science_Purchase_Points(int);
    // bool Add_Science(ScienceType);
    // bool Grant_Science(ScienceType);
    // bool Attempt_To_Purchase_Science(ScienceType);
    // void Set_Science_Availability(ScienceType, ScienceAvailabilityType);
    // ScienceAvailabilityType Get_Science_Availability_Type_From_String(const Utf8String &) const;
    // void Reset_Sciences();

    // void Reset_Rank();
    // bool Set_Rank_Level(int);

    // void Friend_Set_Skillset(int);
    // bool Add_Skill_Points(int);
    // bool Add_Skill_Points_For_Kill(const Object *, const Object *);

#ifdef GAME_DEBUG_STRUCTS
    bool Is_Instant_Build() const { return m_instantBuild; }
#endif

private:
    PlayerTemplate *m_playerTemplate;
    Utf16String m_playerDisplayName;
    Handicap m_handicap;
    Utf8String m_playerName;
    NameKeyType m_playerNameKey;
    int m_playerIndex;
    Utf8String m_side;
    Utf8String m_baseSide;
    PlayerType m_playerType;
    Money m_money;
    int m_upgradeList;
    int m_radarCount;
    int m_disableProofRadarCount;
    bool m_radarDisabled;
    int m_activeBattlePlans[3];
    BattlePlanBonuses *m_battlePlanBonuses;
    BitFlags<128> m_upgradesInProgress;
    BitFlags<128> m_upgradesCompleted;
    Energy m_energy;
    MissionStats m_missionStats;
    BuildListInfo *m_buildListInfo;
    int m_playerColor;
    int m_playerNightColor;
    std::map<NameKeyType, float> m_productionCostChanges;
    std::map<NameKeyType, float> m_productionTimeChanges;
    std::map<NameKeyType, VeterancyLevel> m_productionVeterancyLevels;
    AIPlayer *m_ai;
    int m_multiplayerStartIndex;
    ResourceGatheringManager *m_resourceGatheringManager;
    TunnelTracker *m_tunnelSystem;
    Team *m_defaultTeam;
    std::vector<ScienceType> m_sciences;
    std::vector<ScienceType> m_disabledSciences;
    std::vector<ScienceType> m_hiddenSciences;
    int m_rankLevel;
    int m_currentSkillPoints;
    int m_sciencePurchasePoints;
    int m_skillPointsNeededForNextRank;
    int m_unk_0x0198; // int m_rankProgress;
    Utf16String m_unk_0x019C; // Utf16String m_scienceGeneralName;
    std::list<TeamPrototype *> m_playerTeamPrototypes;
    PlayerRelationMap *m_playerRelations;
    TeamRelationMap *m_teamRelations;
    AcademyStats m_academy;
    bool m_canBuildUnits;
    bool m_canBuildBase;
    bool m_playerIsObserver;
    bool m_playerIsPreorder;
    float m_skillPointsModifier;
    bool m_listInScoreScreen;
    bool m_unitsShouldHunt;
    bool m_attackedByPlayer[MAX_PLAYER_COUNT];
    int m_lastAttackedByFrame;
    int m_unk_0x0280; // float m_bountyCostToBuild; set by CashBountyPower::onObjectCreated
#if GAME_DEBUG_STRUCTS
    bool m_ignorePrereqs;
    bool m_freeBuild;
    bool m_instantBuild;
#endif
    ScoreKeeper m_scoreKeeper;
    std::list<KindOfPercentProductionChange *> m_kindOfPercentProductionChangeList;
    std::list<SpecialPowerReadyTimerType> m_specialPowerReadyTimerList;
    Squad *m_squads[10];
    Squad *m_aiSquad;
    bool m_playerIsDead;
    bool m_unk_0x044D; // bool m_retaliationModeEnabled;
};

class PlayerRelationMap : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_NAMED_POOL(PlayerRelationMap, PlayerRelationMapPool);

public:
    PlayerRelationMap() {}

    virtual ~PlayerRelationMap() override;
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

#ifdef THYME_USE_STLPORT
    std::hash_map<int, Relationship> m_relationships;
#else
    std::unordered_map<int, Relationship> m_relationships;
#endif
};
