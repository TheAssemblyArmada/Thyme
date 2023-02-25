/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "radar.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
Radar *g_theRadar;
#endif

void Radar::Remove_Object(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Radar, Object *>(PICK_ADDRESS(0x00499F40, 0x0090A0EE), this, obj);
#endif
}

void Radar::Add_Object(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Radar, Object *>(PICK_ADDRESS(0x00499D10, 0x00909D5C), this, obj);
#endif
}
