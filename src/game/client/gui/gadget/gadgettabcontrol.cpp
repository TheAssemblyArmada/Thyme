/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Tab Control
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgettabcontrol.h"

void Gadget_Tab_Control_Fixup_Sub_Pane_List(GameWindow *tab_control)
{
    int i = 0;
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());
    GameWindow *child = tab_control->Win_Get_Child();

    if (child != nullptr) {
        for (GameWindow *next = child->Win_Get_Next(); next != nullptr; next = child->Win_Get_Next()) {
            child = child->Win_Get_Next();
        }

        while (child != nullptr) {
            tc_data->m_subPanes[i++] = child;
            child = child->Win_Get_Prev();
        }
    }
}

WindowMsgHandledType Gadget_Tab_Control_Input(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D7F60, 0x00A5A620), tab_control, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Tab_Control_System(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x006D8070, 0x00A5A72F), tab_control, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

void Gadget_Tab_Control_Compute_Tab_Region(GameWindow *tab_control)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *>(PICK_ADDRESS(0x006D8160, 0x00A5A7E4), tab_control);
#endif
}

void Gadget_Tab_Control_Show_Sub_Pane(GameWindow *tab_control, int which_pane)
{
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());

    for (int i = 0; i < NUM_TAB_PANES; i++) {
        if (tc_data->m_subPanes[i] != nullptr) {
            tc_data->m_subPanes[i]->Win_Hide(true);
        }
    }

    if (tc_data->m_subPanes[which_pane] != nullptr) {
        tc_data->m_activeTab = which_pane;
    } else {
        tc_data->m_activeTab = 0;
    }

    tc_data->m_activeTab = GameMath::Min(tc_data->m_activeTab, tc_data->m_tabCount - 1);
    tc_data->m_subPanes[tc_data->m_activeTab]->Win_Hide(false);
}

void Gadget_Tab_Control_Create_Sub_Panes(GameWindow *tab_control)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *>(PICK_ADDRESS(0x006D83F0, 0x00A5ABFC), tab_control);
#endif
}
