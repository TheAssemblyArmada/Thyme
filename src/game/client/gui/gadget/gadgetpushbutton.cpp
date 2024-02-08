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
#include "gadgetpushbutton.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "gamewindowmanager.h"
#include "keyboard.h"

bool Button_Triggers_On_Mouse_Down(GameWindow *push_button)
{
    if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
        return true;
    }

    return (push_button->Win_Get_Status() & WIN_STATUS_ON_MOUSE_DOWN) != 0;
}

WindowMsgHandledType Gadget_Push_Button_Input(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    WinInstanceData *instance_data = push_button->Win_Get_Instance_Data();

    switch (message) {
        case GWM_LEFT_DOWN: {
            _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());
            AudioEventRTS click;

            if (data != nullptr && data->m_altSound.Is_Not_Empty()) {
                click.Set_Event_Name(data->m_altSound);
            } else {
                click.Set_Event_Name("GUIClick");
            }

            if (g_theAudio != nullptr) {
                g_theAudio->Add_Audio_Event(&click);
            }

            if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                if ((instance_data->m_state & 4) != 0) {
                    instance_data->m_state &= ~4;
                } else {
                    instance_data->m_state |= 4;
                }
            } else {
                instance_data->m_state |= 4;
            }

            if (Button_Triggers_On_Mouse_Down(push_button)) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            }

            return MSG_HANDLED;
        }
        case GWM_LEFT_UP: {
            if ((instance_data->Get_State() & 4) == 0 || (push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                return MSG_IGNORED;
            }

            if (!Button_Triggers_On_Mouse_Down(push_button)) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            }

            instance_data->m_state &= ~4;
            return MSG_HANDLED;
        }
        case GWM_LEFT_DRAG:
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                instance_data->Get_Owner(), GGM_LEFT_DRAG, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            return MSG_HANDLED;
        case GWM_RIGHT_DOWN: {
            _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());
            AudioEventRTS click;

            if (data != nullptr && data->m_altSound.Is_Not_Empty()) {
                click.Set_Event_Name(data->m_altSound);
            } else {
                click.Set_Event_Name("GUIClick");
            }

            if ((instance_data->Get_Status() & WIN_STATUS_RIGHT_CLICK) == 0) {
                return MSG_IGNORED;
            }

            if (g_theAudio != nullptr) {
                g_theAudio->Add_Audio_Event(&click);
            }

            if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                if ((instance_data->m_state & 4) != 0) {
                    instance_data->m_state &= ~4;
                } else {
                    instance_data->m_state |= 4;
                }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_SELECTED_RIGHT, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            } else {
                instance_data->m_state |= 4;
            }

            return MSG_HANDLED;
        }
        case GWM_RIGHT_UP: {
            if ((instance_data->Get_Status() & WIN_STATUS_RIGHT_CLICK) == 0) {
                return MSG_IGNORED;
            }

            if ((instance_data->Get_State() & 4) == 0 || (push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                return MSG_IGNORED;
            }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                instance_data->Get_Owner(), GBM_SELECTED_RIGHT, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            instance_data->m_state &= ~4;
            return MSG_HANDLED;
        }
        case GWM_MOUSE_ENTERING: {
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data->m_state |= 2;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_MOUSE_ENTERING, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            }

            if (push_button->Win_Get_Parent() != nullptr) {
                if ((push_button->Win_Get_Parent()->Win_Get_Style() & GWS_HORZ_SLIDER) != 0) {
                    push_button->Win_Get_Parent()->Win_Get_Instance_Data()->m_state |= 2;
                }
            }

            return MSG_HANDLED;
        }
        case GWM_MOUSE_LEAVING: {
            if ((instance_data->Get_Style() & GWS_MOUSE_TRACK) != 0) {
                instance_data->m_state &= ~2;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                g_theWindowManager->Win_Send_System_Msg(
                    instance_data->Get_Owner(), GBM_MOUSE_LEAVING, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
            }

            if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) == 0 && (instance_data->Get_State() & 4) != 0) {
                instance_data->m_state &= ~4;
            }

            if (push_button->Win_Get_Parent() != nullptr) {
                if ((push_button->Win_Get_Parent()->Win_Get_Style() & GWS_HORZ_SLIDER) != 0) {
                    push_button->Win_Get_Parent()->Win_Get_Instance_Data()->m_state &= ~2;
                }
            }

            return MSG_HANDLED;
        }
        case GWM_CHAR: {
            switch (data_1) {
                case Keyboard::KEY_TAB:
                case Keyboard::KEY_RIGHT:
                case Keyboard::KEY_DOWN:
                    if ((data_2 & 2) != 0) {
                        g_theWindowManager->Win_Next_Tab(push_button);
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_RETURN:
                case Keyboard::KEY_SPACE:
                    if ((data_2 & 1) != 0) {
                        if ((instance_data->Get_State() & 4) != 0
                            && (push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) == 0) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                            g_theWindowManager->Win_Send_System_Msg(
                                instance_data->Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(push_button), 0);
#endif
                            instance_data->m_state &= ~4;
                        }
                    } else if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                        if ((instance_data->m_state & 4) != 0) {
                            instance_data->m_state &= ~4;
                        } else {
                            instance_data->m_state |= 4;
                        }

#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
                        g_theWindowManager->Win_Send_System_Msg(
                            instance_data->Get_Owner(), GBM_SELECTED, reinterpret_cast<unsigned int>(push_button), data_1);
#endif
                    } else {
                        instance_data->m_state |= 4;
                    }

                    return MSG_HANDLED;
                case Keyboard::KEY_UP:
                case Keyboard::KEY_LEFT:
                    if ((data_2 & Keyboard::KEY_STATE_DOWN) != 0) {
                        g_theWindowManager->Win_Prev_Tab(push_button);
                    }

                    return MSG_HANDLED;
                default:
                    return MSG_IGNORED;
            }
        }
        default:
            return MSG_IGNORED;
    }
}

WindowMsgHandledType Gadget_Push_Button_System(
    GameWindow *push_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    WinInstanceData *instance_data = push_button->Win_Get_Instance_Data();

    if (message > GWM_INPUT_FOCUS) {
        if (message != GGM_SET_LABEL) {
            return MSG_IGNORED;
        }

        push_button->Win_Set_Text(*reinterpret_cast<Utf16String *>(data_1));
    } else if (message == GWM_INPUT_FOCUS) {
        if (data_1 != 0) {
            instance_data->m_state |= 2;
        } else {
            instance_data->m_state &= ~2;
        }

        g_theWindowManager->Win_Send_System_Msg(
            instance_data->Get_Owner(), GGM_FOCUS_CHANGE, data_1, push_button->Win_Get_Window_Id());

        if (data_1 != 0) {
            *reinterpret_cast<bool *>(data_2) = true;
        } else {
            *reinterpret_cast<bool *>(data_2) = false;
        }
    } else if (message != GWM_CREATE) {
        if (message != GWM_DESTROY) {
            return MSG_IGNORED;
        }

        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data != nullptr) {
            delete data;
        }

        push_button->Win_Set_User_Data(nullptr);
    }

    return MSG_HANDLED;
}

void Gadget_Check_Like_Button_Set_Visual_Check(GameWindow *push_button, bool is_checked)
{
    if (push_button != nullptr) {
        WinInstanceData *instance_data = push_button->Win_Get_Instance_Data();

        if (instance_data != nullptr) {
            if ((push_button->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0) {
                if (is_checked) {
                    instance_data->m_state |= 4;
                } else {
                    instance_data->m_state &= ~4;
                }
            } else {
                captainslog_dbgassert(false, "Gadget_Check_Like_Button_Set_Visual_Check: Window is not 'CHECK-LIKE'");
            }
        }
    }
}

void Gadget_Button_Enable_Check_Like(GameWindow *push_button, bool is_enabled, bool is_checked)
{
    if (push_button != nullptr) {
        WinInstanceData *instance_data = push_button->Win_Get_Instance_Data();

        if (instance_data != nullptr) {
            if (is_enabled) {
                push_button->Win_Set_Status(WIN_STATUS_CHECK_LIKE);
            } else {
                push_button->Win_Clear_Status(WIN_STATUS_CHECK_LIKE);
            }

            if (is_checked) {
                instance_data->m_state |= 4;
            } else {
                instance_data->m_state &= ~4;
            }
        }
    }
}

void Gadget_Button_Set_Text(GameWindow *push_button, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(push_button, GGM_SET_LABEL, reinterpret_cast<unsigned int>(&text), 0);
#endif
}

_PushButtonData *Get_New_Push_Button_Data()
{
    _PushButtonData *data = new _PushButtonData();

    if (data == nullptr) {
        return nullptr;
    }

    data->m_userData = nullptr;
    data->m_drawBorder = false;
    data->m_drawClock = false;
    data->m_overlayImage = nullptr;
    return data;
}

void Gadget_Button_Set_Border(GameWindow *push_button, int color, bool draw_border)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data == nullptr) {
            data = Get_New_Push_Button_Data();
        }

        data->m_drawBorder = draw_border;
        data->m_colorBorder = color;
        push_button->Win_Set_User_Data(data);
    }
}

void Gadget_Button_Draw_Inverse_Clock(GameWindow *push_button, int percent, int color)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data == nullptr) {
            data = Get_New_Push_Button_Data();
        }

        data->m_drawClock = 2;
        data->m_percentClock = percent;
        data->m_colorClock = color;
        push_button->Win_Set_User_Data(data);
    }
}

void Gadget_Button_Draw_Overlay_Image(GameWindow *push_button, const Image *image)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data == nullptr) {
            data = Get_New_Push_Button_Data();
        }

        data->m_overlayImage = image;
        push_button->Win_Set_User_Data(data);
    }
}

void Gadget_Button_Set_Data(GameWindow *push_button, void *user_data)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data == nullptr) {
            data = Get_New_Push_Button_Data();
        }

        data->m_userData = user_data;
        push_button->Win_Set_User_Data(data);
    }
}

void *Gadget_Button_Get_Data(GameWindow *push_button)
{
    if (push_button == nullptr) {
        return nullptr;
    }

    _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

    if (data != nullptr) {
        return data->m_userData;
    } else {
        return nullptr;
    }
}

void Gadget_Button_Set_Alt_Sound(GameWindow *push_button, Utf8String alt_sound)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data != nullptr) {
            data->m_altSound = alt_sound;
        }

        push_button->Win_Set_User_Data(data);
    }
}

bool Gadget_Check_Like_Button_Is_Checked(GameWindow *push_button)
{
    if (push_button == nullptr) {
        return false;
    }

    WinInstanceData *instance_data = push_button->Win_Get_Instance_Data();
    return instance_data != nullptr && (instance_data->m_state & 4) != 0;
}

void Gadget_Button_Draw_Clock(GameWindow *push_button, int percent, int color)
{
    if (push_button != nullptr) {
        _PushButtonData *data = static_cast<_PushButtonData *>(push_button->Win_Get_User_Data());

        if (data == nullptr) {
            data = Get_New_Push_Button_Data();
        }

        data->m_drawClock = 1;
        data->m_percentClock = percent;
        data->m_colorClock = color;
        push_button->Win_Set_User_Data(data);
    }
}
