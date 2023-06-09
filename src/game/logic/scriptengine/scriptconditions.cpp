/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine condition processor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptconditions.h"

#ifndef GAME_DLL
ScriptConditionsInterface *g_theScriptConditions;
TransportStatus *ScriptConditions::s_transportStatuses;
#endif

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
    Parameter *param1, Parameter *param2, Parameter *param3, bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptConditions, Parameter *, Parameter *, Parameter *, bool>(
        PICK_ADDRESS(0x00526180, 0x008B47C8), this, param1, param2, param3, b);
#else
    return false;
#endif
}

bool ScriptConditions::Evaluate_Team_Is_Contained(Parameter *param, bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptConditions, Parameter *, bool>(PICK_ADDRESS(0x00525660, 0x008B3C43), this, param, b);
#else
    return false;
#endif
}
