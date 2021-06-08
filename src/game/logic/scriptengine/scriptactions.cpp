/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Script engine action processor.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptactions.h"
#include "gamelogic.h"
#include "object.h"
#include "scriptengine.h"

#ifndef GAME_DLL
ScriptActionsInterface *g_theScriptActions;
#endif

ScriptActions::ScriptActions() : m_unkBool(false), m_unkString(Utf8String::s_emptyString) {}

ScriptActions::~ScriptActions()
{
    Reset();
}

void ScriptActions::Init()
{
    Reset();
}

void ScriptActions::Reset()
{
    m_unkBool = false;
    Close_Windows(false);
}

void ScriptActions::Execute_Action(ScriptAction *action)
{
    // TODO Requires action handlers implementing for all script actions.
#ifdef GAME_DLL
    Call_Method<int, ScriptActions, ScriptAction *>(PICK_ADDRESS(0x00532590, 0x008C3134), this, action);
#endif
}

int ScriptActions::Close_Windows(bool unk)
{
    // TODO Requires WindowManager vtable implementing.
#ifdef GAME_DLL
    return Call_Method<int, ScriptActions, bool>(PICK_ADDRESS(0x00528E50, 0x008B7632), this, unk);
#else
    return 0;
#endif
}

void ScriptActions::Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool bonus)
{
    for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        obj->Set_Receiving_Difficulty_Bonus(bonus);
    }

    g_theScriptEngine->Set_Use_Object_Difficulty_Bonus(bonus);
}
