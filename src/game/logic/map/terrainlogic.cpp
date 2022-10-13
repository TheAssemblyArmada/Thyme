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
#include "object.h"

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

PathfindLayerEnum TerrainLogic::Get_Highest_Layer_For_Destination(const Coord3D *pos, bool b)
{
#ifdef GAME_DLL
    return Call_Method<PathfindLayerEnum, TerrainLogic, const Coord3D *, bool>(
        PICK_ADDRESS(0x0044B6B0, 0x00744B8E), this, pos, b);
#else
    return LAYER_INVALID;
#endif
}

void TerrainLogic::Get_Bridge_Attack_Points(const Object *bridge, TBridgeAttackInfo *attack_info)
{
    ObjectID id = bridge->Get_ID();

    for (Bridge *b = Get_First_Bridge(); b != nullptr; b = b->Get_Next()) {
        const BridgeInfo *info = b->Peek_Bridge_Info();

        if (info->bridge_object_id == id) {
            Coord3D distance = info->to - info->from;
            distance.Normalize();
            float f = (info->from_right - info->from_left).Length() / 2.0f;
            attack_info->m_attackPoint1 = info->from + distance * f;
            attack_info->m_attackPoint2 = info->to - distance * f;
            return;
        }
    }

    attack_info->m_attackPoint1 = *bridge->Get_Position();
    attack_info->m_attackPoint2 = *bridge->Get_Position();
}
