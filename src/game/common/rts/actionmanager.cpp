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
#include "behaviormodule.h"
#include "object.h"
#include "player.h"
#include "specialpower.h"
#include "terrainlogic.h"
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

bool ActionManager::Can_Do_Special_Power(
    const Object *obj, const SpecialPowerTemplate *sp_template, CommandSourceType source, unsigned int i, bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const SpecialPowerTemplate *,
        CommandSourceType,
        unsigned int,
        bool>(PICK_ADDRESS(0x00497A70, 0x008E202D), this, obj, sp_template, source, i, b);
#else
    return false;
#endif
}

bool ActionManager::Can_Do_Special_Power_At_Location(const Object *obj,
    const Coord3D *loc,
    CommandSourceType source,
    const SpecialPowerTemplate *sp_template,
    const Object *object_in_way,
    unsigned int i,
    bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const Coord3D *,
        CommandSourceType,
        const SpecialPowerTemplate *,
        const Object *,
        unsigned int,
        bool>(PICK_ADDRESS(0x004972F0, 0x008E1907), this, obj, loc, source, sp_template, object_in_way, i, b);
#else
    return false;
#endif
}

bool ActionManager::Can_Do_Special_Power_At_Object(const Object *obj,
    const Object *target,
    CommandSourceType source,
    const SpecialPowerTemplate *sp_template,
    unsigned int i,
    bool b)
{
#ifdef GAME_DLL
    return Call_Method<bool,
        ActionManager,
        const Object *,
        const Object *,
        CommandSourceType,
        const SpecialPowerTemplate *,
        unsigned int,
        bool>(PICK_ADDRESS(0x00497530, 0x008E1AEB), this, obj, target, source, sp_template, i, b);
#else
    return false;
#endif
}
