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
