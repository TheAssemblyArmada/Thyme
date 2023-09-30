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
