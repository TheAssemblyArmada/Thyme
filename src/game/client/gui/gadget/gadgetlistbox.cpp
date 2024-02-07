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
#include "gadgetlistbox.h"
#include "audioeventrts.h"
#include "audiomanager.h"
#include "displaystring.h"
#include "displaystringmanager.h"
#include "gadgetpushbutton.h"
#include "gadgetslider.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"

void Gadget_List_Box_Get_Entry_Based_On_Coord(GameWindow *list_box, int x, int y, int &row, int &column)
{
    WinInstanceData *instance_data = list_box->Win_Get_Instance_Data();
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    int screen_x;
    int screen_y;
    list_box->Win_Get_Screen_Position(&screen_x, &screen_y);

    if (instance_data->Get_Text_Length() != 0) {
        screen_y += g_theWindowManager->Win_Font_Height(instance_data->Get_Font());
    }

    int irow = -2;
    int icol;

    for (icol = 0;; icol++) {
        if (icol > 0 && data->m_listData[icol - 1].m_listHeight > data->m_displayHeight + data->m_displayPos) {
            break;
        }

        if (icol == data->m_endPos) {
            irow = -1;
            break;
        }

        if (data->m_listData[icol].m_listHeight > data->m_displayPos + y - screen_y) {
            break;
        }
    }

    column = -1;

    if (irow == -2) {
        irow = icol;
        int col_width = 0;

        for (icol = 0; icol < data->m_columns; icol++) {
            col_width += data->m_columnWidth[icol];

            if (x - screen_x < col_width) {
                column = icol;
                break;
            }
        }
    }

    row = irow;
}

void Gadget_List_Box_Get_Entry_Based_On_XY(GameWindow *list_box, int x, int y, int &row, int &column)
{
    Gadget_List_Box_Get_Entry_Based_On_Coord(list_box, x, y, row, column);
}

void Do_Audio_Feedback(GameWindow *list_box)
{
    if (list_box) {
        _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

        if (data != nullptr) {
            if (data->m_audioFeedback) {
                AudioEventRTS event("GUIComboBoxClick");

                if (g_theAudio != nullptr) {
                    g_theAudio->Add_Audio_Event(&event);
                }
            }
        }
    }
}

int Get_List_Box_Top_Entry(_ListboxData *list)
{
    for (int i = 0;; i++) {
        if (list->m_listData[i].m_listHeight > list->m_displayPos) {
            return i;
        }

        if (i >= list->m_endPos) {
            break;
        }
    }

    return 0;
}

void Adjust_Display(GameWindow *list_box, int adjustment, bool update_slider)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    int index = adjustment + Get_List_Box_Top_Entry(data);

    if (index >= 0) {
        if (index >= data->m_endPos) {
            index = data->m_endPos - 1;
        }
    } else {
        index = 0;
    }

    if (update_slider) {
        if (index > 0) {
            data->m_displayPos = data->m_listData[index - 1].m_listHeight + 1;
        } else {
            data->m_displayPos = 0;
        }
    }

    if (data->m_slider != nullptr) {
        _SliderData *silder_data = static_cast<_SliderData *>(data->m_slider->Win_Get_User_Data());
        int slider_width;
        int slider_height;
        data->m_slider->Win_Get_Size(&slider_width, &slider_height);
        silder_data->m_maxVal = data->m_totalHeight - (data->m_displayHeight - 2) + 1;

        if (silder_data->m_maxVal < 0) {
            silder_data->m_maxVal = 0;
        }

        GameWindow *child = data->m_slider->Win_Get_Child();
        int child_width;
        int child_height;
        child->Win_Get_Size(&child_width, &child_height);
        silder_data->m_numTicks = (float)(slider_height - child_height) / (float)silder_data->m_maxVal;

        if (update_slider) {
            g_theWindowManager->Win_Send_System_Msg(
                data->m_slider, GSM_SET_SLIDER, silder_data->m_maxVal - data->m_displayPos, 0);
        }
    }
}

WindowMsgHandledType Gadget_List_Box_Input(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005E50A0, 0x00A049C3), list_box, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_List_Box_Multi_Input(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005E5CC0, 0x00A057B8), list_box, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

void Remove_Selection(_ListboxData *data, int selection)
{
    memcpy(
        &data->m_selections[selection], &data->m_selections[selection + 1], sizeof(int) * (data->m_listLength - selection));
    data->m_selections[data->m_listLength - 1] = -1;
}

void Compute_Total_Height(GameWindow *list_box)
{
    int total_height = 0;
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    WinInstanceData *instance_data = list_box->Win_Get_Instance_Data();

    for (int i = 0; i < data->m_endPos; i++) {
        if (data->m_listData[i].m_cell == nullptr) {
            continue;
        }

        int height = 0;

        for (int j = 0; j < data->m_columns; j++) {
            int new_height = 0;

            if (data->m_listData[i].m_cell[j].m_cellType != LISTBOX_TEXT) {
                if (data->m_listData[i].m_cell[j].m_cellType != LISTBOX_IMAGE) {
                    if (new_height > height) {
                        height = new_height;
                    }

                    continue;
                }

                if (data->m_listData[i].m_cell[j].m_height > 0) {
                    new_height = data->m_listData[i].m_cell[j].m_height + 1;

                    if (new_height > height) {
                        height = new_height;
                    }

                    continue;
                }

                new_height = g_theWindowManager->Win_Font_Height(instance_data->Get_Font());

                if (new_height > height) {
                    height = new_height;
                }

                continue;
            }

            if ((list_box->Win_Get_Status() & GWS_TAB_PANE) != 0) {
                new_height = g_theWindowManager->Win_Font_Height(instance_data->Get_Font());

                if (new_height > height) {
                    height = new_height;
                }

                continue;
            }

            DisplayString *string = static_cast<DisplayString *>(data->m_listData[i].m_cell[j].m_data);

            if (string != nullptr) {
                string->Get_Size(nullptr, &new_height);
            }

            if (new_height > height) {
                height = new_height;
            }
        }

        data->m_listData[i].m_height = height;
        total_height += data->m_listData[i].m_height + 1;
        data->m_listData[i].m_listHeight = total_height;
    }

    data->m_totalHeight = total_height;
    Adjust_Display(list_box, 0, true);
}

int Add_Image_Entry(
    Image *image, int color, int row, int column, GameWindow *list_box, bool overwrite, int height, int width)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (column < data->m_columns && row < data->m_listLength) {
        if (row == -1) {
            row = data->m_insertPos++;
            data->m_endPos++;
        }

        if (column == -1) {
            column = 0;
        }

        _ListEntryRow *list_row = &data->m_listData[row];

        if (list_row->m_cell == nullptr) {
            list_row->m_cell = new _ListEntryCell[data->m_columns];
            memset(list_row->m_cell, 0, sizeof(_ListEntryCell) * data->m_columns);
        }

        if (list_row->m_cell[column].m_cellType == LISTBOX_TEXT) {
            g_theDisplayStringManager->Free_Display_String(static_cast<DisplayString *>(list_row->m_cell[column].m_data));
        }

        list_row->m_cell[column].m_cellType = LISTBOX_IMAGE;
        list_row->m_cell[column].m_data = image;
        list_row->m_cell[column].m_textColor = color;
        list_row->m_cell[column].m_height = width;
        list_row->m_cell[column].m_width = height;
        Compute_Total_Height(list_box);
        return row;
    } else {
        captainslog_dbgassert(false, "Tried to add Image to Listbox at invalid position");
        return -1;
    }
}

int Move_Rows_Down(_ListboxData *data, int row)
{
    int size = sizeof(_ListEntryRow) * (data->m_endPos - row);
    char *buf = new char[size];
    memcpy(buf, &data->m_listData[row], size);
    memcpy(&data->m_listData[row + 1], buf, size);
    delete[] buf;
    data->m_insertPos = ++data->m_endPos;
    data->m_listData[row].m_cell = 0;
    data->m_listData[row].m_height = 0;
    data->m_listData[row].m_listHeight = 0;

    if (data->m_multiSelect) {
        for (int i = 0; data->m_selections[i] >= 0; i++) {
            if (row <= data->m_selections[i]) {
                data->m_selections[i]++;
            }
        }
    } else if (data->m_selectPos >= row) {
        data->m_selectPos++;
    }

    return 1;
}

int Add_Entry(Utf16String *string, int color, int row, int column, GameWindow *list_box, bool overwrite)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (column < data->m_columns && row < data->m_listLength) {
        if (row == -1) {
            row = data->m_insertPos++;
            data->m_endPos++;
        }

        if (column == -1) {
            column = 0;
        }

        int wrap = data->m_columnWidth[column] - 7;
        int adjust = 0;
        _ListEntryRow *list_row = &data->m_listData[row];

        if (list_row->m_cell != nullptr) {
            if (!overwrite) {
                Move_Rows_Down(data, row);
                list_row->m_cell = new _ListEntryCell[data->m_columns];
                memset(list_row->m_cell, 0, sizeof(_ListEntryCell) * data->m_columns);
                adjust = 1;
            }
        } else {
            list_row->m_cell = new _ListEntryCell[data->m_columns];
            memset(list_row->m_cell, 0, sizeof(_ListEntryCell) * data->m_columns);
            adjust = 1;
        }

        list_row->m_cell[column].m_cellType = LISTBOX_TEXT;
        list_row->m_cell[column].m_textColor = color;

        if (list_row->m_cell[column].m_data == nullptr) {
            list_row->m_cell[column].m_data = g_theDisplayStringManager->New_Display_String();
        }

        DisplayString *text = static_cast<DisplayString *>(list_row->m_cell[column].m_data);

        if ((list_box->Win_Get_Status() & GWS_TAB_PANE) == 0) {
            text->Set_Word_Wrap(wrap);
        }

        text->Set_Font(list_box->Win_Get_Font());

        if (overwrite) {
            int height = list_row->m_height;
            int list_height = list_row->m_listHeight;

            if (list_height == 0 && row != 0) {
                list_height = data->m_listData[row - 1].m_listHeight;
            }

            int size;
            text->Get_Size(nullptr, &size);

            if (size > height) {
                list_row->m_height = size;
                list_row->m_listHeight = adjust + (size - height + list_height);
                data->m_totalHeight += adjust + size - height;
                Adjust_Display(list_box, 0, true);
            }
        } else {
            Compute_Total_Height(list_box);
        }

        return row;
    } else {
        captainslog_dbgassert(false, "Tried to add Text to Listbox at invalid position");
        return -1;
    }
}

WindowMsgHandledType Gadget_List_Box_System(
    GameWindow *list_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005E6140, 0x00A05CDE), list_box, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

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
    int hilite_selected_item_border_color)
{
    _ListboxData *lb_data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    Gadget_List_Box_Set_Enabled_Color(list_box, enabled_color);
    Gadget_List_Box_Set_Enabled_Border_Color(list_box, enabled_border_color);
    Gadget_List_Box_Set_Enabled_Selected_Item_Color(list_box, enabled_selected_item_color);
    Gadget_List_Box_Set_Enabled_Selected_Item_Border_Color(list_box, enabled_selected_item_border_color);
    Gadget_List_Box_Set_Disabled_Color(list_box, disabled_color);
    Gadget_List_Box_Set_Disabled_Border_Color(list_box, disabled_border_color);
    Gadget_List_Box_Set_Disabled_Selected_Item_Color(list_box, disabled_selected_item_color);
    Gadget_List_Box_Set_Disabled_Selected_Item_Border_Color(list_box, disabled_selected_item_border_color);
    Gadget_List_Box_Set_Hilite_Color(list_box, hilite_color);
    Gadget_List_Box_Set_Hilite_Border_Color(list_box, hilite_border_color);
    Gadget_List_Box_Set_Hilite_Selected_Item_Color(list_box, hilite_selected_item_color);
    Gadget_List_Box_Set_Hilite_Selected_Item_Border_Color(list_box, hilite_selected_item_border_color);
    GameWindow *slider = lb_data->m_slider;

    if (slider != nullptr) {
        GameWindow *up_button = lb_data->m_upButton;
        GameWindow *down_button = lb_data->m_downButton;
        Gadget_Slider_Set_Enabled_Color(slider, Gadget_List_Box_Get_Enabled_Color(list_box));
        Gadget_Slider_Set_Enabled_Border_Color(slider, Gadget_List_Box_Get_Enabled_Border_Color(list_box));
        Gadget_Slider_Set_Disabled_Color(slider, Gadget_List_Box_Get_Disabled_Color(list_box));
        Gadget_Slider_Set_Disabled_Border_Color(slider, Gadget_List_Box_Get_Disabled_Border_Color(list_box));
        Gadget_Slider_Set_Hilite_Color(slider, Gadget_List_Box_Get_Hilite_Color(list_box));
        Gadget_Slider_Set_Hilite_Border_Color(slider, Gadget_List_Box_Get_Hilite_Border_Color(list_box));

        Gadget_Button_Set_Enabled_Color(up_button, Gadget_Slider_Get_Enabled_Color(slider));
        Gadget_Button_Set_Enabled_Border_Color(up_button, Gadget_Slider_Get_Enabled_Border_Color(slider));
        Gadget_Button_Set_Enabled_Selected_Color(up_button, Gadget_Slider_Get_Thumb_Enabled_Selected_Color(slider));
        Gadget_Button_Set_Enabled_Selected_Border_Color(
            up_button, Gadget_Slider_Get_Thumb_Enabled_Selected_Border_Color(slider));

        Gadget_Button_Set_Disabled_Color(up_button, Gadget_Slider_Get_Disabled_Color(slider));
        Gadget_Button_Set_Disabled_Border_Color(up_button, Gadget_Slider_Get_Disabled_Border_Color(slider));
        Gadget_Button_Set_Disabled_Selected_Color(up_button, Gadget_Slider_Get_Thumb_Disabled_Selected_Color(slider));
        Gadget_Button_Set_Disabled_Selected_Border_Color(
            up_button, Gadget_Slider_Get_Thumb_Disabled_Selected_Border_Color(slider));

        Gadget_Button_Set_Hilite_Color(up_button, Gadget_Slider_Get_Hilite_Color(slider));
        Gadget_Button_Set_Hilite_Border_Color(up_button, Gadget_Slider_Get_Hilite_Border_Color(slider));
        Gadget_Button_Set_Hilite_Selected_Color(up_button, Gadget_Slider_Get_Thumb_Hilite_Selected_Color(slider));
        Gadget_Button_Set_Hilite_Selected_Border_Color(
            up_button, Gadget_Slider_Get_Thumb_Hilite_Selected_Border_Color(slider));

        Gadget_Button_Set_Enabled_Color(down_button, Gadget_Slider_Get_Enabled_Color(slider));
        Gadget_Button_Set_Enabled_Border_Color(down_button, Gadget_Slider_Get_Enabled_Border_Color(slider));
        Gadget_Button_Set_Enabled_Selected_Color(down_button, Gadget_Slider_Get_Thumb_Enabled_Selected_Color(slider));
        Gadget_Button_Set_Enabled_Selected_Border_Color(
            down_button, Gadget_Slider_Get_Thumb_Enabled_Selected_Border_Color(slider));

        Gadget_Button_Set_Disabled_Color(down_button, Gadget_Slider_Get_Disabled_Color(slider));
        Gadget_Button_Set_Disabled_Border_Color(down_button, Gadget_Slider_Get_Disabled_Border_Color(slider));
        Gadget_Button_Set_Disabled_Selected_Color(down_button, Gadget_Slider_Get_Thumb_Disabled_Selected_Color(slider));
        Gadget_Button_Set_Disabled_Selected_Border_Color(
            down_button, Gadget_Slider_Get_Thumb_Disabled_Selected_Border_Color(slider));

        Gadget_Button_Set_Hilite_Color(down_button, Gadget_Slider_Get_Hilite_Color(slider));
        Gadget_Button_Set_Hilite_Border_Color(down_button, Gadget_Slider_Get_Hilite_Border_Color(slider));
        Gadget_Button_Set_Hilite_Selected_Color(down_button, Gadget_Slider_Get_Thumb_Hilite_Selected_Color(slider));
        Gadget_Button_Set_Hilite_Selected_Border_Color(
            down_button, Gadget_Slider_Get_Thumb_Hilite_Selected_Border_Color(slider));
    }
}

Utf16String Gadget_List_Box_Get_Text(GameWindow *list_box, int row, int column)
{
    int color;
    return Gadget_List_Box_Get_Text_And_Color(list_box, &color, row, column);
}

Utf16String Gadget_List_Box_Get_Text_And_Color(GameWindow *list_box, int *color, int row, int column)
{
    *color = 0;

    if (list_box == nullptr || row == -1 || column == -1) {
        return Utf16String::s_emptyString;
    } else {
        if ((list_box->Win_Get_Style() & GWS_SCROLL_LISTBOX) != 0) {
            _TextAndColor text;
            _GetTextStruct get;
            get.column = column;
            get.row = row;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
            g_theWindowManager->Win_Send_System_Msg(
                list_box, GLM_GET_TEXT, reinterpret_cast<unsigned int>(&get), reinterpret_cast<unsigned int>(&text));
#endif
            *color = text.color;
            return text.string;
        } else {
            return Utf16String::s_emptyString;
        }
    }
}

int Gadget_List_Box_Add_Entry_Text(GameWindow *list_box, Utf16String text, int color, int row, int column, bool overwrite)
{
    if (list_box != nullptr) {
        if (text.Is_Empty()) {
            text = U_CHAR(" ");
        }

        _AddMessageStruct entry;
        entry.row = row;
        entry.column = column;
        entry.type = 1;
        entry.data = &text;
        entry.overwrite = overwrite;
        entry.width = -1;
        entry.height = -1;
        _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

        bool list_exceeded = data->m_listLength <= data->m_endPos;
        int i = list_exceeded ? 0 : 1;
        int bottom = Gadget_List_Box_Get_Bottom_Visible_Entry(list_box);
        int index = 0;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        index = static_cast<int>(
            g_theWindowManager->Win_Send_System_Msg(list_box, GLM_ADD_ENTRY, reinterpret_cast<unsigned int>(&entry), color));
#endif
        if (data->m_scrollIfAtEnd && (index - bottom) == i && Gadget_List_Box_Is_Full(list_box)) {
            Gadget_List_Box_Set_Bottom_Visible_Entry(list_box, index);
        }

        return index;
    } else {
        return -1;
    }
}

int Gadget_List_Box_Add_Entry_Image(
    GameWindow *list_box, const Image *image, int row, int column, int width, int height, bool overwrite, int color)
{
    _AddMessageStruct entry;
    entry.row = row;
    entry.column = column;
    entry.type = 2;
    entry.data = const_cast<Image *>(image);
    entry.overwrite = overwrite;
    entry.width = width;
    entry.height = height;
    int index = 0;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    index = static_cast<int>(
        g_theWindowManager->Win_Send_System_Msg(list_box, GLM_ADD_ENTRY, reinterpret_cast<unsigned int>(&entry), color));
#endif
    return index;
}

int Gadget_List_Box_Add_Entry_Image(GameWindow *list_box, const Image *image, int row, int column, bool overwrite, int color)
{
    return Gadget_List_Box_Add_Entry_Image(list_box, image, row, column, -1, -1, overwrite, color);
}

void Gadget_List_Box_Set_Font(GameWindow *list_box, GameFont *font)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    DisplayString *string = list_box->Win_Get_Instance_Data()->Get_Text_DisplayString();

    if (string != nullptr) {
        string->Set_Font(font);
    }

    DisplayString *tooltip_string = list_box->Win_Get_Instance_Data()->Get_Tooltip_DisplayString();

    if (tooltip_string != nullptr) {
        tooltip_string->Set_Font(font);
    }

    if (data != nullptr) {
        for (int i = 0; i < data->m_listLength; i++) {
            if (data->m_listData[i].m_cell != nullptr) {
                for (int j = 0; j < data->m_columns; j++) {
                    if (data->m_listData[i].m_cell[j].m_cellType == LISTBOX_TEXT) {
                        if (data->m_listData[i].m_cell[j].m_data != nullptr) {
                            static_cast<DisplayString *>(data->m_listData[i].m_cell[j].m_data)->Set_Font(font);
                        }
                    }
                }
            }
        }
    }
}

void Gadget_List_Box_Create_Scroll_Bar(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    WinInstanceData instance_data;
    _SliderData slider_data;
    memset(&slider_data, 0, sizeof(slider_data));
    int status = list_box->Win_Get_Status();
    bool has_text = false;

    int width;
    int height;
    list_box->Win_Get_Size(&width, &height);

    if (list_box->Win_Get_Text_Length() != 0) {
        has_text = true;
    }

    status &= ~(WIN_STATUS_BORDER | WIN_STATUS_NO_INPUT | WIN_STATUS_HIDDEN);
    int font_height = g_theWindowManager->Win_Font_Height(list_box->Win_Get_Font());
    int top;
    int bottom;

    if (has_text) {
        top = font_height + 1;
    } else {
        top = 0;
    }

    if (has_text) {
        bottom = height - (font_height + 1);
    } else {
        bottom = height;
    }

    instance_data.Init();
    int i3 = 21;
    int i4 = 22;
    status |= WIN_STATUS_IMAGE;
    instance_data.m_owner = list_box;
    instance_data.m_style = GWS_PUSH_BUTTON;

    if ((list_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
        instance_data.m_style |= GWS_MOUSE_TRACK;
    }

    data->m_upButton = g_theWindowManager->Go_Go_Gadget_Push_Button(list_box,
        status | WIN_STATUS_ENABLED | WIN_STATUS_ACTIVE,
        width - i3 - 2,
        top + 2,
        i3,
        i4,
        &instance_data,
        nullptr,
        true);
    instance_data.Init();
    instance_data.m_style = GWS_PUSH_BUTTON;
    instance_data.m_owner = list_box;

    if ((list_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
        instance_data.m_style |= GWS_MOUSE_TRACK;
    }

    data->m_downButton = g_theWindowManager->Go_Go_Gadget_Push_Button(list_box,
        status | WIN_STATUS_ENABLED | WIN_STATUS_ACTIVE,
        width - i3 - 2,
        bottom + top - i4 - 2,
        i3,
        i4,
        &instance_data,
        nullptr,
        true);

    instance_data.Init();
    instance_data.m_style = GWS_VERT_SLIDER;
    instance_data.m_owner = list_box;

    if ((list_box->Win_Get_Style() & GWS_MOUSE_TRACK) != 0) {
        instance_data.m_style |= GWS_MOUSE_TRACK;
    }

    memset(&slider_data, 0, sizeof(slider_data));
    data->m_slider = g_theWindowManager->Go_Go_Gadget_Slider(list_box,
        status | WIN_STATUS_ENABLED | WIN_STATUS_ACTIVE,
        width - i3 - 2,
        top + i4 + 3,
        i3,
        bottom - 2 * i4 - 6,
        &instance_data,
        &slider_data,
        nullptr,
        true);
    data->m_scrollBar = true;
}

void Gadget_List_Box_Add_Multi_Select(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    captainslog_dbgassert(data != nullptr && data->m_selections == nullptr, "selections is not NULL");
    data->m_selections = new int[data->m_listLength];
    captainslog_debug("Enable list box multi select: listLength (select) = %d * %d = %d bytes;",
        data->m_listLength,
        sizeof(int),
        sizeof(int) * data->m_listLength);

    if (data->m_selections != nullptr) {
        memset(data->m_selections, -1, sizeof(int) * data->m_listLength);
        data->m_multiSelect = 1;
        list_box->Win_Set_Input_Func(Gadget_List_Box_Multi_Input);
    } else {
        delete data->m_listData;
    }
}

void Gadget_List_Box_Remove_Multi_Select(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data->m_selections != nullptr) {
        delete data->m_selections;
        data->m_selections = nullptr;
    }

    data->m_multiSelect = false;
    list_box->Win_Set_Input_Func(Gadget_List_Box_Input);
}

void Gadget_List_Box_Set_List_Length(GameWindow *list_box, int new_length)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    captainslog_dbgassert(data != nullptr, "We don't have our needed listboxData!");

    if (data != nullptr) {
        captainslog_dbgassert(data->m_columns > 0, "We need at least one Column in the listbox");

        if (data->m_columns >= 1) {
            int columns = data->m_columns;
            _ListEntryRow *row = new _ListEntryRow[new_length];
            captainslog_dbgassert(row != nullptr, "Unable to allocate new data structures for the Listbox");

            if (row != nullptr) {
                memset(row, 0, sizeof(_ListEntryRow) * new_length);

                if (new_length < data->m_listLength) {
                    if (data->m_displayPos > new_length) {
                        data->m_displayPos = new_length;
                    }

                    if (data->m_selectPos > new_length || data->m_multiSelect) {
                        data->m_selectPos = -1;
                    }

                    if (data->m_insertPos > new_length) {
                        data->m_insertPos = new_length;
                    }

                    data->m_endPos = new_length;
                    memcpy(row, data->m_listData, sizeof(_ListEntryRow) * new_length);
                } else {
                    memcpy(row, data->m_listData, sizeof(_ListEntryRow) * data->m_listLength);
                }

                for (int i = 0; i < data->m_listLength; i++) {
                    _ListEntryCell *cell = data->m_listData[i].m_cell;
                    for (int j = columns - 1; j >= 0 && cell != nullptr; --j) {
                        if (i >= new_length && cell[j].m_cellType == LISTBOX_TEXT && i >= new_length) {
                            if (cell[j].m_data != nullptr) {
                                g_theDisplayStringManager->Free_Display_String(static_cast<DisplayString *>(cell[j].m_data));
                            }
                        }
                    }

                    if (i >= new_length) {
                        delete[] data->m_listData[i].m_cell;
                    }

                    data->m_listData[i].m_cell = nullptr;
                }

                data->m_listLength = new_length;

                if (data->m_listData != nullptr) {
                    delete[] data->m_listData;
                }

                data->m_listData = row;

                Compute_Total_Height(list_box);

                if (data->m_listData != nullptr) {
                    if (data->m_multiSelect) {
                        Gadget_List_Box_Remove_Multi_Select(list_box);
                        Gadget_List_Box_Add_Multi_Select(list_box);
                    }
                } else {
                    captainslog_debug("Unable to allocate listbox data pointer");
                }
            }
        }
    }
}

int Gadget_List_Box_Get_List_Length(GameWindow *list_box)
{
    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data->m_multiSelect) {
        return data->m_listLength;
    } else {
        return 1;
    }
}

int Gadget_List_Box_Get_Num_Entries(GameWindow *list_box)
{
    if (list_box == nullptr) {
        return 0;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr) {
        return data->m_endPos;
    } else {
        return 0;
    }
}

void Gadget_List_Box_Get_Selected(GameWindow *list_box, int *select_list)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(list_box, GLM_GET_SELECTION, 0, reinterpret_cast<unsigned int>(select_list));
    }
#endif
}

void Gadget_List_Box_Set_Selected(GameWindow *list_box, int select_index)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(
            list_box, GLM_SET_SELECTION, reinterpret_cast<unsigned int>(&select_index), 1);
    }
#endif
}

void Gadget_List_Box_Set_Selected(GameWindow *list_box, int select_count, int *select_list)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(
            list_box, GLM_SET_SELECTION, select_count, reinterpret_cast<unsigned int>(select_list));
    }
#endif
}

void Gadget_List_Box_Reset(GameWindow *list_box)
{
    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(list_box, GLM_DEL_ALL, 0, 0);
    }
}

void Gadget_List_Box_Set_Item_Data(GameWindow *list_box, void *data, int row, int column)
{
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    ICoord2D pos;
    pos.x = column;
    pos.y = row;

    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(
            list_box, GLM_SET_ITEM_DATA, reinterpret_cast<unsigned int>(&pos), reinterpret_cast<unsigned int>(data));
    }
#endif
}

void *Gadget_List_Box_Get_Item_Data(GameWindow *list_box, int row, int column)
{
    void *data = nullptr;
    _GetTextStruct get;
    get.column = column;
    get.row = row;
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
    g_theWindowManager->Win_Send_System_Msg(
        list_box, GLM_GET_ITEM_DATA, reinterpret_cast<unsigned int>(&get), reinterpret_cast<unsigned int>(&data));
#endif
    return data;
}

int Get_List_Box_Bottom_Entry(_ListboxData *list)
{
    for (int i = list->m_endPos - 1;; i--) {
        if (list->m_listData[i].m_listHeight == list->m_displayHeight + list->m_displayPos) {
            return i;
        }

        if (list->m_listData[i].m_listHeight < list->m_displayHeight + list->m_displayPos && i != list->m_endPos - 1) {
            return i + 1;
        }

        if (list->m_listData[i].m_listHeight < list->m_displayHeight + list->m_displayPos) {
            return i;
        }

        if (i < 0) {
            break;
        }
    }

    return 0;
}

int Gadget_List_Box_Get_Bottom_Visible_Entry(GameWindow *list_box)
{
    if (list_box == nullptr) {
        return 0;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr) {
        return Get_List_Box_Bottom_Entry(data);
    } else {
        return 0;
    }
}

bool Gadget_List_Box_Is_Full(GameWindow *list_box)
{
    if (list_box == nullptr) {
        return false;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());
    return data != nullptr
        && data->m_listData[Get_List_Box_Bottom_Entry(data)].m_listHeight >= data->m_displayPos + data->m_displayHeight - 5;
}

void Gadget_List_Box_Set_Bottom_Visible_Entry(GameWindow *list_box, int row)
{
    if (list_box != nullptr) {
        if (list_box->Win_Get_User_Data() != nullptr) {
            Adjust_Display(list_box, row - Gadget_List_Box_Get_Bottom_Visible_Entry(list_box) + 1, true);
        }
    }
}

int Gadget_List_Box_Get_Top_Visible_Entry(GameWindow *list_box)
{
    if (list_box == nullptr) {
        return false;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr) {
        return Get_List_Box_Top_Entry(data);
    } else {
        return 0;
    }
}

void Gadget_List_Box_Set_Top_Visible_Entry(GameWindow *list_box, int row)
{
    if (list_box != nullptr) {
        if (list_box->Win_Get_User_Data() != nullptr) {
            Adjust_Display(list_box, row - Gadget_List_Box_Get_Top_Visible_Entry(list_box), true);
        }
    }
}

void Gadget_List_Box_Set_Audio_Feedback(GameWindow *list_box, bool audio_feedback)
{
    if (list_box != nullptr) {
        _ListboxData *lb_data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

        if (lb_data != nullptr) {
            lb_data->m_audioFeedback = audio_feedback;
        }
    }
}

int Gadget_List_Box_Get_Num_Columns(GameWindow *list_box)
{
    if (list_box == nullptr) {
        return false;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data != nullptr) {
        return data->m_columns;
    } else {
        return 0;
    }
}

int Gadget_List_Box_Get_Column_Width(GameWindow *list_box, int column)
{
    if (list_box == nullptr) {
        return false;
    }

    _ListboxData *data = static_cast<_ListboxData *>(list_box->Win_Get_User_Data());

    if (data == nullptr) {
        return 0;
    }

    if (data->m_columns > column && column >= 0) {
        return data->m_columnWidth[column];
    }

    return 0;
}
