/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class representing a script action.
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
#include "datachunk.h"
#include "mempoolobj.h"
#include "scriptparam.h"

class ObjectTypes;

class Condition : public MemoryPoolObject
{
    IMPLEMENT_POOL(Condition);

    enum
    {
        MAX_CONDITION_PARAMETERS = 12,
    };

public:
    enum ConditionType : int32_t
    {
        CONDITION_FALSE,
        COUNTER,
        FLAG,
        CONDITION_TRUE,
        TIMER_EXPIRED,
        PLAYER_ALL_DESTROYED,
        PLAYER_ALL_BUILDFACILITIES_DESTROYED,
        TEAM_INSIDE_AREA_PARTIALLY,
        TEAM_DESTROYED,
        CAMERA_MOVEMENT_FINISHED,
        TEAM_HAS_UNITS,
        TEAM_STATE_IS,
        TEAM_STATE_IS_NOT,
        NAMED_INSIDE_AREA,
        NAMED_OUTSIDE_AREA,
        NAMED_DESTROYED,
        NAMED_NOT_DESTROYED,
        TEAM_INSIDE_AREA_ENTIRELY,
        TEAM_OUTSIDE_AREA_ENTIRELY,
        NAMED_ATTACKED_BY_OBJECTTYPE,
        TEAM_ATTACKED_BY_OBJECTTYPE,
        NAMED_ATTACKED_BY_PLAYER,
        TEAM_ATTACKED_BY_PLAYER,
        BUILT_BY_PLAYER,
        NAMED_CREATED,
        TEAM_CREATED,
        PLAYER_HAS_CREDITS,
        NAMED_DISCOVERED,
        TEAM_DISCOVERED,
        MISSION_ATTEMPTS,
        NAMED_OWNED_BY_PLAYER,
        TEAM_OWNED_BY_PLAYER,
        PLAYER_HAS_N_OR_FEWER_BUILDINGS,
        PLAYER_HAS_POWER,
        NAMED_REACHED_WAYPOINTS_END,
        TEAM_REACHED_WAYPOINTS_END,
        NAMED_SELECTED,
        NAMED_ENTERED_AREA,
        NAMED_EXITED_AREA,
        TEAM_ENTERED_AREA_ENTIRELY,
        TEAM_ENTERED_AREA_PARTIALLY,
        TEAM_EXITED_AREA_ENTIRELY,
        TEAM_EXITED_AREA_PARTIALLY,
        MULTIPLAYER_ALLIED_VICTORY,
        MULTIPLAYER_ALLIED_DEFEAT,
        MULTIPLAYER_PLAYER_DEFEAT,
        PLAYER_HAS_NO_POWER,
        HAS_FINISHED_VIDEO,
        HAS_FINISHED_SPEECH,
        HAS_FINISHED_AUDIO,
        BUILDING_ENTERED_BY_PLAYER,
        ENEMY_SIGHTED,
        UNIT_HEALTH,
        BRIDGE_REPAIRED,
        BRIDGE_BROKEN,
        NAMED_DYING,
        NAMED_TOTALLY_DEAD,
        PLAYER_HAS_OBJECT_COMPARISON,
        OBSOLETE_SCRIPT_1,
        OBSOLETE_SCRIPT_2,
        PLAYER_TRIGGERED_SPECIAL_POWER,
        PLAYER_COMPLETED_SPECIAL_POWER,
        PLAYER_MIDWAY_SPECIAL_POWER,
        PLAYER_TRIGGERED_SPECIAL_POWER_FROM_NAMED,
        PLAYER_COMPLETED_SPECIAL_POWER_FROM_NAMED,
        PLAYER_MIDWAY_SPECIAL_POWER_FROM_NAMED,
        PLAYER_SELECTED_GENERAL,
        PLAYER_SELECTED_GENERAL_FROM_NAMED,
        PLAYER_BUILT_UPGRADE,
        PLAYER_BUILT_UPGRADE_FROM_NAMED,
        PLAYER_DESTROYED_N_BUILDINGS_PLAYER,
        UNIT_COMPLETED_SEQUENTIAL_EXECUTION,
        TEAM_COMPLETED_SEQUENTIAL_EXECUTION,
        PLAYER_HAS_COMPARISON_UNIT_TYPE_IN_TRIGGER_AREA,
        PLAYER_HAS_COMPARISON_UNIT_KIND_IN_TRIGGER_AREA,
        UNIT_EMPTIED,
        TYPE_SIGHTED,
        NAMED_BUILDING_IS_EMPTY,
        PLAYER_HAS_N_OR_FEWER_FACTION_BUILDINGS,
        UNIT_HAS_OBJECT_STATUS,
        TEAM_ALL_HAS_OBJECT_STATUS,
        TEAM_SOME_HAVE_OBJECT_STATUS,
        PLAYER_POWER_COMPARE_PERCENT,
        PLAYER_EXCESS_POWER_COMPARE_VALUE,
        SKIRMISH_SPECIAL_POWER_READY,
        SKIRMISH_VALUE_IN_AREA,
        SKIRMISH_PLAYER_FACTION,
        SKIRMISH_SUPPLIES_VALUE_WITHIN_DISTANCE,
        SKIRMISH_TECH_BUILDING_WITHIN_DISTANCE,
        SKIRMISH_COMMAND_BUTTON_READY_ALL,
        SKIRMISH_COMMAND_BUTTON_READY_PARTIAL,
        SKIRMISH_UNOWNED_FACTION_UNIT_EXISTS,
        SKIRMISH_PLAYER_HAS_PREREQUISITE_TO_BUILD,
        SKIRMISH_PLAYER_HAS_COMPARISON_GARRISONED,
        SKIRMISH_PLAYER_HAS_COMPARISON_CAPTURED_UNITS,
        SKIRMISH_NAMED_AREA_EXIST,
        SKIRMISH_PLAYER_HAS_UNITS_IN_AREA,
        SKIRMISH_PLAYER_HAS_BEEN_ATTACKED_BY_PLAYER,
        SKIRMISH_PLAYER_IS_OUTSIDE_AREA,
        SKIRMISH_PLAYER_HAS_DISCOVERED_PLAYER,
        PLAYER_ACQUIRED_SCIENCE,
        PLAYER_HAS_SCIENCEPURCHASEPOINTS,
        PLAYER_CAN_PURCHASE_SCIENCE,
        MUSIC_TRACK_HAS_COMPLETED,
        PLAYER_LOST_OBJECT_TYPE,
        SUPPLY_SOURCE_SAFE,
        SUPPLY_SOURCE_ATTACKED,
        START_POSITION_IS,
        NAMED_HAS_FREE_CONTAINER_SLOTS,

        CONDITION_COUNT,
    };

protected:
    virtual ~Condition() override;

public:
    Condition();
    Condition(ConditionType type);

    Condition *Duplicate();
    Condition *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    void Set_Condition_Type(ConditionType type);
    int Get_UI_Strings(Utf8String *const strings);
    Utf8String Get_UI_Text();

    ConditionType Get_Condition_Type() const { return m_conditionType; }
    int Get_Num_Parameters() const { return m_numParams; }
    Condition *Get_Next() const { return m_nextAndCondition; }
    int Get_Custom_Data() const { return m_customData; }
    int Get_Frame() const { return m_frame; }

    Parameter *Get_Parameter(int ndx) const
    {
        if (ndx < 0 || ndx >= m_numParams) {
            return nullptr;
        } else {
            return m_params[ndx];
        }
    }

    void Set_Warnings(bool set) { m_hasWarnings = set; }
    void Set_Next_Condition(Condition *condition) { m_nextAndCondition = condition; }
    void Set_Custom_Data(int data) { m_customData = data; }
    void Set_Frame(int frame) { m_frame = frame; }

    static bool Parse_Condition_Data_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_Condition_Data_Chunk(DataChunkOutput &output, Condition *condition);

private:
    ConditionType m_conditionType;
    int m_numParams;
    Parameter *m_params[MAX_CONDITION_PARAMETERS];
    Condition *m_nextAndCondition;
    int m_hasWarnings;
    int m_customData;
    int m_frame;
};

class OrCondition : public MemoryPoolObject
{
    IMPLEMENT_POOL(OrCondition);

protected:
    virtual ~OrCondition() override;

public:
    OrCondition() : m_nextOr(nullptr), m_firstAnd(nullptr) {}

    OrCondition *Duplicate();
    OrCondition *Duplicate_And_Qualify(const Utf8String &str1, const Utf8String &str2, const Utf8String &str3);
    Condition *Remove_Condition(Condition *condition);
    void Delete_Condition(Condition *condition);
    Condition *Find_Previous_Condition(Condition *condition);

    OrCondition *Get_Next_Or_Condition() const { return m_nextOr; }
    Condition *Get_First_And_Condition() const { return m_firstAnd; }

    void Set_Next_Or_Condition(OrCondition *condition) { m_nextOr = condition; }
    void Set_First_And_Condition(Condition *condition) { m_firstAnd = condition; }

    static bool Parse_OrCondition_Chunk(DataChunkInput &input, DataChunkInfo *info, void *data);
    static void Write_OrCondition_Chunk(DataChunkOutput &output, OrCondition *condition);

private:
    OrCondition *m_nextOr;
    Condition *m_firstAnd;
};

class ScriptConditionsInterface : public SubsystemInterface
{
public:
    ScriptConditionsInterface();
    virtual ~ScriptConditionsInterface() override;
    virtual bool Evaluate_Condition(Condition *condition) = 0;
    virtual bool Evaluate_Skirmish_Command_Button_Is_Ready(
        Parameter *param1, Parameter *param2, Parameter *param3, bool b) = 0;
    virtual bool Evaluate_Team_Is_Contained(Parameter *param, bool b) = 0;
};

class ScriptConditions : public ScriptConditionsInterface
{
public:
    ScriptConditions();
    virtual ~ScriptConditions() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override;
    virtual bool Evaluate_Condition(Condition *condition) override;
    virtual bool Evaluate_Skirmish_Command_Button_Is_Ready(
        Parameter *param1, Parameter *param2, Parameter *param3, bool b) override;
    virtual bool Evaluate_Team_Is_Contained(Parameter *param, bool b) override;

    Player *Player_From_Param(Parameter *param);
    void Object_Types_From_Param(Parameter *param, ObjectTypes *types);
    bool Evaluate_All_Destroyed(Parameter *param);
    bool Evaluate_All_Build_Facilities_Destroyed(Parameter *param);
    bool Evaluate_Is_Destroyed(Parameter *param);
    bool Evaluate_Bridge_Broken(Parameter *param);
    bool Evaluate_Bridge_Repaired(Parameter *param);
    bool Evaluate_Named_Unit_Destroyed(Parameter *param);
    bool Evaluate_Named_Unit_Exists(Parameter *param);
    bool Evaluate_Named_Unit_Dying(Parameter *param);
    bool Evaluate_Named_Unit_Totally_Dead(Parameter *param);
    bool Evaluate_Has_Units(Parameter *param);
    bool Evaluate_Team_Inside_Area_Partially(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Named_Inside_Area(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Has_Unit_Type_In_Area(
        Condition *condition, Parameter *param1, Parameter *param2, Parameter *, Parameter *param3, Parameter *param4);
    bool Evaluate_Player_Has_Unit_Kind_In_Area(
        Condition *condition, Parameter *param1, Parameter *param2, Parameter *, Parameter *param3, Parameter *param4);
    bool Evaluate_Team_State_Is(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_State_Is_Not(Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Outside_Area(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Inside_Area_Entirely(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Team_Outside_Area_Entirely(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Named_Attacked_By_Type(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Attacked_By_Type(Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Attacked_By_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Attacked_By_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Built_By_Player(Condition *condition, Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Created(Parameter *param);
    bool Evaluate_Team_Created(Parameter *param);
    bool Evaluate_Unit_Health(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Player_Has_Credits(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Building_Entered(Parameter *param1, Parameter *param2);
    bool Evaluate_Is_Building_Empty(Parameter *param);
    bool Evaluate_Enemy_Sighted(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Type_Sighted(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Named_Discovered(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Discovered(Parameter *param1, Parameter *param2);
    bool Evaluate_Mission_Attempts(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Named_Owned_By_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Owned_By_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Has_N_Or_Fewer_Buildings(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Has_N_Or_Fewer_Faction_Buildings(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Has_Power(Parameter *param);
    bool Evaluate_Named_Reached_Waypoints_End(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Reached_Waypoints_End(Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Selected(Condition *condition, Parameter *parameter);
    bool Evaluate_Video_Has_Completed(Parameter *param);
    bool Evaluate_Speech_Has_Completed(Parameter *param);
    bool Evaluate_Audio_Has_Completed(Parameter *param);
    bool Evaluate_Player_Special_Power_From_Unit_Triggered(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Player_Special_Power_From_Unit_Midway(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Player_Special_Power_From_Unit_Complete(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Upgrade_From_Unit_Complete(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Science_Acquired(Parameter *param1, Parameter *param2);
    bool Evaluate_Can_Purchase_Science(Parameter *param1, Parameter *param2);
    bool Evaluate_Science_Purchase_Points(Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Has_Free_Container_Slots(Parameter *param);
    bool Evaluate_Named_Entered_Area(Parameter *param1, Parameter *param2);
    bool Evaluate_Named_Exited_Area(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Entered_Area_Entirely(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Team_Entered_Area_Partially(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Team_Exited_Area_Entirely(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Team_Exited_Area_Partially(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Multiplayer_Allied_Victory();
    bool Evaluate_Multiplayer_Allied_Defeat();
    bool Evaluate_Multiplayer_Player_Defeat();
    bool Evaluate_Player_Unit_Condition(
        Condition *condition, Parameter *param1, Parameter *param2, Parameter *param3, Parameter *param4);
    bool Evaluate_Player_Has_Comparison_Percent_Power(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Player_Has_Comparison_Value_Excess_Power(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Skirmish_Special_Power_Is_Ready(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Destroyed_N_Or_More_Buildings(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Unit_Has_Emptied(Parameter *param);
    bool Evaluate_Unit_Has_Object_Status(Parameter *param1, Parameter *param2);
    bool Evaluate_Team_Has_Object_Status(Parameter *param1, Parameter *param3, bool b);
    bool Evaluate_Skirmish_Value_In_Area(
        Condition *condition, Parameter *param1, Parameter *param2, Parameter *param3, Parameter *param4);
    bool Evaluate_Skirmish_Player_Is_Faction(Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Supplies_Within_Distance_Perimeter(
        Parameter *param1, Parameter *param2, Parameter *param3, Parameter *param4);
    bool Evaluate_Skirmish_Player_Tech_Building_Within_Distance_Perimeter(
        Condition *condition, Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Skirmish_Unowned_Faction_Unit_Comparison(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Skirmish_Player_Has_Prereqs_To_Build(Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Player_Has_Comparison_Garrisoned(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Skirmish_Player_Has_Comparison_Captured_Units(Parameter *param1, Parameter *param2, Parameter *param3);
    bool Evaluate_Skirmish_Named_Area_Exists(Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Player_Has_Units_In_Area(Condition *condition, Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Supply_Source_Safe(Condition *condition, Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Supply_Source_Attacked(Parameter *param);
    bool Evaluate_Skirmish_Start_Position(Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Player_Has_Been_Attacked_By_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Player_Is_Outside_Area(Condition *condition, Parameter *param1, Parameter *param2);
    bool Evaluate_Skirmish_Player_Has_Discovered_Player(Parameter *param1, Parameter *param2);
    bool Evaluate_Music_Has_Completed(Parameter *param1, Parameter *param2);
    bool Evaluate_Player_Lost_Object_Type(Parameter *param1, Parameter *param2);
};
