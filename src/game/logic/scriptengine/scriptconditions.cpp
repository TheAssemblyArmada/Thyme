#include "scriptconditions.h"

#ifndef GAME_DLL
ScriptConditionsInterface *g_theScriptConditions;
TransportStatus *ScriptConditions::s_transportStatuses;
#endif

ScriptConditions::ScriptConditions() {}

ScriptConditions::~ScriptConditions()
{
    Reset();
}

void ScriptConditions::Init()
{
    Reset();
}

void ScriptConditions::Reset()
{
    s_transportStatuses->Delete_Instance();
    s_transportStatuses = nullptr;
}

bool ScriptConditions::Evaluate_Condition(Condition *condition)
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptConditions, Condition *>(PICK_ADDRESS(0x00527570, 0x008B596C), this, condition);
#else
    return false;
#endif
}

bool ScriptConditions::Evaluate_Skirmish_Command_Button_Is_Ready(
    Parameter *param_one, Parameter *param_two, Parameter *param_three, bool unk)
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptConditions, Parameter *, Parameter *, Parameter *, bool>(
        PICK_ADDRESS(0x00526180, 0x008B47C8), this, param_one, param_two, param_three, unk);
#else
    return false;
#endif
}

bool ScriptConditions::Evaluate_Team_Is_Contained(Parameter *param, bool unk)
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptConditions, Parameter *, bool>(PICK_ADDRESS(0x00525660, 0x008B3C43), this, param, unk);
#else
    return false;
#endif
}
