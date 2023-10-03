/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Combo Box
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dcombobox.h"

void W3D_Gadget_Combo_Box_Draw(GameWindow *combo_box, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CBEF0, combo_box, data);
#endif
}

void W3D_Gadget_Combo_Box_Image_Draw(GameWindow *combo_box, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CC0C0, combo_box, data);
#endif
}
