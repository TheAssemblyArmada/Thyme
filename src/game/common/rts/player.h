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

#include "always.h"
#include "academystats.h"
#include "energy.h"
#include "handicap.h"
#include "missionstats.h"
#include "money.h"
#include "object.h"
#include "scorekeeper.h"
#include "snapshot.h"
#include "unicodestring.h"
#include "upgrade.h"
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
class ResourceGatheringManager;
class PlayerRelationMap;
class TeamRelationMap;
class TunnelTracker;
class Upgrade;

class KindOfPercentProductionChange : public MemoryPoolObject
{
    IMPLEMENT_POOL(KindOfPercentProductionChange)

public:
    KindOfPercentProductionChange() {}
    virtual ~KindOfPercentProductionChange() override {}

private:
    BitFlags<KINDOF_COUNT> m_flags;
    float m_percent;
    unsigned int m_count;

    friend class Player;
};

class BattlePlanBonuses : public MemoryPoolObject // not 100% sure on this class
{
    IMPLEMENT_POOL(BattlePlanBonuses)

public:
    BattlePlanBonuses() {}
    virtual ~BattlePlanBonuses() override {}

    float m_armorBonus;
    int m_bombardment;
    int m_searchAndDestroy;
    int m_holdTheLine;
    float m_sightBonus;
    BitFlags<KINDOF_COUNT> m_validKindOf;
    BitFlags<KINDOF_COUNT> m_invalidKindOf;
};

struct SpecialPowerReadyTimerType
{
    SpecialPowerReadyTimerType() { Reset(); }

    void Reset()
    {
        m_frame = 0xFFFFFFFF;
        m_id = 0;
    }

    unsigned int m_id;
    unsigned int m_frame;
};

enum ScienceAvailabilityType
{
    SCIENCE_AVAILABILITY_INVALID = -1,
    SCIENCE_AVAILABILITY_AVAILABLE,
    SCIENCE_AVAILABILITY_DISABLED,
    SCIENCE_AVAILABILITY_HIDDEN,
};

enum BattlePlanStatus
{
    BATTLE_PLAN_STATUS_NONE,
    BATTLE_PLAN_STATUS_BOMBARDMENT,
    BATTLE_PLAN_STATUS_HOLD_THE_LINE,
    BATTLE_PLAN_STATUS_SEARCH_AND_DESTROY,
};

class Player : public SnapShot
{
public:
    enum PlayerType
    {
        PLAYER_HUMAN = 0,
        PLAYER_COMPUTER = 1,
    };

    enum
    {
        SQUAD_COUNT = 10
    };

    Player(int32_t player_index);
    virtual ~Player();

    // SnapShot
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}
    //~SnapShot

    void Init(const PlayerTemplate *pt);
    void Init_From_Dict(const Dict *d);
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
    Energy *Get_Energy() { return &m_energy; }
    PlayerType Get_Player_Type() const { return m_playerType; }
    const PlayerTemplate *Get_Player_Template() const { return m_playerTemplate; }
    AcademyStats *Get_Academy_Stats() { return &m_academy; }
    Utf8String Get_Side() const { return m_side; }
    int Get_Multiplayer_Start_Index() const { return m_multiplayerStartIndex; }
    const Money *Get_Money() const { return &m_money; }
    Money *Get_Money() { return &m_money; }
    int Get_Current_Skill_Points() const { return m_currentSkillPoints; }
    int Get_Skill_Points_Needed_For_Next_Rank() const { return m_skillPointsNeededForNextRank; }
    int Get_Rank_Progress() const { return m_rankProgress; }
    int Get_Night_Color() const { return m_playerNightColor; }
    Utf16String Get_Player_Display_Name() const { return m_playerDisplayName; }
    BitFlags<128> Get_Upgrades_Completed() const { return m_upgradesCompleted; }
    ScoreKeeper *Get_Score_Keeper() { return &m_scoreKeeper; }
    bool Get_Units_Should_Hunt() const { return m_unitsShouldHunt; }
    const std::list<TeamPrototype *> *Get_Player_Team_Prototype_List() const { return &m_playerTeamPrototypes; }
    ResourceGatheringManager *Get_Resource_Gathering_Manager() const { return m_resourceGatheringManager; }
    BuildListInfo *Get_Build_List() { return m_buildListInfo; }
    bool Get_Can_Build_Units() const { return m_canBuildUnits; }
    bool Get_Can_Build_Base() const { return m_canBuildBase; }
    int Get_Last_Attacked_By_Frame() const { return m_lastAttackedByFrame; }
    int Get_Science_Purchase_Points() const { return m_sciencePurchasePoints; }
    int Get_Rank_Level() const { return m_rankLevel; }

    void Set_Retaliation_Mode_Enabled(bool set) { m_retaliationModeEnabled = set; }
    void Set_Can_Build_Units(bool set) { m_canBuildUnits = set; }
    void Set_Can_Build_Base(bool set) { m_canBuildBase = set; }
    void Set_Skill_Points_Modifer(int set) { m_skillPointsModifier = set; }

    int Get_Total_Battle_Plan_Count() const
    {
        return m_activeBattlePlans[2] + m_activeBattlePlans[1] + m_activeBattlePlans[0];
    }

    bool Is_Player_Active() const { return !m_playerIsObserver && !m_playerIsDead; }
    bool Is_Retaliation_Mode_Enabled() const { return m_retaliationModeEnabled; }

    void On_Upgrade_Removed() {}

    void Add_Power_Bonus(Object *obj) { m_energy.Add_Power_Bonus(obj); }
    void Remove_Power_Bonus(Object *obj) { m_energy.Add_Power_Bonus(obj); }

    Player *Get_Current_Enemy();
    GameDifficulty Get_Player_Difficulty() const;

    bool Is_Skirmish_AI_Player() const;
    bool Is_Player_Observer() const;
    bool Is_Player_Dead() const;
    bool Is_Playable_Side() const;
    bool Is_Local_Player() const;
    bool Has_Any_Shortcut_Special_Power() const;
    bool Has_Any_Buildings() const;
    bool Has_Any_Buildings(BitFlags<KINDOF_COUNT> must_be_set) const;
    bool Has_Any_Units() const;
    bool Has_Any_Objects() const;
    bool Has_Any_Build_Facility() const;

    void On_Unit_Created(Object *factory, Object *unit);
    void On_Structure_Created(Object *builder, Object *structure);
    void On_Structure_Undone(Object *structure);
    void On_Structure_Construction_Complete(Object *builder, Object *structure, bool add_score);
    void On_Upgrade_Completed(const UpgradeTemplate *upgrade_template);
    void On_Power_Brown_Out_Change(bool power_disable);

    bool Is_Supply_Source_Safe(int source) const;
    bool Is_Supply_Source_Attacked() const;
    void Guard_Supply_Center(Team *team, int center);
    unsigned int Get_Supply_Box_Value() const;

    void Set_Team_Delay_Seconds(int seconds);
    void Pre_Team_Destroy(const Team *team);

    void Add_Team_To_List(TeamPrototype *team);
    void Remove_Team_From_List(TeamPrototype *team);
    void Set_Default_Team();
    void Becoming_Team_Member(Object *thg, bool yes);
    void Set_Player_Relationship(const Player *that, Relationship r);
    bool Remove_Player_Relationship(const Player *that);
    void Set_Team_Relationship(const Team *that, Relationship r);
    bool Remove_Team_Relationship(const Team *that);
    Relationship Get_Relationship(const Team *that) const;
    void Update_Team_States();

    void Process_Create_Team_GameMessage(int squad, GameMessage *message);
    void Process_Add_Team_GameMessage(int squad, GameMessage *message);
    void Process_Select_Team_GameMessage(int squad, GameMessage *message);

    bool Allowed_To_Build(const ThingTemplate *tmplate) const;
    bool Can_Afford_Build(const ThingTemplate *tmplate) const;
    bool Can_Build(const ThingTemplate *tmplate) const;
    bool Can_Build_More_Of_Type(const ThingTemplate *tmplate) const;
    void Build_Specific_Team(TeamPrototype *team_proto);
    void Recruit_Specific_Team(TeamPrototype *team_proto, float recruit_radius);
    void Build_Base_Defense(bool build);
    void Build_Base_Defense_Structure(const Utf8String &name, bool build);
    void Build_Specific_Building(const Utf8String &name);
    void Build_By_Supplies(int supplies, const Utf8String &name);
    void Build_Specific_Building_Nearest_Team(const Utf8String &name, const Team *team);

    void Set_Build_List(BuildListInfo *build_list);
    void Add_To_Priority_Build_List(Utf8String template_name, Coord3D *loc, float angle);
    void Add_To_Build_List(Object *obj);

    Upgrade *Add_Upgrade(const UpgradeTemplate *upgrade_template, UpgradeStatusType status);
    void Remove_Upgrade(const UpgradeTemplate *upgrade_template);
    void Delete_Upgrade_List();
    void Build_Upgrade(const Utf8String &upgrade);
    Upgrade *Find_Upgrade(const UpgradeTemplate *upgrade_template);
    bool Has_Upgrade_In_Production(const UpgradeTemplate *upgrade_template) const;
    bool Has_Upgrade_Complete(BitFlags<128> test_mask) const;
    bool Has_Upgrade_Complete(const UpgradeTemplate *upgrade_template) const;

    void Add_Kind_Of_Production_Cost_Change(BitFlags<KINDOF_COUNT> flags, float percent);
    void Remove_Kind_Of_Production_Cost_Change(BitFlags<KINDOF_COUNT> flags, float percent);
    float Get_Production_Cost_Change_Based_On_Kind_Of(BitFlags<KINDOF_COUNT> flags) const;
    VeterancyLevel Get_Production_Veterancy_Level(Utf8String build_template_name) const;
    float Get_Production_Cost_Change_Percent(Utf8String build_template_name) const;
    float Get_Production_Time_Change_Percent(Utf8String build_template_name) const;

    bool Calc_Closest_Construction_Zone_Location(const ThingTemplate *tmplate, Coord3D *pos) const;

    void Set_List_In_Score_Screen(bool list_in_score_screen);
    bool Get_List_In_Score_Screen() const;

    void Add_Radar(bool disable_proof);
    void Remove_Radar(bool disable_proof);
    void Enable_Radar();
    void Disable_Radar();
    bool Has_Radar() const;
    bool Ok_To_Play_Radar_Edge_Sound();

    void Add_AIGroup_To_Current_Selection(AIGroup *group);
    void Set_Currently_Selected_AIGroup(AIGroup *group);
    void Get_Current_Selection_As_AI_Group(AIGroup *group) const;

    Squad *Get_Hotkey_Squad(int squad);
    int Get_Squad_Number_For_Object(const Object *obj) const;
    void Remove_Object_From_Hotkey_Squad(Object *obj);

    int Count_Buildings() const;
    int Count_Objects(BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear) const;
    int Count_Ready_Shortcut_Special_Powers_Of_Type(SpecialPowerType type) const;
    void Count_Objects_By_Thing_Template(int num_tmplates,
        ThingTemplate const *const *things,
        bool ignore_dead,
        int *counts,
        bool ignore_under_construction) const;
    void Iterate_Objects(void (*func)(Object *, void *), void *data) const;

    Object *Find_Any_Existing_Object_With_Thing_Template(const ThingTemplate *tmplate);
    Object *Find_Most_Ready_Shortcut_Special_Power_For_Thing(const ThingTemplate *tmplate, unsigned int &percent);
    Object *Find_Most_Ready_Shortcut_Weapon_For_Thing(const ThingTemplate *tmplate, unsigned int &percent);
    Object *Find_Most_Ready_Shortcut_Special_Power_Of_Type(SpecialPowerType type);
    Object *Find_Natural_Command_Center();
    Object *Find_Closest_By_Kind_Of(Object *obj, BitFlags<KINDOF_COUNT> must_be_set, BitFlags<KINDOF_COUNT> must_be_clear);

    void Set_Player_Type(PlayerType t, bool is_skirmish);
    void Set_Attacked_By(int player);
    bool Get_Attacked_By(int player) const;
    void Set_Objects_Enabled(Utf8String template_type_to_affect, bool enable);
    void Set_Units_Should_Idle_Or_Resume(bool should_idle_or_resume);
    void Set_Units_Vision_Spied(bool vision_spied, BitFlags<KINDOF_COUNT> flags, int player);
    void Set_Units_Should_Hunt(bool should_hunt, CommandSourceType source);
    void Garrison_All_Units(CommandSourceType source);
    void Ungarrison_All_Units(CommandSourceType source);
    void Do_Bounty_For_Kill(const Object *killer, const Object *killed);

    void Heal_All_Objects();
    void Sell_Everything_Under_The_Sun();
    void Transfer_Assets_From_That(Player *that);
    void Becoming_Local_Player(bool yes);
    void Kill_Player();

    void Add_New_Shared_Special_Power_Timer(const SpecialPowerTemplate *tmplate, unsigned int frame);
    unsigned int Get_Or_Start_Special_Power_Ready_Frame(const SpecialPowerTemplate *tmplate);
    void Express_Special_Power_Ready_Frame(const SpecialPowerTemplate *tmplate, unsigned int frame);
    void Reset_Or_Start_Special_Power_Ready_Frame(const SpecialPowerTemplate *tmplate);

    bool Does_Object_Qualify_For_Battle_Plan(Object *obj) const;
    int Get_Battle_Plans_Active_Specific(BattlePlanStatus status) const;
    void Apply_Battle_Plan_Bonuses_For_Object(Object *obj);
    void Apply_Battle_Plan_Bonuses_For_Player_Objects(const BattlePlanBonuses *bonuses);
    void Change_Battle_Plan(BattlePlanStatus status, int count, BattlePlanBonuses *bonuses);
    void Remove_Battle_Plan_Bonuses_For_Object(Object *obj);

    void Friend_Apply_Difficulty_Bonuses_For_Object(Object *obj, bool apply);

    bool Has_Science(ScienceType t) const;
    bool Has_Prereqs_For_Science(ScienceType t) const;
    bool Is_Capable_Of_Purchasing_Science(ScienceType t) const;
    bool Is_Science_Hidden(ScienceType t) const;
    bool Is_Science_Disabled(ScienceType t) const;
    void Add_Science_Purchase_Points(int points);
    bool Add_Science(ScienceType t);
    bool Grant_Science(ScienceType t);
    bool Attempt_To_Purchase_Science(ScienceType t);
    void Set_Science_Availability(ScienceType t, ScienceAvailabilityType a);
    ScienceAvailabilityType Get_Science_Availability_Type_From_String(const Utf8String &availability) const;
    void Reset_Sciences();

    void Reset_Rank();
    bool Set_Rank_Level(int rank);

    void Friend_Set_Skillset(int skillset);
    bool Add_Skill_Points(int points);
    bool Add_Skill_Points_For_Kill(const Object *killer, const Object *killed);

#ifdef GAME_DEBUG_STRUCTS
    bool Is_Instant_Build() const { return m_instantBuild; }
    bool Is_Ignore_Prereqs() const { return m_ignorePrereqs; }
    void Toggle_Ignore_Prereqs() { m_ignorePrereqs = !m_ignorePrereqs; }
    void Toggle_Free_Build() { m_freeBuild = !m_freeBuild; }
    void Toggle_Instant_Build() { m_instantBuild = !m_instantBuild; }
#endif

private:
    const PlayerTemplate *m_playerTemplate;
    Utf16String m_playerDisplayName;
    Handicap m_handicap;
    Utf8String m_playerName;
    NameKeyType m_playerNameKey;
    int m_playerIndex;
    Utf8String m_side;
    Utf8String m_baseSide;
    PlayerType m_playerType;
    Money m_money;
    Upgrade *m_upgradeList;
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
    int m_rankProgress; // not 100% sure yet
    Utf16String m_scienceGeneralName; // not 100% sure yet
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
    float m_bountyCostToBuild; // not 100% sure yet, set by CashBountyPower::onObjectCreated
#if GAME_DEBUG_STRUCTS
    bool m_ignorePrereqs;
    bool m_freeBuild;
    bool m_instantBuild;
#endif
    ScoreKeeper m_scoreKeeper;
    std::list<KindOfPercentProductionChange *> m_kindOfPercentProductionChangeList;
    std::list<SpecialPowerReadyTimerType> m_specialPowerReadyTimerList;
    Squad *m_squads[SQUAD_COUNT];
    Squad *m_aiSquad;
    bool m_playerIsDead;
    bool m_retaliationModeEnabled; // not 100% sure yet
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
