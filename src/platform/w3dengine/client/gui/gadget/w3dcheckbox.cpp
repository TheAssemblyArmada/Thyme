/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Check Box
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "always.h"
#include "w3dcheckbox.h"

void W3D_Gadget_Check_Box_Draw(GameWindow *check_box, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CDDF0, check_box, data);
#endif
}

void W3D_Gadget_Check_Box_Image_Draw(GameWindow *check_box, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CE130, check_box, data);
#endif
}
