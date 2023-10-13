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

void Hide_Control_Bar(bool hide)
{
#ifdef GAME_DLL
    Call_Function<void, bool>(PICK_ADDRESS(0x0048A3E0, 0x009DF7F2), hide);
#endif
}

void Show_Control_Bar(bool show)
{
#ifdef GAME_DLL
    Call_Function<void, bool>(PICK_ADDRESS(0x0048A250, 0x009DF683), show);
#endif
}
