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
#include "gadgetprogressbar.h"
#include "gamewindowmanager.h"

WindowMsgHandledType Gadget_Progress_Bar_System(
    GameWindow *progress_bar, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message != GPM_SET_PROGRESS) {
        return MSG_IGNORED;
    }

    if (data_1 < 101) {
        progress_bar->Win_Set_User_Data(reinterpret_cast<void *>(data_1));
    }

    return MSG_HANDLED;
}

void Gadget_Progress_Bar_Set_Progress(GameWindow *progress_bar, int progress)
{
    if (progress_bar != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(progress_bar, GPM_SET_PROGRESS, progress, 0);
    }
}
