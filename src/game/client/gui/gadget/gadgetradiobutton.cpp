/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Radio Button
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgetradiobutton.h"
#include "gamewindowmanager.h"

void Do_Radio_Unselect(GameWindow *radio_button, int group, int screen, GameWindow *except)
{
    if (radio_button != except && (radio_button->Win_Get_Style() & GWS_RADIO_BUTTON) != 0) {
        _RadioButtonData *data = static_cast<_RadioButtonData *>(radio_button->Win_Get_User_Data());

        if (data->m_group == group && data->m_screen == screen) {
            radio_button->Win_Get_Instance_Data()->m_state &= ~4;
        }
    }

    for (GameWindow *child = radio_button->Win_Get_Child(); child != nullptr; child = child->Win_Get_Next()) {
        Do_Radio_Unselect(child, group, screen, except);
    }
}

void Unselect_Other_Radio_Of_Group(int group, int screen, GameWindow *except)
{
    for (GameWindow *window = g_theWindowManager->Win_Get_Window_List(); window != nullptr;
         window = window->Win_Get_Next()) {
        Do_Radio_Unselect(window, group, screen, except);
    }
}

WindowMsgHandledType Gadget_Radio_Button_Input(
    GameWindow *radio_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D8610, 0x00A59F40), radio_button, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Radio_Button_System(
    GameWindow *radio_button, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D8AA0, 0x00A5A3CD), radio_button, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

void Gadget_Radio_Set_Text(GameWindow *radio_button, Utf16String text)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(radio_button, GGM_SET_LABEL, reinterpret_cast<unsigned int>(&text), 0);
#endif
}

void Gadget_Radio_Set_Group(GameWindow *radio_button, int group, int screen)
{
    _RadioButtonData *data = static_cast<_RadioButtonData *>(radio_button->Win_Get_User_Data());
    data->m_group = group;
    data->m_screen = screen;
}

void Gadget_Radio_Set_Selection(GameWindow *radio_button, bool send_msg)
{
    if (radio_button != nullptr) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        g_theWindowManager->Win_Send_System_Msg(
            radio_button, GBM_SET_SELECTION, reinterpret_cast<unsigned int>(&send_msg), 0);
#endif
    }
}
