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
#include "gamewindowmanager.h"

WindowMsgHandledType Gadget_Tab_Control_Input(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());
    int x;
    int y;
    tab_control->Win_Get_Screen_Position(&x, &y);
    int xpos = (data_1 & 0x0000FFFF) - x;
    int ypos = ((data_1 & 0xFFFF0000) >> 16) - y;

    if (message == GWM_LEFT_DOWN) {
        if (xpos < tc_data->m_tabsLeftLimit || xpos > tc_data->m_tabsRightLimit || ypos < tc_data->m_tabsTopLimit
            || ypos > tc_data->m_tabsBottomLimit) {
            return MSG_HANDLED;
        }

        int i1;
        int i2;

        if (tc_data->m_tabEdge == 4 || tc_data->m_tabEdge == 5) {
            i1 = ypos - tc_data->m_tabsTopLimit;
            i2 = tc_data->m_tabHeight;
        } else {
            i1 = xpos - tc_data->m_tabsLeftLimit;
            i2 = tc_data->m_tabWidth;
        }

        int which_pane = i1 / i2;

        if (!tc_data->m_subPaneDisabled[which_pane] && which_pane != tc_data->m_activeTab) {
            Gadget_Tab_Control_Show_Sub_Pane(tab_control, which_pane);
        }
    }

    return MSG_IGNORED;
}

WindowMsgHandledType Gadget_Tab_Control_System(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message > GGM_RESIZED) {
        if (message == GBM_SELECTED) {
            GameWindow *parent = tab_control->Win_Get_Parent();

            if (parent != nullptr) {
                return g_theWindowManager->Win_Send_System_Msg(parent, message, data_1, data_2);
            }
        }

        return MSG_IGNORED;
    }
    if (message == GGM_RESIZED) {
        Gadget_Tab_Control_Resize_Sub_Panes(tab_control);
        Gadget_Tab_Control_Compute_Tab_Region(tab_control);
    } else if (message != GWM_CREATE) {
        if (message != GWM_DESTROY) {
            return MSG_IGNORED;
        }

        _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());
        delete tc_data;
    }

    return MSG_HANDLED;
}

void Gadget_Tab_Control_Compute_Tab_Region(GameWindow *tab_control)
{
    int width;
    int height;
    tab_control->Win_Get_Size(&width, &height);
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());
    int i1 = 0;
    int i2 = 0;

    if (tc_data->m_tabEdge == 3 || tc_data->m_tabEdge == 6) {
        if (tc_data->m_tabOrientation != 0) {
            if (tc_data->m_tabOrientation == 2) {
                i1 = width - 2 * tc_data->m_paneBorder - tc_data->m_tabWidth * tc_data->m_tabCount;
            } else if (tc_data->m_tabOrientation == 1) {
                i1 = 0;
            }
        } else {
            i1 = width - 2 * tc_data->m_paneBorder - tc_data->m_tabWidth * tc_data->m_tabCount;
            i1 /= 2;
        }
    } else if (tc_data->m_tabOrientation != 0) {
        if (tc_data->m_tabOrientation == 2) {
            i2 = height - 2 * tc_data->m_paneBorder - tc_data->m_tabHeight * tc_data->m_tabCount;
        } else if (tc_data->m_tabOrientation == 1) {
            i2 = 0;
        }
    } else {
        i2 = height - 2 * tc_data->m_paneBorder - tc_data->m_tabHeight * tc_data->m_tabCount;
        i2 /= 2;
    }

    switch (tc_data->m_tabEdge) {
        case 3:
            tc_data->m_tabsTopLimit = tc_data->m_paneBorder;
            tc_data->m_tabsBottomLimit = tc_data->m_tabHeight + tc_data->m_paneBorder;
            tc_data->m_tabsLeftLimit = i1 + tc_data->m_paneBorder;
            tc_data->m_tabsRightLimit = tc_data->m_tabCount * tc_data->m_tabWidth + i1 + tc_data->m_paneBorder;
            break;
        case 6:
            tc_data->m_tabsTopLimit = height - tc_data->m_paneBorder - tc_data->m_tabHeight;
            tc_data->m_tabsBottomLimit = height - tc_data->m_paneBorder;
            tc_data->m_tabsLeftLimit = i1 + tc_data->m_paneBorder;
            tc_data->m_tabsRightLimit = tc_data->m_tabCount * tc_data->m_tabWidth + i1 + tc_data->m_paneBorder;
            break;
        case 4:
            tc_data->m_tabsLeftLimit = width - tc_data->m_paneBorder - tc_data->m_tabWidth;
            tc_data->m_tabsRightLimit = width - tc_data->m_paneBorder;
            tc_data->m_tabsTopLimit = i2 + tc_data->m_paneBorder;
            tc_data->m_tabsBottomLimit = tc_data->m_tabCount * tc_data->m_tabHeight + i2 + tc_data->m_paneBorder;
            break;
        case 5:
            tc_data->m_tabsLeftLimit = tc_data->m_paneBorder;
            tc_data->m_tabsRightLimit = tc_data->m_tabWidth + tc_data->m_paneBorder;
            tc_data->m_tabsTopLimit = i2 + tc_data->m_paneBorder;
            tc_data->m_tabsBottomLimit = tc_data->m_tabCount * tc_data->m_tabHeight + i2 + tc_data->m_paneBorder;
            break;
    }
}

void Gadget_Tab_Control_Compute_Sub_Pane_Size(GameWindow *tab_control, int *width, int *height, int *x, int *y)
{
    int w;
    int h;
    tab_control->Win_Get_Size(&w, &h);
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());

    if (tc_data->m_tabEdge == 3 || tc_data->m_tabEdge == 6) {
        *height = h - 2 * tc_data->m_paneBorder - tc_data->m_tabHeight;
    } else {
        *height = h - 2 * tc_data->m_paneBorder;
    }

    if (tc_data->m_tabEdge == 5 || tc_data->m_tabEdge == 4) {
        *width = w - 2 * tc_data->m_paneBorder - tc_data->m_tabWidth;
    } else {
        *width = w - 2 * tc_data->m_paneBorder;
    }

    if (tc_data->m_tabEdge == 5) {
        *x = tc_data->m_tabWidth + tc_data->m_paneBorder;
    } else {
        *x = tc_data->m_paneBorder;
    }

    if (tc_data->m_tabEdge == 3) {
        *y = tc_data->m_tabHeight + tc_data->m_paneBorder;
    } else {
        *y = tc_data->m_paneBorder;
    }
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
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());

    int width;
    int height;
    int x;
    int y;
    Gadget_Tab_Control_Compute_Sub_Pane_Size(tab_control, &width, &height, &x, &y);

    for (int i = 0; i < NUM_TAB_PANES; i++) {
        if (tc_data->m_subPanes[i] != nullptr) {
            tc_data->m_subPanes[i]->Win_Set_Size(width, height);
            tc_data->m_subPanes[i]->Win_Set_Position(x, y);
        } else {
            tc_data->m_subPanes[i] = g_theWindowManager->Win_Create(
                tab_control, 0, x, y, width, height, Pass_Selected_Buttons_To_Parent_System, nullptr);
            WinInstanceData *data = tc_data->m_subPanes[i]->Win_Get_Instance_Data();
            data->m_style |= GWS_TAB_PANE;

            char buf[20];
            sprintf(buf, "Pane %d", i);
            data->m_decoratedNameString = buf;
            tc_data->m_subPanes[i]->Win_Enable((tab_control->Win_Get_Status() & WIN_STATUS_ENABLED) != 0);
        }
    }

    Gadget_Tab_Control_Show_Sub_Pane(tab_control, tc_data->m_activeTab);
}

void Gadget_Tab_Control_Resize_Sub_Panes(GameWindow *tab_control)
{
    _TabControlData *tc_data = static_cast<_TabControlData *>(tab_control->Win_Get_User_Data());

    int width;
    int height;
    int x;
    int y;
    Gadget_Tab_Control_Compute_Sub_Pane_Size(tab_control, &width, &height, &x, &y);

    for (int i = 0; i < NUM_TAB_PANES; i++) {
        if (tc_data->m_subPanes[i] != nullptr) {
            tc_data->m_subPanes[i]->Win_Set_Size(width, height);
            tc_data->m_subPanes[i]->Win_Set_Position(x, y);
        }
    }
}

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
