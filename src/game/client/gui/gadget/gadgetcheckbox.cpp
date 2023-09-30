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
#include "gadgetcheckbox.h"
#include "gamewindowmanager.h"

void Gadget_Check_Box_Set_Text(GameWindow *check_box, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(check_box, GGM_SET_LABEL, reinterpret_cast<unsigned int>(&text), 0);
#endif
}
