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
#include "playerlist.h"
#include "veterancygaincreate.h"

struct PopulateInvButtonData
{
    int curr_index;
    int max_index;
    GameWindow **controls;
    Object *transport;
};

void ControlBar::Reset_Contain_Data()
{
    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; ++i) {
        s_containData[i].button = nullptr;
        s_containData[i].id = INVALID_OBJECT_ID;
    }
}

void ControlBar::Reset_Build_Queue_Data()
{
    for (int i = 0; i < QUEUE_ENTRY_COUNT; i++) {
        m_queueData[i].control = nullptr;
        m_queueData[i].type = ProductionEntry::PRODUCTION_INVALID;
        m_queueData[i].production_id = INVALID_PRODUCTION_ID;
    }
}

void ControlBar::Populate_Inv_Data_Callback(Object *obj, void *user_data)
{
    PopulateInvButtonData *data = static_cast<PopulateInvButtonData *>(user_data);

    if (data->curr_index <= data->max_index) {
        GameWindow *control = data->controls[data->curr_index];
        captainslog_dbgassert(control, "Populate_Inv_Data_Callback: Control not found");
        s_containData[data->curr_index].button = control;
        s_containData[data->curr_index].id = obj->Get_ID();
        data->curr_index++;
        Gadget_Button_Set_Enabled_Image(control, obj->Get_Template()->Get_Button_Image());
        Gadget_Button_Draw_Overlay_Image(control, Calculate_Veterancy_Overlay_For_Object(obj));
        control->Win_Enable(true);
    } else {
        captainslog_dbgassert(false,
            "There is not enough GUI slots to hold the # of items inside a '%s'",
            data->transport->Get_Template()->Get_Name().Str());
    }
}

const Image *ControlBar::Calculate_Veterancy_Overlay_For_Object(const Object *object)
{
    if (object == nullptr) {
        return nullptr;
    }

    switch (object->Get_Veterancy_Level()) {
        case VETERANCY_VETERAN:
            return s_rankVeteranIcon;
        case VETERANCY_ELITE:
            return s_rankEliteIcon;
        case VETERANCY_HEROIC:
            return s_rankHeroicIcon;
    }

    return nullptr;
}

const Image *ControlBar::Calculate_Veterancy_Overlay_For_Thing(const ThingTemplate *thing)
{
    VeterancyLevel level = VETERANCY_REGULAR;

    if (thing == nullptr) {
        return nullptr;
    }

    Player *player = g_thePlayerList->Get_Local_Player();

    if (player == nullptr) {
        return nullptr;
    }

    const VeterancyGainCreateModuleData *data = nullptr;
    Utf8String str;
    const ModuleInfo *modules = thing->Get_Body_Modules();

    for (unsigned int i = 0; i < modules->Get_Count(); i++) {
        str = modules->Get_Nth_Name(i);

        if (str == "VeterancyGainCreate") {
            data = static_cast<const VeterancyGainCreateModuleData *>(modules->Get_Nth_Data(i));

            if (data != nullptr) {
                if (data->m_scienceRequired == SCIENCE_INVALID
                    || (player->Has_Science(data->m_scienceRequired) && data->m_startingLevel > level)) {
                    level = data->m_startingLevel;
                }
            }
        }
    }

    switch (level) {
        case VETERANCY_VETERAN:
            return s_rankVeteranIcon;
        case VETERANCY_ELITE:
            return s_rankEliteIcon;
        case VETERANCY_HEROIC:
            return s_rankHeroicIcon;
        default:
            return nullptr;
    }
}

void ControlBar::Update_Context_Command()
{
#ifdef GAME_DLL
    Call_Method<void, ControlBar>(PICK_ADDRESS(0x005A3B20, 0x008E3643), this);
#endif
}
