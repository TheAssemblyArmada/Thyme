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
