/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ai.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void AIGroup::Add(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, AIGroup, Object *>(PICK_ADDRESS(0x0054FB60, 0x008D27B0), this, obj);
#endif
}

#ifndef GAME_DLL
AI *g_theAI = nullptr;
#endif
