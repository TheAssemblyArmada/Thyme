/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "controlbar.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
ControlBar *g_theControlBar;
#endif

void ControlBar::Mark_UI_Dirty()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x0045B3F0, 0x00729C50), this);
#endif
}

void ControlBar::Set_Control_Bar_Scheme_By_Player(Player *player)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, Player *>(PICK_ADDRESS(0x00460340, 0x0072F421), this, player);
#endif
}

void ControlBar::Set_Control_Bar_Scheme_By_Player_Template(PlayerTemplate *tmplate)
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar, PlayerTemplate *>(PICK_ADDRESS(0x004606B0, 0x0072F642), this, tmplate);
#endif
}
