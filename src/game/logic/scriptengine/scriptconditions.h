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
#include "mempoolobj.h"
#include "subsysteminterface.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

class Condition;
class Parameter;
class Player;
class ObjectTypes;

class TransportStatus : public MemoryPoolObject
{
    IMPLEMENT_POOL(TransportStatus);

public:
    TransportStatus() : m_nextStatus(nullptr), m_objID(0), m_frameNumber(0), m_unitCount(0) {}
    virtual ~TransportStatus() override {}

    TransportStatus *m_nextStatus;
    unsigned int m_objID;
    unsigned int m_frameNumber;
    int m_unitCount;
};

class ScriptConditionsInterface : public SubsystemInterface
{
public:
    ScriptConditionsInterface() {}
    virtual ~ScriptConditionsInterface() override {}
    virtual bool Evaluate_Condition(Condition *condition) = 0;
    virtual bool Evaluate_Skirmish_Command_Button_Is_Ready(
        Parameter *param1, Parameter *param2, Parameter *param3, bool b) = 0;
    virtual bool Evaluate_Team_Is_Contained(Parameter *param, bool b) = 0;
};

class ScriptConditions : public ScriptConditionsInterface
{
public:
#ifdef GAME_DLL
    ScriptConditions *Hook_Ctor() { return new (this) ScriptConditions(); }
    void Hook_Dtor() { ScriptConditions::~ScriptConditions(); }
#endif

    ScriptConditions() {}
    virtual ~ScriptConditions() override;
    virtual void Init() override;
    virtual void Reset() override;
    virtual void Update() override {}
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

private:
#ifdef GAME_DLL
    static TransportStatus *&s_transportStatuses;
#else
    static TransportStatus *s_transportStatuses;
#endif
};

#ifdef GAME_DLL
extern ScriptConditionsInterface *&g_theScriptConditions;
#else
extern ScriptConditionsInterface *g_theScriptConditions;
#endif
