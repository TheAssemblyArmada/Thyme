/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief List Box
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
class GameWindow;
class GameFont;

enum
{
    LISTBOX_TEXT = 1,
    LISTBOX_IMAGE = 2,
};

struct _ListEntryCell
{
    int m_cellType;
    int m_textColor;
    void *m_data;
    void *m_userData;
    int m_width;
    int m_height;
};

struct _ListEntryRow
{
    int m_listHeight;
    char m_height;
    _ListEntryCell *m_cell;
};

struct _ListboxData
{
    short m_listLength;
    short m_columns;
    int *m_columnWidthPercentage;
    bool m_autoScroll;
    bool m_autoPurge;
    bool m_scrollBar;
    bool m_multiSelect;
    bool m_forceSelect;
    bool m_scrollIfAtEnd;
    int *m_columnWidth;
    _ListEntryRow *m_listData;
    GameWindow *m_upButton;
    GameWindow *m_downButton;
    GameWindow *m_slider;
    int m_totalHeight;
    short m_endPos;
    short m_insertPos;
    int m_selectPos;
    int *m_selections;
    short m_displayHeight;
    int m_doubleClickTime;
    short m_displayPos;
};

void Gadget_List_Box_Set_Font(GameWindow *window, GameFont *font);
