/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief In Game UI
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "ingameui.h"

#ifndef GAME_DLL
InGameUI *g_theInGameUI;
#endif

void InGameUI::Add_World_Animation(
    Anim2DTemplate *anim, const Coord3D *pos, WorldAnimationOptions options, float time, float z_rise)
{
#ifdef GAME_DLL
    Call_Method<void, InGameUI, Anim2DTemplate *, const Coord3D *, WorldAnimationOptions, float, float>(
        PICK_ADDRESS(0x00510A10, 0x00812ED9), this, anim, pos, options, time, z_rise);
#endif
}
