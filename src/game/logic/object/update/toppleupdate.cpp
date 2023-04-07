/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Topple Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "toppleupdate.h"

bool ToppleUpdate::Is_Able_To_Be_Toppled() const
{
    return m_toppleState == TOPPLE_UPRIGHT;
}

void ToppleUpdate::Apply_Toppling_Force(const Coord3D *topple_direction, float topple_speed, unsigned int options)
{
#ifdef GAME_DLL
    Call_Method<void, ToppleUpdate, const Coord3D *, float, unsigned int>(
        PICK_ADDRESS(0x006730C0, 0x00992FE7), this, topple_direction, topple_speed, options);
#endif
}
