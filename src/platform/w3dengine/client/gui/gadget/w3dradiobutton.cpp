/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radio Button
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dradiobutton.h"

void W3D_Gadget_Radio_Button_Draw(GameWindow *radio_button, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CD880, radio_button, data);
#endif
}

void W3D_Gadget_Radio_Button_Image_Draw(GameWindow *radio_button, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CDBB0, radio_button, data);
#endif
}
