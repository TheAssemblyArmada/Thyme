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
