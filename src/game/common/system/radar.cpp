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

void Radar::Try_Under_Attack_Event(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Radar, Object *>(PICK_ADDRESS(0x0049AA00, 0x0090AC89), this, obj);
#endif
}

void Radar::Try_Event(RadarEventType event, const Coord3D *pos)
{
#ifdef GAME_DLL
    Call_Method<void, Radar, RadarEventType, const Coord3D *>(PICK_ADDRESS(0x0049AE70, 0x0090B0F5), this, event, pos);
#endif
}

void Radar::Try_Infiltration_Event(Object *obj)
{
#ifdef GAME_DLL
    Call_Method<void, Radar, Object *>(PICK_ADDRESS(0x0049AD70, 0x0090AFF1), this, obj);
#endif
}

bool Radar::Local_Pixel_To_Radar(const ICoord2D *pixel, ICoord2D *radar)
{
#ifdef GAME_DLL
    return Call_Method<bool, Radar, const ICoord2D *, ICoord2D *>(PICK_ADDRESS(0x0049A190, 0x0090A315), this, pixel, radar);
#else
    return false;
#endif
}

bool Radar::Radar_To_World(const ICoord2D *radar, Coord3D *world)
{
#ifdef GAME_DLL
    return Call_Method<bool, Radar, const ICoord2D *, Coord3D *>(PICK_ADDRESS(0x0049A070, 0x0090A22B), this, radar, world);
#else
    return false;
#endif
}

bool Radar::Screen_Pixel_To_World(const ICoord2D *screen, Coord3D *world)
{
#ifdef GAME_DLL
    return Call_Method<bool, Radar, const ICoord2D *, Coord3D *>(PICK_ADDRESS(0x0049A300, 0x0090A47B), this, screen, world);
#else
    return false;
#endif
}
