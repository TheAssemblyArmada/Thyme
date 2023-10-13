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
#include "gadgetpushbutton.h"
#include "hotkey.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"
#include "thingtemplate.h"

struct PopulateButtonInfo
{
    Object *source;
    int button_index;
    ControlBar *self;
    GameWindow **inventory_buttons;
};

void ControlBar::Populate_Button_Proc(Object *obj, void *user_data)
{
    PopulateButtonInfo *data = static_cast<PopulateButtonInfo *>(user_data);
    captainslog_dbgassert(data->button_index < 10,
        "Too many objects inside '%s' for the inventory buttons to hold",
        data->source->Get_Template()->Get_Name().Str());
    s_containData[data->button_index].button = data->inventory_buttons[data->button_index];
    s_containData[data->button_index].id = obj->Get_ID();
    Gadget_Button_Set_Enabled_Image(data->inventory_buttons[data->button_index], obj->Get_Template()->Get_Button_Image());
    Gadget_Button_Draw_Overlay_Image(
        data->inventory_buttons[data->button_index], Calculate_Veterancy_Overlay_For_Object(obj));
    data->inventory_buttons[data->button_index]->Win_Enable(true);
    data->button_index++;
}

void ControlBar::Update_Context_Structure_Inventory()
{
    Object *object = m_currentSelectedDrawable->Get_Object();

    if (object->Is_Locally_Controlled()
        || g_thePlayerList->Get_Local_Player()->Get_Relationship(object->Get_Team()) == NEUTRAL) {
        ContainModuleInterface *contain = object->Get_Contain();
        captainslog_dbgassert(contain != nullptr, "No contain module defined for object in the inventory bar");

        if (contain != nullptr && m_lastRecordedInventoryCount != contain->Get_Contain_Count()) {
            Populate_Structure_Inventory(object);
        }
    } else {
        Drawable *drawable = object->Get_Drawable();

        if (drawable != nullptr) {
            g_theInGameUI->Deselect_Drawable(drawable);
        }
    }
}

void ControlBar::Populate_Structure_Inventory(Object *building)
{
    Reset_Contain_Data();

    if (g_theHotKeyManager != nullptr) {
        g_theHotKeyManager->Reset();
    }

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        if (m_commandWindows[i] != nullptr) {
            m_commandWindows[i]->Win_Hide(true);
        }
    }

    ContainModuleInterface *contain = building->Get_Contain();
    captainslog_dbgassert(contain != nullptr, "Object in structure inventory does not contain a Contain Module");

    if (contain != nullptr) {
        Set_Control_Command(m_commandWindows[11], Find_Command_Button("Command_Evacuate"));
        Set_Control_Command(m_commandWindows[10], Find_Command_Button("Command_Stop"));
        const CommandButton *exit = Find_Command_Button("Command_StructureExit");

        for (int i = 0; i < 10; i++) {
            m_commandWindows[i]->Win_Hide(false);
            m_commandWindows[i]->Win_Enable(false);
            m_commandWindows[i]->Win_Set_Status(WIN_STATUS_ALWAYS_COLOR);
            m_commandWindows[i]->Win_Clear_Status(WIN_STATUS_NOT_READY);
            Set_Control_Command(m_commandWindows[i], exit);
            Gadget_Button_Draw_Overlay_Image(m_commandWindows[i], nullptr);

            if (i + 1 > contain->Get_Contain_Max()) {
                m_commandWindows[i]->Win_Hide(true);
            }
        }

        m_commandWindows[11]->Win_Hide(false);
        m_commandWindows[10]->Win_Hide(false);

        if (contain->Get_Contain_Count()) {
            m_commandWindows[11]->Win_Enable(true);
            m_commandWindows[10]->Win_Enable(true);
        }

        PopulateButtonInfo info;
        info.source = building;
        info.button_index = 0;
        info.self = this;
        info.inventory_buttons = m_commandWindows;
        contain->Iterate_Contained(Populate_Button_Proc, &info, false);
    }
}
