/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Static Text
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgetstatictext.h"
#include "displaystring.h"
#include "gamewindow.h"

void Gadget_Static_Text_Set_Font(GameWindow *static_text, GameFont *font)
{
    _TextData *data = static_cast<_TextData *>(static_text->Win_Get_User_Data());
    DisplayString *string = static_text->Win_Get_Instance_Data()->Get_Text_DisplayString();

    if (string != nullptr) {
        string->Set_Font(font);
    }

    DisplayString *tooltip_string = static_text->Win_Get_Instance_Data()->Get_Tooltip_DisplayString();

    if (tooltip_string != nullptr) {
        tooltip_string->Set_Font(font);
    }

    if (data != nullptr) {
        if (data->m_text != nullptr) {
            data->m_text->Set_Font(font);
        }
    }
}
