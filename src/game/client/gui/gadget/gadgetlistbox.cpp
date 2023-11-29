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
#include "displaystring.h"
#include "gadgetpushbutton.h"
#include "gadgetslider.h"
#include "gamewindow.h"
#include "gamewindowmanager.h"

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

void Gadget_List_Box_Create_Scroll_Bar(GameWindow *list_box)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *>(PICK_ADDRESS(0x005E7EF0, 0x00A0846C), list_box);
#endif
}

void Gadget_List_Box_Set_List_Length(GameWindow *list_box, int new_length)
{
#ifdef GAME_DLL
    Call_Function<void, GameWindow *, int>(PICK_ADDRESS(0x005E8110, 0x00A08933), list_box, new_length);
#endif
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
        if (index <= 0) {
            data->m_displayPos = 0;
        } else {
            data->m_displayPos = data->m_listData[index - 1].m_listHeight + 1;
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

void Gadget_List_Box_Set_Top_Visible_Entry(GameWindow *list_box, int row)
{
    if (list_box != nullptr) {
        if (list_box->Win_Get_User_Data() != nullptr) {
            Adjust_Display(list_box, row - Gadget_List_Box_Get_Top_Visible_Entry(list_box), true);
        }
    }
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

void Gadget_List_Box_Reset(GameWindow *list_box)
{
    if (list_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(list_box, GLM_DEL_ALL, 0, 0);
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

void Gadget_List_Box_Set_Bottom_Visible_Entry(GameWindow *list_box, int row)
{
    if (list_box != nullptr) {
        if (list_box->Win_Get_User_Data() != nullptr) {
            Adjust_Display(list_box, row - Gadget_List_Box_Get_Bottom_Visible_Entry(list_box) + 1, true);
        }
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
