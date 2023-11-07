/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Generals Exp Points
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "generalsexppoints.h"
#include "controlbar.h"
#include "ingameui.h"
#include "keyboard.h"

WindowMsgHandledType Generals_Exp_Points_Input(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message == GWM_MOUSE_ENTERING) {
        if (g_theInGameUI != nullptr) {
            g_theInGameUI->Place_Build_Available(nullptr, nullptr);
        }

        return MSG_HANDLED;
    }

    if (message != GWM_CHAR) {
        return MSG_HANDLED;
    }

    if (data_1 == Keyboard::KEY_ESCAPE) {
        g_theControlBar->Hide_Purchase_Science();
    }

    return MSG_HANDLED;
}

WindowMsgHandledType Generals_Exp_Points_System(
    GameWindow *window, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message == GWM_INPUT_FOCUS) {
        if (data_1 == 1) {
            *reinterpret_cast<bool *>(data_2) = false;
        }

        return MSG_HANDLED;
    } else {
        if (message != GGM_FOCUS_CHANGE) {
            if (message != GBM_SELECTED) {
                return MSG_IGNORED;
            }

            int id = reinterpret_cast<GameWindow *>(data_1)->Win_Get_Window_Id();
            static const NameKeyType buttonExitID = g_theNameKeyGenerator->Name_To_Key("GeneralsExpPoints.wnd:ButtonExit");

            if (id == buttonExitID) {
                g_theControlBar->Hide_Purchase_Science();
            } else {
                g_theControlBar->Process_Context_Sensitive_Button_Click(
                    reinterpret_cast<GameWindow *>(data_1), static_cast<GadgetGameMessage>(message));
            }
        }

        return MSG_HANDLED;
    }
}
