/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Object for tracking player information.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "player.h"

void Player::On_Power_Brown_Out_Change(bool b)
{
#ifdef GAME_DLL
    Call_Method<void, Player, bool>(PICK_ADDRESS(0x00456820, 0x0086048D), this, b);
#endif
}
