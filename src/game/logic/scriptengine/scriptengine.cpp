/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Script engine
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "scriptengine.h"

#ifndef GAME_DLL
ScriptEngine *g_theScriptEngine = nullptr;
#endif

bool ScriptEngine::Is_Time_Frozen_Debug()
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptEngine>(PICK_ADDRESS(0x00436FF0, 0x00712D9E), this);
#else
    return false;
#endif
}

bool ScriptEngine::Is_Time_Fast()
{
#ifdef GAME_DLL
    return Call_Method<bool, ScriptEngine>(PICK_ADDRESS(0x00437040, 0x00712E03), this);
#else
    return false;
#endif
}

void ScriptEngine::Notify_Of_Team_Destruction(Team *team_destroyed)
{
#ifdef GAME_DLL
    Call_Method<Team *>(PICK_ADDRESS(0x00436070, 0x00711E87), this, team_destroyed);
#endif
}
