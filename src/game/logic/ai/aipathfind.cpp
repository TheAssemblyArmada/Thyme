/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Pathfinding
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aipathfind.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

bool Pathfinder::Valid_Movement_Terrain(PathfindLayerEnum layer, const Locomotor *locomotor, const Coord3D *pos)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, PathfindLayerEnum, const Locomotor *, const Coord3D *>(
        PICK_ADDRESS(0x00561100, 0x00892877), this, layer, locomotor, pos);
#else
    return false;
#endif
}

bool Pathfinder::Adjust_Target_Destination(const Object *source_obj,
    const Object *target_obj,
    const Coord3D *target_pos,
    const Weapon *weapon,
    Coord3D *destination_pos)
{
#ifdef GAME_DLL
    return Call_Method<bool, Pathfinder, const Object *, const Object *, const Coord3D *, const Weapon *, Coord3D *>(
        PICK_ADDRESS(0x00562EB0, 0x00893CA1), this, source_obj, target_obj, target_pos, weapon, destination_pos);
#else
    return false;
#endif
}
