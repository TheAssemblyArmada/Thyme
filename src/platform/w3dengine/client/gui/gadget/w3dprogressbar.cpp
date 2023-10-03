/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Progress Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "w3dprogressbar.h"

void W3D_Gadget_Progress_Bar_Draw(GameWindow *progress_bar, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CB2F0, progress_bar, data);
#endif
}

void W3D_Gadget_Progress_Bar_Image_Draw(GameWindow *progress_bar, WinInstanceData *data)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, WinInstanceData *>(0x007CB520, progress_bar, data);
#endif
}
