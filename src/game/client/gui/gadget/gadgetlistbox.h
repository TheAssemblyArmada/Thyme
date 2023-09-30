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
#include "gamewindow.h"

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
    bool m_audioFeedback;
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

struct _GetTextStruct
{
    int column;
    int row;
};

struct _TextAndColor
{
    Utf16String string;
    int color;
};

struct _AddMessageStruct
{
    int row;
    int column;
    void *data;
    int type;
    bool overwrite;
    int height;
    int width;
};

inline GameWindow *Gadget_List_Box_Get_Slider(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr && data->m_slider != nullptr) {
        return data->m_slider;
    } else {
        return nullptr;
    }
}

inline GameWindow *Gadget_List_Box_Get_Up_Button(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr && data->m_upButton != nullptr) {
        return data->m_upButton;
    } else {
        return nullptr;
    }
}

inline GameWindow *Gadget_List_Box_Get_Down_Button(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr && data->m_downButton != nullptr) {
        return data->m_downButton;
    } else {
        return nullptr;
    }
}

inline void Gadget_List_Box_Set_Enabled_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Enabled_Image(0, image);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Left_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Enabled_Image(1, image);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Right_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Enabled_Image(2, image);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Enabled_Image(3, image);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Small_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Enabled_Image(4, image);
}

inline void Gadget_List_Box_Set_Disabled_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Disabled_Image(0, image);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Left_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Disabled_Image(1, image);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Right_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Disabled_Image(2, image);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Disabled_Image(3, image);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Small_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Disabled_Image(4, image);
}

inline void Gadget_List_Box_Set_Hilite_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Hilite_Image(0, image);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Left_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Hilite_Image(1, image);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Right_End_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Hilite_Image(2, image);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Hilite_Image(3, image);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Small_Repeating_Center_Image(GameWindow *list_box, const Image *image)
{
    list_box->Win_Set_Hilite_Image(4, image);
}

inline void Gadget_List_Box_Set_Enabled_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Enabled_Color(0, color);
}

inline void Gadget_List_Box_Set_Enabled_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Enabled_Border_Color(0, color);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Enabled_Color(1, color);
}

inline void Gadget_List_Box_Set_Enabled_Selected_Item_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Enabled_Border_Color(1, color);
}

inline int Gadget_List_Box_Get_Enabled_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Enabled_Color(0);
}

inline int Gadget_List_Box_Get_Enabled_Border_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Enabled_Border_Color(0);
}

inline void Gadget_List_Box_Set_Disabled_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Disabled_Color(0, color);
}

inline void Gadget_List_Box_Set_Disabled_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Disabled_Border_Color(0, color);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Disabled_Color(1, color);
}

inline void Gadget_List_Box_Set_Disabled_Selected_Item_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Disabled_Border_Color(1, color);
}

inline int Gadget_List_Box_Get_Disabled_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Disabled_Color(0);
}

inline int Gadget_List_Box_Get_Disabled_Border_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Disabled_Border_Color(0);
}

inline void Gadget_List_Box_Set_Hilite_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Hilite_Color(0, color);
}

inline void Gadget_List_Box_Set_Hilite_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Hilite_Border_Color(0, color);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Hilite_Color(1, color);
}

inline void Gadget_List_Box_Set_Hilite_Selected_Item_Border_Color(GameWindow *list_box, int color)
{
    list_box->Win_Set_Hilite_Border_Color(1, color);
}

inline int Gadget_List_Box_Get_Hilite_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Hilite_Color(0);
}

inline int Gadget_List_Box_Get_Hilite_Border_Color(GameWindow *list_box)
{
    return list_box->Win_Get_Hilite_Border_Color(0);
}

WindowMsgHandledType Gadget_List_Box_Input(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_List_Box_Multi_Input(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2);

WindowMsgHandledType Gadget_List_Box_System(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2);

void Gadget_List_Box_Set_Colors(GameWindow *list_box,
    int enabled_color,
    int enabled_border_color,
    int enabled_selected_item_color,
    int enabled_selected_item_border_color,
    int disabled_color,
    int disabled_border_color,
    int disabled_selected_item_color,
    int disabled_selected_item_border_color,
    int hilite_color,
    int hilite_border_color,
    int hilite_selected_item_color,
    int hilite_selected_item_border_color);

int Gadget_List_Box_Add_Entry_Image(
    GameWindow *list_box, const Image *image, int row, int column, int width, int height, bool overwrite, int color);

int Gadget_List_Box_Add_Entry_Image(
    GameWindow *list_box, const Image *image, int row, int column, bool overwrite, int color);

void Gadget_List_Box_Set_Font(GameWindow *list_box, GameFont *font);
void Gadget_List_Box_Get_Entry_Based_On_XY(GameWindow *list_box, int x, int y, int &row, int &column);
Utf16String Gadget_List_Box_Get_Text(GameWindow *list_box, int row, int column);
Utf16String Gadget_List_Box_Get_Text_And_Color(GameWindow *list_box, int *color, int row, int column);
int Gadget_List_Box_Add_Entry_Text(GameWindow *list_box, Utf16String text, int color, int row, int column, bool overwrite);
void Gadget_List_Box_Create_Scroll_Bar(GameWindow *list_box);
void Gadget_List_Box_Add_Multi_Select(GameWindow *list_box);
void Gadget_List_Box_Remove_Multi_Select(GameWindow *list_box);
void Gadget_List_Box_Set_List_Length(GameWindow *list_box, int new_length);
int Gadget_List_Box_Get_List_Length(GameWindow *list_box);
int Gadget_List_Box_Get_Num_Entries(GameWindow *list_box);
void Gadget_List_Box_Get_Selected(GameWindow *list_box, int *select_list);
void Gadget_List_Box_Set_Selected(GameWindow *list_box, int select_index);
void Gadget_List_Box_Set_Selected(GameWindow *list_box, int select_count, int *select_list);
void Gadget_List_Box_Reset(GameWindow *list_box);
void Gadget_List_Box_Set_Item_Data(GameWindow *list_box, void *data, int row, int column);
void *Gadget_List_Box_Get_Item_Data(GameWindow *list_box, int row, int column);
int Gadget_List_Box_Get_Bottom_Visible_Entry(GameWindow *list_box);
bool Gadget_List_Box_Is_Full(GameWindow *list_box);
void Gadget_List_Box_Set_Bottom_Visible_Entry(GameWindow *list_box, int row);
int Gadget_List_Box_Get_Top_Visible_Entry(GameWindow *list_box);
void Gadget_List_Box_Set_Top_Visible_Entry(GameWindow *list_box, int row);
void Gadget_List_Box_Set_Audio_Feedback(GameWindow *list_box, bool audio_feedback);
int Gadget_List_Box_Get_Num_Columns(GameWindow *list_box);
int Gadget_List_Box_Get_Column_Width(GameWindow *list_box, int column);
