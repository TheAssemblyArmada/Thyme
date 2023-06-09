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

ScriptActions::ScriptActions() : m_windowsClosed(false), m_name(Utf8String::s_emptyString) {}

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
    m_windowsClosed = false;
    Close_Windows(false);
}

void ScriptActions::Execute_Action(ScriptAction *action)
{
#ifdef GAME_DLL
    Call_Method<void, ScriptActions, ScriptAction *>(PICK_ADDRESS(0x00532590, 0x008C3134), this, action);
#endif
}

void ScriptActions::Close_Windows(bool close)
{
#ifdef GAME_DLL
    Call_Method<void, ScriptActions, bool>(PICK_ADDRESS(0x00528E50, 0x008B7632), this, close);
#endif
}

void ScriptActions::Do_Enable_Or_Disable_Object_Difficulty_Bonuses(bool enable)
{
    for (Object *obj = g_theGameLogic->Get_First_Object(); obj != nullptr; obj = obj->Get_Next_Object()) {
        obj->Set_Receiving_Difficulty_Bonus(enable);
    }

    g_theScriptEngine->Set_Use_Object_Difficulty_Bonus(enable);
}
