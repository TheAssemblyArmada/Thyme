/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Text Entry
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dtextentry.h"

void W3D_Gadget_Text_Entry_Draw(GameWindow *text_entry, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CA650, text_entry, data);
#endif
}

void W3D_Gadget_Text_Entry_Image_Draw(GameWindow *text_entry, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CAC00, text_entry, data);
#endif
}
