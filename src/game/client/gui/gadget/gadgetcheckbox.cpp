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
#include "keyboard.h"

WindowMsgHandledType Gadget_Check_Box_Input(
    GameWindow *check_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    WinInstanceData *instance_data = check_box->Win_Get_Instance_Data();

    switch (message) {
        case GWM_LEFT_DOWN:
        case GWM_RIGHT_DOWN:
            return MSG_HANDLED;
        case GWM_LEFT_UP:
            if ((instance_data->Get_State() & 2) == 0) {
                return MSG_IGNORED;
            }

            instance_data->m_state ^= 4;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                check_box->Win_Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(check_box), data_1);
#endif
            return MSG_HANDLED;
        case GWM_LEFT_DRAG:
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                check_box->Win_Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(check_box), data_1);
#endif
            return MSG_HANDLED;
        case GWM_RIGHT_UP:
            if ((instance_data->Get_State() & 4) == 0) {
                return MSG_IGNORED;
            }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                check_box->Win_Get_Owner(), GBM_SELECTED_RIGHT, reinterpret_cast<unsigned int>(check_box), data_1);
#endif
            instance_data->m_state &= ~4;
            return MSG_HANDLED;
        case GWM_MOUSE_ENTERING:
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data->m_state |= 2;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    check_box->Win_Get_Owner(), GBM_MOUSE_ENTERING, reinterpret_cast<unsigned int>(check_box), data_1);
#endif
            }

            return MSG_HANDLED;
        case GWM_MOUSE_LEAVING:
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data->m_state &= ~2;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    check_box->Win_Get_Owner(), GBM_MOUSE_LEAVING, reinterpret_cast<unsigned int>(check_box), data_1);
#endif
            }

            return MSG_HANDLED;
        case GWM_CHAR:
            switch (data_1) {
                case Keyboard::KEY_TAB:
                case Keyboard::KEY_RIGHT:
                case Keyboard::KEY_DOWN:
                    if ((data_2 & 2) != 0) {
                        g_theWindowManager->Win_Next_Tab(check_box);
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_RETURN:
                case Keyboard::KEY_SPACE:
                    if ((data_2 & 2) != 0) {
                        instance_data->m_state ^= 4;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                        g_theWindowManager->Win_Send_System_Msg(
                            check_box->Win_Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(check_box), 0);
#endif
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_UP:
                case Keyboard::KEY_LEFT:
                    if ((data_2 & 2) != 0) {
                        g_theWindowManager->Win_Prev_Tab(check_box);
                    }

                    return MSG_HANDLED;
                default:
                    return MSG_IGNORED;
            }
        default:
            return MSG_IGNORED;
    }
}

WindowMsgHandledType Gadget_Check_Box_System(
    GameWindow *check_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    WinInstanceData *instance_data = check_box->Win_Get_Instance_Data();

    if (message > GWM_INPUT_FOCUS) {
        if (message != GGM_SET_LABEL) {
            return MSG_IGNORED;
        }

        check_box->Win_Set_Text(*reinterpret_cast<Utf16String *>(data_1));
    } else if (message == GWM_INPUT_FOCUS) {
        if (data_1 != 0) {
            instance_data->m_state |= 2;
        } else {
            instance_data->m_state &= ~2;
        }

        g_theWindowManager->Win_Send_System_Msg(
            instance_data->Get_Owner(), GGM_FOCUS_CHANGE, data_1, check_box->Win_Get_Window_Id());

        if (data_1 != 0) {
            *reinterpret_cast<bool *>(data_2) = true;
        } else {
            *reinterpret_cast<bool *>(data_2) = false;
        }
    } else if (message != GWM_CREATE && message != GWM_DESTROY) {
        return MSG_IGNORED;
    }

    return MSG_HANDLED;
}

void Gadget_Check_Box_Set_Text(GameWindow *check_box, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(check_box, GGM_SET_LABEL, reinterpret_cast<unsigned int>(&text), 0);
#endif
}

void Gadget_Check_Box_Set_Checked(GameWindow *check_box, bool is_checked)
{
    WinInstanceData *instance_data = check_box->Win_Get_Instance_Data();

    if (is_checked) {
        instance_data->m_state |= 4;
    } else {
        instance_data->m_state &= ~4;
    }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(
        check_box->Win_Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(check_box), 0);
#endif
}

void Gadget_Check_Box_Toggle(GameWindow *check_box)
{
    WinInstanceData *instance_data = check_box->Win_Get_Instance_Data();

    if ((instance_data->m_state & 4) != 0) {
        instance_data->m_state &= ~4;
    } else {
        instance_data->m_state |= 4;
    }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(
        check_box->Win_Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(check_box), 0);
#endif
}

bool Gadget_Check_Box_Is_Checked(GameWindow *check_box)
{
    return (check_box->Win_Get_Instance_Data()->m_state & 4) != 0;
}
