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
#include "snapshot.h"
#include "unicodestring.h"
#include <list>
#include <map>

class PlayerTemplate;
class SpecialPowerTemplate;
class UpgradeTemplate;
class Object;
class Waypoint;
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

class AcademyStats
{
    int members[45];
};

class ScoreKeeper
{
    int members[100];
};

enum KindOfType : int32_t;
enum NameKeyType : int32_t;
enum VeterancyLevel;
enum Relationship;
enum ScienceType;
// enum SpecialPowerType;
// enum ScienceAvailabilityType;
// enum BattlePlanStatus;
// enum UpgradeStatusType;

enum PlayerType
{
    PLAYER_TYPE_UNK = 0,
};

// #TODO this is just a skeleton to support other objects at the moment.

class Player : public SnapShot
{
public:
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
    Team *Get_Default_Team() const;
    // int Get_Current_Enemy() const;
    // int Get_Player_Difficulty() const;

    // bool Is_Player_Active() const;
    // bool Is_Skirmish_AI_Player() const;
    // bool Is_Player_Observer() const;
    // bool Is_Player_Dead() const;
    // bool Is_Playable_Side() const;
    // bool Is_Local_Player() const;
    // bool Has_Any_Shortcut_Special_Power() const;
    // bool Has_Any_Buildings() const;
    // bool Has_Any_Buildings(BitFlags<116ul>) const;
    // bool Has_Any_Units() const;
    // bool Has_Any_Objects() const;
    // bool Has_Any_Build_Facility() const;

    // void On_Unit_Created(Object *, Object *);
    // void On_Structure_Created(Object *, Object *);
    // void On_Structure_Undone(Object *);
    // void On_Structure_Construction_Complete(Object *, Object *, char);
    // void On_Upgrade_Completed(const UpgradeTemplate *);
    void On_Power_Brown_Out_Change(bool b);

    // bool Is_Supply_Source_Safe(int) const;
    // bool Is_Supply_Source_Attacked() const;
    // void Guard_Supply_Center(Team *, int);
    // int Get_Supply_Box_Value() const;

    // void Set_Team_Delay_Seconds(int);
    // void Pre_Team_Destroy(const Team *);

    // void Add_Team_To_List(TeamPrototype *);
    // void Remove_Team_From_List(TeamPrototype *);
    // void Set_Default_Team();
    // void Becoming_Team_Member(Object *, char);
    // void Set_Player_Relationship(const Player *, Relationship);
    // void Remove_Player_Relationship(const Player *);
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
    // void Build_Base_Defense(char);
    // void Build_Base_Defense_Structure(const Utf8String &, char);
    // void Build_Specific_Building(const Utf8String &);
    // void Build_By_Supplies(const Utf8String &);
    // void Build_Specific_Building_Nearest_Team(const Utf8String &, const Team *);

    // void Set_Build_List(BuildListInfo *);
    // void Add_ToPriority_Build_List(Utf8String, Coord3D *, float);
    // void Add_To_Build_List(Object *);

    // void Add_Upgrade(UpgradeTemplate const *, UpgradeStatusType);
    // void Remove_Upgrade(UpgradeTemplate const *);
    // void Delete_Upgrade_List();
    // void Build_Upgrade(const Utf8String &);
    // int Find_Upgrade(const UpgradeTemplate *) const;
    // bool Has_Upgrade_In_Production(UpgradeTemplate const *) const;
    // bool Has_Upgrade_Complete(BitFlags<128ul>) const;
    // bool Has_Upgrade_Complete(const UpgradeTemplate *) const;

    // void Add_Kind_Of_Production_Cost_Change(BitFlags<116ul>, float);
    // void Remove_Kind_Of_Production_Cost_Change(BitFlags<116ul>, float);
    // int Get_Production_Cost_Change_Based_On_Kind_Of(BitFlags<116ul>) const;
    // int Get_Production_Veterancy_Level(Utf8String) const;
    // int Get_Production_Cost_Change_Percent(Utf8String) const;
    // int Get_Production_Time_Change_Percent(Utf8String) const;

    // bool Calc_Closest_Construction_Zone_Location(ThingTemplate const *, Coord3D *) const;

    // void Set_List_In_Score_Screen(char);
    // char Get_List_In_Score_Screen() const;

    // void Add_Radar(char);
    // void Remove_Radar(char);
    // void Enable_Radar();
    // void Disable_Radar();
    // bool Has_Radar() const;
    // bool Ok_To_Play_Radar_Edge_Sound() const;

    // void Add_AIGroup_To_Current_Selection(AIGroup *);
    // void Set_Currently_Selected_AIGroup(AIGroup *);
    // int Get_Current_Selection_As_AI_Group(AIGroup *) const;

    // int Get_Hotkey_Squad(int) const;
    int Get_Squad_Number_For_Object(const Object *obj) const;
    // void Remove_Object_From_Hotkey_Squad(Object *);

    // int Count_Buildings() const;
    // int Count_Objects(BitFlags<116ul>, BitFlags<116ul>) const;
    // int Count_Ready_Shortcut_SpecialPowers_Of_Type(SpecialPowerType) const;
    // int Count_Objects_By_ThingTemplate(int, ThingTemplate const *const *, char, int *, char) const;
    // void Iterate_Objects(void (*)(Object *, void *), void *) const;

    // int Find_Any_Existing_Object_With_ThingTemplate(ThingTemplate const *) const;
    // int Find_Most_Ready_Shortcut_SpecialPower_For_Thing(ThingTemplate const *, unsigned int &) const;
    // int Find_Most_Ready_Shortcut_Weapon_For_Thing(ThingTemplate const *, unsigned int &) const;
    // int Find_Most_Ready_Shortcut_SpecialPower_Of_Type(SpecialPowerType) const;
    // int Find_Natural_CommandCenter() const;
    // int Find_Closest_By_Kind_Of(Object *, BitFlags<116ul>, BitFlags<116ul>) const;

    // void Set_Player_Type(PlayerType, char);
    // void Set_Attacked_By(int);
    // int Get_Attacked_By(int) const;
    // void Set_Objects_Enabled(Utf8String, char);
    // void Set_Units_Should_Idle_Or_Resume(char);
    // void Set_Units_Vision_Spied(char, BitFlags<116ul>, int);
    // void Set_Units_Should_Hunt(char, CommandSourceType);
    // void Garrison_All_Units(CommandSourceType);
    // void Ungarrison_All_Units(CommandSourceType);
    // void Do_Bounty_For_Kill(Object const *, Object const *);

    // void Heal_All_Objects();
    // void Sell_Everything_Under_The_Sun();
    // void Transfer_Assets_From_That(Player *);
    void Becoming_Local_Player(bool b);
    // void Kill_Player();

    // void Add_New_Shared_SpecialPower_Timer(const SpecialPowerTemplate *, unsigned int);
    // int Get_Or_Start_SpecialPower_Ready_Frame(const SpecialPowerTemplate *) const;
    // int Express_SpecialPower_Ready_Frame(const SpecialPowerTemplate *, unsigned int);
    // void Reset_Or_Start_SpecialPower_Ready_Frame(const SpecialPowerTemplate *);

    // bool Does_Object_Qualify_For_Battle_Plan(Object *) const;
    // int Get_Battle_Plans_Active_Specific(BattlePlanStatus) const;
    // void Apply_Battle_Plan_Bonuses_For_Object(Object *);
    // void Apply_Battle_Plan_Bonuses_For_Player_Objects(const BattlePlanBonuses *);
    // void Change_Battle_Plan(BattlePlanStatus, int, BattlePlanBonuses *);
    // void Remove_Battle_Plan_Bonuses_For_Object(Object *);

    // void Friend_Apply_Difficulty_Bonuses_For_Object(Object *, char);

    // bool Has_Science(ScienceType) const;
    // bool Has_Prereqs_For_Science(ScienceType) const;
    // bool Is_Capable_Of_Purchasing_Science(ScienceType) const;
    // bool Is_Science_Hidden(ScienceType) const;
    // bool Is_Science_Disabled(ScienceType) const;
    // void Add_Science_Purchase_Points(int);
    // void Add_Science(ScienceType);
    // void Grant_Science(ScienceType);
    // void Attempt_To_Purchase_Science(ScienceType);
    // void Set_Science_Availability(ScienceType, ScienceAvailabilityType);
    // int Get_Science_Availability_Type_From_String(const Utf8String &) const;
    // void Reset_Sciences();

    // void Reset_Rank();
    // void Set_Rank_Level(int);

    // void Friend_Set_Skillset(int);
    // void Add_Skill_Points(int);
    // void Add_Skill_Points_For_Kill(const Object *, const Object *);

public:
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
    int m_unk_0x0048; // int m_radarCount2;
    int m_unk_0x004C; // bool m_hasRadar;
    int m_unk_0x0050; // int m_activeBattlePlans[3];
    int m_unk_0x0054;
    int m_unk_0x0058;
    BattlePlanBonuses *m_battlePlanBonuses;
    int m_unk_0x0060; // int m_upgradesInProgress[4];
    int m_unk_0x0064;
    int m_unk_0x0068;
    int m_unk_0x006C;
    int m_unk_0x0070; // int m_upgradesCompleted[4];
    int m_unk_0x0074;
    int m_unk_0x0078;
    int m_unk_0x007C;
    Energy m_energy;
    MissionStats m_missionStats;
    BuildListInfo *m_buildListInfo;
    int m_playerColor;
    int m_unk_0x0128; // int m_playerNightColor;
    std::map<NameKeyType, float> m_productionCostChanges;
    std::map<NameKeyType, float> m_productionTimeChanges;
    std::map<NameKeyType, VeterancyLevel> m_productionVeterancyLevels;
    void *m_ai; // 4 byte type unknown
    int m_multiplayerStartIndex;
    ResourceGatheringManager *m_resourceGatheringManager;
    void *m_tunnelSystem; // 4 byte type unknown
    Team *m_defaultTeam;
    int m_unk_0x0164; // std::vector<int> m_sciences;
    int m_unk_0x0168;
    int m_unk_0x016C;
    int m_unk_0x0170; // std::vector<int> m_disabledSciences;
    int m_unk_0x0174;
    int m_unk_0x0178;
    int m_unk_0x017C; // std::vector<int> m_hiddenSciences;
    int m_unk_0x0180;
    int m_unk_0x0184;
    int m_unk_0x0188; // int m_rankLevel;
    int m_unk_0x018C; // int m_skillPoints1;
    int m_unk_0x0190; // int m_sciencePurchasePoints;
    int m_unk_0x0194; // int m_skillPoints2;
    int m_unk_0x0198; // int m_rankProgress;
    int m_unk_0x019C; // Utf16String scienceGeneralName;
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
    char m_unk_0x026A[18]; // char m_attackedBy[16];
    int m_unk_0x027C; // int m_whenWasAttackedBySet;
    int m_unk_0x0280; // float m_bountyCostToBuild;
#if GAME_DEBUG_STRUCTS
    bool m_ignorePrereqs;
    bool m_freeBuild;
    bool m_instantBuild;
#endif
    ScoreKeeper m_scoreKeeper;
    void *m_kindOfPercentProductionChangeList; // 4 byte type unknown
    void *m_specialPowerReadyTimerList; // 4 byte type unknown
    Squad *m_squads[10];
    Squad *m_aiSquad;
    int m_unk_0x044C; // bool m_playerIsDead;
                      // bool m_retaliationModeEnabled;
};
