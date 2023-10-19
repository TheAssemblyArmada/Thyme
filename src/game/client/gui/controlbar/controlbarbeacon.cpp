/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Control Bar
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "controlbar.h"
#include "gadgettextentry.h"
#include "gamewindowmanager.h"
#include "keyboard.h"

void ControlBar::Update_Context_Beacon() {}

void ControlBar::Populate_Beacon(Object *beacon)
{
    Set_Portrait_By_Object(beacon);
    static const NameKeyType text_id = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:EditBeaconText");
    static const NameKeyType static_text_id = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:StaticTextBeaconLabel");
    static const NameKeyType clear_button_id = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonClearBeaconText");
    GameWindow *text = g_theWindowManager->Win_Get_Window_From_Id(nullptr, text_id);
    GameWindow *static_text = g_theWindowManager->Win_Get_Window_From_Id(nullptr, static_text_id);
    GameWindow *clear_button = g_theWindowManager->Win_Get_Window_From_Id(nullptr, clear_button_id);

    if (beacon->Is_Locally_Controlled()) {
        if (text != nullptr) {
            text->Win_Hide(false);
            Gadget_Text_Entry_Set_Text(text, beacon->Get_Drawable()->Get_Caption_Text());
            g_theWindowManager->Win_Set_Focus(text);
        }

        if (static_text != nullptr) {
            static_text->Win_Hide(false);
        }

        if (clear_button != nullptr) {
            clear_button->Win_Hide(false);
        }
    } else {
        if (text != nullptr) {
            text->Win_Hide(true);
        }

        if (static_text != nullptr) {
            static_text->Win_Hide(true);
        }

        if (clear_button != nullptr) {
            clear_button->Win_Hide(true);
        }
    }
}

WindowMsgHandledType Beacon_Window_Input(
    GameWindow *text_entry, unsigned int message, unsigned int data_1, unsigned int data_2)
{
    if (message != GWM_CHAR || data_1 != Keyboard::KEY_ESCAPE) {
        return MSG_IGNORED;
    }

    g_theInGameUI->Deselect_All_Drawables(true);
    return MSG_HANDLED;
}
