/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief AI States
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "aistates.h"

void AIAttackFireWeaponState::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
}

StateReturnType AIAttackFireWeaponState::On_Enter()
{
#ifdef GAME_DLL
    return Call_Method<StateReturnType, AIAttackFireWeaponState>(PICK_ADDRESS(0x0059C290, 0x00838C61), this);
#else
    return STATE_FAILURE;
#endif
}

void AIAttackFireWeaponState::On_Exit(StateExitType status)
{
#ifdef GAME_DLL
    Call_Method<void, AIAttackFireWeaponState>(PICK_ADDRESS(0x0059C5F0, 0x00838D6F), this);
#endif
}

StateReturnType AIAttackFireWeaponState::Update()
{
#ifdef GAME_DLL
    return Call_Method<StateReturnType, AIAttackFireWeaponState>(PICK_ADDRESS(0x0059C320, 0x00839107), this);
#else
    return STATE_FAILURE;
#endif
}
