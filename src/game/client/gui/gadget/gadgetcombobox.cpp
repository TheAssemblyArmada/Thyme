/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Combo Box
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgetcombobox.h"
#include "displaystring.h"
#include "gadgetlistbox.h"
#include "gadgetpushbutton.h"
#include "gadgettextentry.h"
#include "gamewindow.h"

void Hide_List_Box(GameWindow *combo_box)
{
    GameWindow *list_box = Gadget_Combo_Box_Get_List_Box(combo_box);

    if (list_box != nullptr) {
        if (!list_box->Win_Is_Hidden()) {
            list_box->Win_Hide(true);
            int combo_width;
            int combo_height;
            int edit_width;
            int edit_height;
            Gadget_Combo_Box_Get_Edit_Box(combo_box)->Win_Get_Size(&edit_width, &edit_height);
            combo_box->Win_Get_Size(&combo_width, &combo_height);
            combo_box->Win_Set_Size(combo_width, edit_height);
        }
    }
}

WindowMsgHandledType Gadget_Combo_Box_Input(
    GameWindow *combo_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005C33F0, 0x00A525A0), combo_box, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Combo_Box_System(
    GameWindow *combo_box, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005C3870, 0x00A52B06), combo_box, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

void Gadget_Combo_Box_Set_Colors(GameWindow *combo_box,
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
    Gadget_Combo_Box_Set_Enabled_Color(combo_box, enabled_color);
    Gadget_Combo_Box_Set_Enabled_Border_Color(combo_box, enabled_border_color);
    Gadget_Combo_Box_Set_Enabled_Selected_Item_Color(combo_box, enabled_selected_item_color);
    Gadget_Combo_Box_Set_Enabled_Selected_Item_Border_Color(combo_box, enabled_selected_item_border_color);
    Gadget_Combo_Box_Set_Disabled_Color(combo_box, disabled_color);
    Gadget_Combo_Box_Set_Disabled_Border_Color(combo_box, disabled_border_color);
    Gadget_Combo_Box_Set_Disabled_Selected_Item_Color(combo_box, disabled_selected_item_color);
    Gadget_Combo_Box_Set_Disabled_Selected_Item_Border_Color(combo_box, disabled_selected_item_border_color);
    Gadget_Combo_Box_Set_Hilite_Color(combo_box, hilite_color);
    Gadget_Combo_Box_Set_Hilite_Border_Color(combo_box, hilite_border_color);
    Gadget_Combo_Box_Set_Hilite_Selected_Item_Color(combo_box, hilite_selected_item_color);
    Gadget_Combo_Box_Set_Hilite_Selected_Item_Border_Color(combo_box, hilite_selected_item_border_color);
    GameWindow *edit_box = Gadget_Combo_Box_Get_Edit_Box(combo_box);

    if (edit_box != nullptr) {
        Gadget_Button_Set_Enabled_Color(edit_box, enabled_color);
        Gadget_Button_Set_Enabled_Border_Color(edit_box, enabled_border_color);
        Gadget_Button_Set_Enabled_Selected_Color(edit_box, enabled_selected_item_color);
        Gadget_Button_Set_Enabled_Selected_Border_Color(edit_box, enabled_selected_item_border_color);
        Gadget_Button_Set_Disabled_Color(edit_box, disabled_color);
        Gadget_Button_Set_Disabled_Border_Color(edit_box, disabled_border_color);
        Gadget_Button_Set_Disabled_Selected_Color(edit_box, disabled_selected_item_color);
        Gadget_Button_Set_Disabled_Selected_Border_Color(edit_box, disabled_selected_item_border_color);
        Gadget_Button_Set_Hilite_Color(edit_box, hilite_color);
        Gadget_Button_Set_Hilite_Border_Color(edit_box, hilite_border_color);
        Gadget_Button_Set_Hilite_Selected_Color(edit_box, hilite_selected_item_color);
        Gadget_Button_Set_Hilite_Selected_Border_Color(edit_box, hilite_selected_item_border_color);
    }

    GameWindow *drop_down_button = Gadget_Combo_Box_Get_Drop_Down_Button(combo_box);

    if (drop_down_button != nullptr) {
        Gadget_Button_Set_Enabled_Color(drop_down_button, enabled_color);
        Gadget_Button_Set_Enabled_Border_Color(drop_down_button, enabled_border_color);
        Gadget_Button_Set_Enabled_Selected_Color(drop_down_button, enabled_selected_item_color);
        Gadget_Button_Set_Enabled_Selected_Border_Color(drop_down_button, enabled_selected_item_border_color);
        Gadget_Button_Set_Disabled_Color(drop_down_button, disabled_color);
        Gadget_Button_Set_Disabled_Border_Color(drop_down_button, disabled_border_color);
        Gadget_Button_Set_Disabled_Selected_Color(drop_down_button, disabled_selected_item_color);
        Gadget_Button_Set_Disabled_Selected_Border_Color(drop_down_button, disabled_selected_item_border_color);
        Gadget_Button_Set_Hilite_Color(drop_down_button, hilite_color);
        Gadget_Button_Set_Hilite_Border_Color(drop_down_button, hilite_border_color);
        Gadget_Button_Set_Hilite_Selected_Color(drop_down_button, hilite_selected_item_color);
        Gadget_Button_Set_Hilite_Selected_Border_Color(drop_down_button, hilite_selected_item_border_color);
    }
    GameWindow *list_box = Gadget_Combo_Box_Get_List_Box(combo_box);

    if (list_box != nullptr) {
        Gadget_List_Box_Set_Colors(list_box,
            enabled_color,
            enabled_border_color,
            enabled_selected_item_color,
            enabled_selected_item_border_color,
            disabled_color,
            disabled_border_color,
            disabled_selected_item_color,
            disabled_selected_item_border_color,
            hilite_color,
            hilite_border_color,
            hilite_selected_item_color,
            hilite_selected_item_border_color);
    }
}

void Gadget_Combo_Box_Set_Is_Editable(GameWindow *combo_box, bool is_editable)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());
    GameWindow *edit_box = Gadget_Combo_Box_Get_Edit_Box(combo_box);

    if (edit_box != nullptr) {
        data->m_isEditable = is_editable;

        if (is_editable) {
            edit_box->Win_Set_Status(edit_box->Win_Get_Status() & ~WIN_STATUS_NO_INPUT);
        } else {
            edit_box->Win_Set_Status(edit_box->Win_Get_Status() | WIN_STATUS_NO_INPUT);
        }
    }
}

void Gadget_Combo_Box_Set_Letters_And_Numbers_Only(GameWindow *combo_box, bool letters_and_numbers_only)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());
        data->m_lettersAndNumbersOnly = letters_and_numbers_only;

        if (data->m_entryData != nullptr) {
            data->m_entryData->m_alphaNumericalOnly = letters_and_numbers_only;
        }
    }
}

void Gadget_Combo_Box_Set_Ascii_Only(GameWindow *combo_box, bool ascii_only)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());
        data->m_asciiOnly = ascii_only;

        if (data->m_entryData != nullptr) {
            data->m_entryData->m_asciiOnly = ascii_only;
        }
    }
}

void Gadget_Combo_Box_Set_Max_Chars(GameWindow *combo_box, int max_chars)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());
        data->m_maxChars = max_chars;
        data->m_entryData->m_maxTextLen = max_chars;
    }
}

void Gadget_Combo_Box_Set_Max_Display(GameWindow *combo_box, int max_display)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());
    data->m_maxDisplay = max_display;
}

Utf16String Gadget_Combo_Box_Get_Text(GameWindow *combo_box)
{
    if (combo_box != nullptr) {
        if ((combo_box->Win_Get_Style() & GWS_COMBO_BOX) != 0) {
            return Gadget_Text_Entry_Get_Text(Gadget_Combo_Box_Get_Edit_Box(combo_box));
        } else {
            return Utf16String::s_emptyString;
        }
    } else {
        return Utf16String::s_emptyString;
    }
}

void Gadget_Combo_Box_Set_Text(GameWindow *combo_box, Utf16String text)
{
    if (combo_box != nullptr) {
        Gadget_Text_Entry_Set_Text(Gadget_Combo_Box_Get_Edit_Box(combo_box), text);
    }
}

int Gadget_Combo_Box_Add_Entry(GameWindow *combo_box, Utf16String text, int color)
{
    if (combo_box != nullptr) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        return g_theWindowManager->Win_Send_System_Msg(
            combo_box, GCM_ADD_ENTRY, reinterpret_cast<unsigned int>(&text), color);
#else
        return -1;
#endif
    } else {
        return -1;
    }
}

void Gadget_Combo_Box_Reset(GameWindow *combo_box)
{
    if (combo_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(combo_box, GCM_DEL_ALL, 0, 0);
    }
}

void Gadget_Combo_Box_Set_Font(GameWindow *combo_box, GameFont *font)
{
    GameWindow *list_box = Gadget_Combo_Box_Get_List_Box(combo_box);

    if (list_box != nullptr) {
        list_box->Win_Set_Font(font);
    }

    GameWindow *edit_box = Gadget_Combo_Box_Get_Edit_Box(combo_box);

    if (edit_box != nullptr) {
        edit_box->Win_Set_Font(font);
    }

    DisplayString *string = combo_box->Win_Get_Instance_Data()->Get_Text_DisplayString();

    if (string != nullptr) {
        string->Set_Font(font);
    }

    DisplayString *tooltip_string = combo_box->Win_Get_Instance_Data()->Get_Tooltip_DisplayString();

    if (tooltip_string != nullptr) {
        tooltip_string->Set_Font(font);
    }
}

void Gadget_Combo_Box_Hide_List(GameWindow *combo_box)
{
    if (combo_box != nullptr) {
        g_theWindowManager->Win_Send_System_Msg(combo_box, GGM_CLOSE, 0, 0);
    }
}

void Gadget_Combo_Box_Set_Enabled_Text_Colors(GameWindow *combo_box, int color, int border_color)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

        if (data->m_listBox != nullptr) {
            data->m_listBox->Win_Set_Enabled_Text_Colors(color, border_color);
        }

        if (data->m_editBox != nullptr) {
            data->m_editBox->Win_Set_Enabled_Text_Colors(color, border_color);
        }
    }
}

void Gadget_Combo_Box_Set_Disabled_Text_Colors(GameWindow *combo_box, int color, int border_color)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

        if (data->m_listBox != nullptr) {
            data->m_listBox->Win_Set_Disabled_Text_Colors(color, border_color);
        }

        if (data->m_editBox != nullptr) {
            data->m_editBox->Win_Set_Disabled_Text_Colors(color, border_color);
        }
    }
}

void Gadget_Combo_Box_Set_Hilite_Text_Colors(GameWindow *combo_box, int color, int border_color)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

        if (data->m_listBox != nullptr) {
            data->m_listBox->Win_Set_Hilite_Text_Colors(color, border_color);
        }

        if (data->m_editBox != nullptr) {
            data->m_editBox->Win_Set_Hilite_Text_Colors(color, border_color);
        }
    }
}

void Gadget_Combo_Box_Set_IME_Composite_Text_Colors(GameWindow *combo_box, int color, int border_color)
{
    if (combo_box != nullptr) {
        _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

        if (data->m_listBox != nullptr) {
            data->m_listBox->Win_Set_IME_Composite_Text_Colors(color, border_color);
        }

        if (data->m_editBox != nullptr) {
            data->m_editBox->Win_Set_IME_Composite_Text_Colors(color, border_color);
        }
    }
}

void Gadget_Combo_Box_Get_Selected_Pos(GameWindow *combo_box, int *selected_index)
{
    if (combo_box != nullptr) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        g_theWindowManager->Win_Send_System_Msg(
            combo_box, GCM_GET_SELECTION, 0, reinterpret_cast<unsigned int>(selected_index));
#endif
    }
}

void Gadget_Combo_Box_Set_Selected_Pos(GameWindow *combo_box, int selected_index, bool dont_hide)
{
    if (combo_box != nullptr) {
        // pointer to an unsigned int on 64 bit
        g_theWindowManager->Win_Send_System_Msg(combo_box, GCM_SET_SELECTION, selected_index, dont_hide);
    }
}

void Gadget_Combo_Box_Set_Item_Data(GameWindow *combo_box, int index, void *data)
{
    if (combo_box != nullptr) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        g_theWindowManager->Win_Send_System_Msg(combo_box, GCM_SET_ITEM_DATA, index, reinterpret_cast<unsigned int>(data));
#endif
    }
}

void *Gadget_Combo_Box_Get_Item_Data(GameWindow *combo_box, int index)
{
    void *data = nullptr;

    if (combo_box != nullptr) {
#ifdef GAME_DLL // temporary since we can't change the definition of Win_Send_System_Msg at this point and we can't cast a
                // pointer to an unsigned int on 64 bit
        g_theWindowManager->Win_Send_System_Msg(combo_box, GCM_GET_ITEM_DATA, index, reinterpret_cast<unsigned int>(&data));
#endif
    }

    return data;
}

int Gadget_Combo_Box_Get_Length(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr) {
        return data->m_entryCount;
    } else {
        return 0;
    }
}
