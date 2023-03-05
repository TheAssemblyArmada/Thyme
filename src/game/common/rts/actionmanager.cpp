/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Action Manager
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "actionmanager.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ActionManager *g_theActionManager;
#endif

bool ActionManager::Can_Enter_Object(
    const Object *obj, const Object *object_to_enter, CommandSourceType source, CanEnterType type)
{
#ifdef GAME_DLL
    return Call_Method<bool, ActionManager, const Object *, const Object *, CommandSourceType, CanEnterType>(
        PICK_ADDRESS(0x00496500, 0x008E06EC), this, obj, object_to_enter, source, type);
#else
    return false;
#endif
}
