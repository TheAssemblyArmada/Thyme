/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Text Entry
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gadgettextentry.h"
#include "displaystring.h"
#include "gamewindow.h"

void Gadget_Text_Entry_Set_Font(GameWindow *text_entry, GameFont *font)
{
    _EntryData *data = static_cast<_EntryData *>(text_entry->Win_Get_User_Data());
    DisplayString *string = text_entry->Win_Get_Instance_Data()->Get_Text_DisplayString();

    if (string != nullptr) {
        string->Set_Font(font);
    }

    DisplayString *tooltip_string = text_entry->Win_Get_Instance_Data()->Get_Tooltip_DisplayString();

    if (tooltip_string != nullptr) {
        tooltip_string->Set_Font(font);
    }

    if (data != nullptr) {
        if (data->m_text != nullptr) {
            data->m_text->Set_Font(font);
        }

        if (data->m_sText != nullptr) {
            data->m_sText->Set_Font(font);
        }
    }
}

WindowMsgHandledType Gadget_Text_Entry_Input(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005CB380, 0x00A51AF0), text_entry, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}

WindowMsgHandledType Gadget_Text_Entry_System(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2)
{
#ifdef GAME_DLL
    return Call_Function<WindowMsgHandledType, GameWindow *, unsigned int, unsigned int, unsigned int>(
        PICK_ADDRESS(0x005CB7C0, 0x00A520D2), text_entry, message, data_1, data_2);
#else
    return MSG_IGNORED;
#endif
}
