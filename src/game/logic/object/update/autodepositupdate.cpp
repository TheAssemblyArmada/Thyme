/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Auto Deposit Update
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "autodepositupdate.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

void AutoDepositUpdate::Award_Initial_Capture_Bonus(Player *p)
{
#ifdef GAME_DLL
    Call_Method<void, AutoDepositUpdate, Player *>(PICK_ADDRESS(0x00588950, 0x00971353), this, p);
#endif
}
