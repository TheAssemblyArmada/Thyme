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
#pragma once
#include "always.h"
#include "gamewindow.h"

enum
{
    NUM_TAB_PANES = 8,
};

struct _TabControlData
{
    int m_tabOrientation;
    int m_tabEdge;
    int m_tabWidth;
    int m_tabHeight;
    int m_tabCount;
    GameWindow *m_subPanes[NUM_TAB_PANES];
    bool m_subPaneDisabled[NUM_TAB_PANES];
    int m_paneBorder;
    int m_activeTab;
    int m_tabsLeftLimit;
    int m_tabsRightLimit;
    int m_tabsTopLimit;
    int m_tabsBottomLimit;
};

WindowMsgHandledType Gadget_Tab_Control_Input(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_Tab_Control_System(
    GameWindow *tab_control, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_Tab_Control_Compute_Tab_Region(GameWindow *tab_control);
void Gadget_Tab_Control_Show_Sub_Pane(GameWindow *tab_control, int which_pane);
void Gadget_Tab_Control_Create_Sub_Panes(GameWindow *tab_control);
void Gadget_Tab_Control_Fixup_Sub_Pane_List(GameWindow *tab_control);
void Gadget_Tab_Control_Compute_Sub_Pane_Size(GameWindow *tab_control, int *width, int *height, int *x, int *y);
void Gadget_Tab_Control_Resize_Sub_Panes(GameWindow *tab_control);
