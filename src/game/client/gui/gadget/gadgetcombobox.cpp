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
#include "gamewindow.h"

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

GameWindow *Gadget_Combo_Box_Get_List_Box(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr && data->m_listBox != nullptr) {
        return data->m_listBox;
    } else {
        return nullptr;
    }
}

GameWindow *Gadget_Combo_Box_Get_Edit_Box(GameWindow *combo_box)
{
    _ComboBoxData *data = static_cast<_ComboBoxData *>(combo_box->Win_Get_User_Data());

    if (data != nullptr && data->m_editBox != nullptr) {
        return data->m_editBox;
    } else {
        return nullptr;
    }
}
