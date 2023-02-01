/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Victory Conditions
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "victoryconditions.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
VictoryConditionsInterface *g_theVictoryConditions;
#endif

VictoryConditionsInterface *Create_Victory_Conditions()
{
#ifdef GAME_DLL
    return Call_Function<VictoryConditionsInterface *>(PICK_ADDRESS(0x00498FF0, 0x009E12C0));
#else
    return nullptr;
#endif
}
