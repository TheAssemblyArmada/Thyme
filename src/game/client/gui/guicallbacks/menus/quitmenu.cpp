/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Quit Menu
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "quitmenu.h"

#ifdef GAME_DLL
#include "hooker.h"
#endif

void Hide_Quit_Menu()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x005C9B20, 0x00A0F692));
#endif
}

void Toggle_Quit_Menu()
{
#ifdef GAME_DLL
    Call_Function<void>(PICK_ADDRESS(0x005C9C20, 0x00A0F784));
#endif
}
