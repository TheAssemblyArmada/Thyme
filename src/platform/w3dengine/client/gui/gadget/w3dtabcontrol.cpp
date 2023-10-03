/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Tab Control
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtabcontrol.h"

void W3D_Gadget_Tab_Control_Draw(GameWindow *tab_control, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CCD90, tab_control, data);
#endif
}

void W3D_Gadget_Tab_Control_Image_Draw(GameWindow *tab_control, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CD4A0, tab_control, data);
#endif
}
