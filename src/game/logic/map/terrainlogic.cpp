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
#include "terrainlogic.h"

#ifndef GAME_DLL
TerrainLogic *g_theTerrainLogic = nullptr;
#endif

BridgeInfo::BridgeInfo() :
    bridge_width(0.0f), bridge_index(0), cur_damage_state(BODY_PRISTINE), is_destroyed(false), bridge_object_id(OBJECT_UNK)
{
    from.Zero();
    to.Zero();
    from_left.Zero();
    from_right.Zero();
    to_left.Zero();
    to_right.Zero();

    for (int i = 0; i < BRIDGE_MAX_TOWERS; i++) {
        tower_object_id[i] = OBJECT_UNK;
    }
}
