/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Push Button
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dpushbutton.h"

void W3D_Gadget_Push_Button_Draw(GameWindow *push_button, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CE230, push_button, data);
#endif
}

void W3D_Gadget_Push_Button_Image_Draw(GameWindow *push_button, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CE600, push_button, data);
#endif
}
