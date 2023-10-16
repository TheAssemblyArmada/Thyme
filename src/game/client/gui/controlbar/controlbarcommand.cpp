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
#include "aiupdate.h"
#include "battleplanupdate.h"
#include "buildassistant.h"
#include "controlbar.h"
#include "dozeraiupdate.h"
#include "gadgetpushbutton.h"
#include "gamewindowmanager.h"
#include "overchargebehavior.h"
#include "playerlist.h"
#include "playertemplate.h"
#include "specialabilityupdate.h"
#include "specialpower.h"
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

void ControlBar::Do_Transport_Inventory_UI(Object *transport, const CommandSet *command_set)
{
    if (transport != nullptr) {
        if (command_set != nullptr) {
            ContainModuleInterface *contain = transport->Get_Contain();

            if (contain != nullptr) {
                int slots = contain->Get_Contain_Max();
                slots -= contain->Get_Extra_Slots_In_Use();
                int first_exit_index = -1;
                int last_exit_index = -1;
                int slot_count = 0;

                for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                    if (m_commandWindows[i] != nullptr) {
                        const CommandButton *command_button = command_set->Get_Command_Button(i);

                        if (command_button != nullptr) {
                            if (command_button->Get_Command() == GUI_COMMAND_EXIT_CONTAINER) {
                                if (first_exit_index == -1) {
                                    first_exit_index = i;
                                }

                                last_exit_index = i;
                                slot_count++;
                                m_commandWindows[i]->Win_Hide(false);
                                m_commandWindows[i]->Win_Enable(false);
                                Gadget_Button_Draw_Overlay_Image(m_commandWindows[i], nullptr);

                                if (transport->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNMANNED)) {
                                    m_commandWindows[i]->Win_Hide(false);
                                }

                                if (slot_count > slots) {
                                    m_commandWindows[i]->Win_Hide(true);
                                }

                                Set_Control_Command(m_commandWindows[i], command_button);
                            }
                        }
                    }
                }

                if (last_exit_index >= 0) {
                    PopulateInvButtonData data;
                    data.controls = m_commandWindows;
                    data.curr_index = first_exit_index;
                    data.max_index = last_exit_index;
                    data.transport = transport;
                    contain->Iterate_Contained(Populate_Inv_Data_Callback, &data, false);
                }

                m_lastRecordedInventoryCount = contain->Get_Contain_Count();
            }
        }
    }
}

void ControlBar::Populate_Command(Object *obj)
{
    Player *player = obj->Get_Controlling_Player();
    Reset_Contain_Data();
    Reset_Build_Queue_Data();
    const CommandSet *command_set = g_theControlBar->Find_Command_Set(obj->Get_Command_Set_String());

    if (command_set == nullptr) {
        for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
            if (m_commandWindows[i] != nullptr) {
                m_commandWindows[i]->Win_Hide(true);
            }
        }

        return;
    }

    if (obj->Get_Contain() != nullptr) {
        ContainModuleInterface *contain = obj->Get_Contain();

        if (contain->Is_Displayed_On_Control_Bar()) {
            Do_Transport_Inventory_UI(obj, command_set);
        }
    }

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        if (m_commandWindows[i] == nullptr) {
            continue;
        }

        const CommandButton *command_button = command_set->Get_Command_Button(i);

        if (command_button == nullptr || (command_button->Get_Options() & COMMAND_OPTION_SCRIPT_ONLY) != 0) {
            m_commandWindows[i]->Win_Hide(true);
            continue;
        }

        if (command_button->Get_Command() != GUI_COMMAND_EXIT_CONTAINER) {
            m_commandWindows[i]->Win_Hide(false);
            m_commandWindows[i]->Win_Enable(true);
            Set_Control_Command(m_commandWindows[i], command_button);

            if ((command_button->Get_Options() & COMMAND_OPTION_NEED_SPECIAL_POWER_SCIENCE) != 0) {
                const SpecialPowerTemplate *power = command_button->Get_Special_Power();

                if (power != nullptr) {
                    if (power->Get_Required_Science() != SCIENCE_INVALID
                        && command_button->Get_Command() != GUI_COMMAND_PURCHASE_SCIENCE
                        && command_button->Get_Command() != GUI_COMMAND_PLAYER_UPGRADE
                        && command_button->Get_Command() != GUI_COMMAND_OBJECT_UPGRADE) {

                        if (!player->Has_Science(power->Get_Required_Science())) {
                            m_commandWindows[i]->Win_Hide(true);
                            continue;
                        }

                        int owned_science_index = -1;
                        ScienceType science;

                        for (unsigned int j = 0; j < command_button->Get_Sciences()->size(); j++) {
                            science = (*command_button->Get_Sciences())[j];

                            if (!player->Has_Science(science)) {
                                break;
                            }

                            owned_science_index = j;
                        }

                        if (owned_science_index != -1) {
                            science = (*command_button->Get_Sciences())[owned_science_index];

                            if (player != nullptr) {
                                if (player->Get_Player_Template() != nullptr) {
                                    if (!player->Get_Player_Template()->Get_Purchase_Command_Set_Rank_One().Is_Empty()) {
                                        if (!player->Get_Player_Template()
                                                 ->Get_Purchase_Command_Set_Rank_Three()
                                                 .Is_Empty()) {
                                            if (!player->Get_Player_Template()
                                                     ->Get_Purchase_Command_Set_Rank_Eight()
                                                     .Is_Empty()) {
                                                const CommandSet *rank_one = g_theControlBar->Find_Command_Set(
                                                    player->Get_Player_Template()->Get_Purchase_Command_Set_Rank_One());
                                                const CommandSet *rank_three = g_theControlBar->Find_Command_Set(
                                                    player->Get_Player_Template()->Get_Purchase_Command_Set_Rank_Three());
                                                const CommandSet *rank_eight = g_theControlBar->Find_Command_Set(
                                                    player->Get_Player_Template()->Get_Purchase_Command_Set_Rank_Eight());

                                                if (rank_one != nullptr) {
                                                    if (rank_three != nullptr && rank_eight != nullptr) {
                                                        bool found = false;

                                                        for (int button_idx = 0; !found && button_idx < RANK_1_BUTTON_COUNT;
                                                             button_idx++) {
                                                            const CommandButton *button =
                                                                rank_one->Get_Command_Button(button_idx);

                                                            if (button != nullptr
                                                                && button->Get_Command() == GUI_COMMAND_PURCHASE_SCIENCE) {
                                                                if (button->Get_Sciences()->empty()) {
                                                                    captainslog_dbgassert(false,
                                                                        "Commandbutton %s is a purchase science button "
                                                                        "without any science! Please add it.",
                                                                        button->Get_Name().Str());
                                                                } else {
                                                                    if ((*button->Get_Sciences())[0] == science) {
                                                                        command_button->Copy_Images_From(button, true);
                                                                        command_button->Copy_Button_Text_From(
                                                                            button, false, true);
                                                                        found = true;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        for (int button_idx = 0; !found && button_idx < RANK_3_BUTTON_COUNT;
                                                             button_idx++) {
                                                            const CommandButton *button =
                                                                rank_three->Get_Command_Button(button_idx);

                                                            if (button != nullptr
                                                                && button->Get_Command() == GUI_COMMAND_PURCHASE_SCIENCE) {
                                                                if (button->Get_Sciences()->empty()) {
                                                                    captainslog_dbgassert(false,
                                                                        "Commandbutton %s is a purchase science button "
                                                                        "without any science! Please add it.",
                                                                        button->Get_Name().Str());
                                                                } else {
                                                                    if ((*button->Get_Sciences())[0] == science) {
                                                                        command_button->Copy_Images_From(button, true);
                                                                        command_button->Copy_Button_Text_From(
                                                                            button, false, true);
                                                                        found = true;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        for (int button_idx = 0; !found && button_idx < RANK_8_BUTTON_COUNT;
                                                             button_idx++) {
                                                            const CommandButton *button =
                                                                rank_eight->Get_Command_Button(button_idx);

                                                            if (button != nullptr
                                                                && button->Get_Command() == GUI_COMMAND_PURCHASE_SCIENCE) {
                                                                if (button->Get_Sciences()->empty()) {
                                                                    captainslog_dbgassert(false,
                                                                        "Commandbutton %s is a purchase science button "
                                                                        "without any science! Please add it.",
                                                                        button->Get_Name().Str());
                                                                } else {
                                                                    if ((*button->Get_Sciences())[0] == science) {
                                                                        command_button->Copy_Images_From(button, true);
                                                                        command_button->Copy_Button_Text_From(
                                                                            button, false, true);
                                                                        found = true;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ExitInterface *exit = obj->Get_Object_Exit_Interface();

    if (exit != nullptr) {
        Show_Rally_Point(exit->Get_Rally_Point());
    }

    Update_Context_Command();
}

void ControlBar::Populate_Build_Queue(Object *producer)
{
    static const CommandButton *cancel_unit_command = Find_Command_Button("Command_CancelUnitCreate");
    static const CommandButton *cancel_upgrade_command = Find_Command_Button("Command_CancelUpgradeCreate");
    Reset_Build_Queue_Data();
    static bool ids_initialized = false;
    static NameKeyType build_queue_ids[QUEUE_ENTRY_COUNT];

    if (!ids_initialized) {
        Utf8String name;

        for (int i = 0; i < QUEUE_ENTRY_COUNT; i++) {
            name.Format("ControlBar.wnd:ButtonQueue%02d", i + 1);
            build_queue_ids[i] = g_theNameKeyGenerator->Name_To_Key(name.Str());
        }

        ids_initialized = true;
    }

    for (int i = 0; i < QUEUE_ENTRY_COUNT; i++) {
        m_queueData[i].control = g_theWindowManager->Win_Get_Window_From_Id(m_contextParent[3], build_queue_ids[i]);
        m_queueData[i].control->Win_Enable(false);
        m_queueData[i].control->Win_Clear_Status(WIN_STATUS_USE_OVERLAY_STATES);
        Gadget_Button_Set_Text(m_queueData[i].control, U_CHAR(""));
        Gadget_Button_Draw_Overlay_Image(m_queueData[i].control, nullptr);
    }

    ProductionUpdateInterface *production = producer->Get_Production_Update_Interface();

    if (production != nullptr) {
        int count = 0;

        for (const ProductionEntry *entry = production->First_Production(); entry != nullptr && count < QUEUE_ENTRY_COUNT;
             entry = production->Next_Production(entry)) {
            if (entry->Get_Type() == ProductionEntry::PRODUCTION_UNIT) {
                Set_Control_Command(m_queueData[count].control, cancel_unit_command);
                m_queueData[count].type = ProductionEntry::PRODUCTION_UNIT;
                m_queueData[count].production_id = entry->Get_Production_ID();
                m_queueData[count].control->Win_Enable(true);
                m_queueData[count].control->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
                Gadget_Button_Set_Enabled_Image(
                    m_queueData[count].control, entry->Get_Production_Object()->Get_Button_Image());
                Gadget_Button_Draw_Overlay_Image(
                    m_queueData[count].control, Calculate_Veterancy_Overlay_For_Thing(entry->Get_Production_Object()));
            } else {
                Set_Control_Command(m_queueData[count].control, cancel_upgrade_command);
                m_queueData[count].type = ProductionEntry::PRODUCTION_UPGRADE;
                m_queueData[count].upgrade_template = entry->Get_Production_Upgrade();
                m_queueData[count].control->Win_Enable(true);
                m_queueData[count].control->Win_Set_Status(WIN_STATUS_USE_OVERLAY_STATES);
                Gadget_Button_Set_Enabled_Image(
                    m_queueData[count].control, entry->Get_Production_Upgrade()->Get_Button_Image());
            }

            count++;
        }

        m_displayedQueueCount = production->Get_Production_Count();
    }
}

void ControlBar::Update_Context_Command()
{
    Object *obj = nullptr;

    if (m_currentSelectedDrawable != nullptr) {
        obj = m_currentSelectedDrawable->Get_Object();
    }

    ContainModuleInterface *contain;

    if (obj != nullptr) {
        contain = obj->Get_Contain();
    } else {
        contain = nullptr;
    }

    if (contain != nullptr && contain->Get_Contain_Max() > 0
        && m_lastRecordedInventoryCount != contain->Get_Contain_Count()) {
        m_lastRecordedInventoryCount = contain->Get_Contain_Count();
        Evaluate_Context_UI();
    }

    ProductionUpdateInterface *production;

    if (obj != nullptr) {
        production = obj->Get_Production_Update_Interface();
    } else {
        production = nullptr;
    }

    if (m_contextParent[3]->Win_Is_Hidden()) {
        if (production != nullptr && production->First_Production() != nullptr) {
            Set_Portrait_By_Object(nullptr);
            m_contextParent[3]->Win_Hide(false);
            Populate_Build_Queue(obj);
        }
    } else if (production != nullptr && production->First_Production() == nullptr) {
        m_contextParent[3]->Win_Hide(true);
        Set_Portrait_By_Object(obj);
    }

    if (!m_contextParent[3]->Win_Is_Hidden()) {
        Set_Portrait_By_Object(nullptr);

        if (production != nullptr) {
            if (production->Get_Production_Count() != m_displayedQueueCount) {
                Populate_Build_Queue(obj);
            }

            const ProductionEntry *entry = production->First_Production();

            if (entry != nullptr) {
                static const NameKeyType win_id = g_theNameKeyGenerator->Name_To_Key("ControlBar.wnd:ButtonQueue01");
                GameWindow *window = g_theWindowManager->Win_Get_Window_From_Id(m_contextParent[3], win_id);
                captainslog_dbgassert(window != nullptr, "Update_Context_Command: Unable to find first build queue button");
                Gadget_Button_Draw_Inverse_Clock(window, entry->Get_Percent_Complete(), m_buildUpClockColor);
            }
        }
    }

    for (int i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
        if (m_commandWindows[i] != nullptr) {
            GameWindow *window = m_commandWindows[i];

            if (!window->Win_Is_Hidden()) {
                CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(window));

                if (button != nullptr) {
                    window->Win_Clear_Status(WIN_STATUS_NOT_READY);
                    window->Win_Clear_Status(WIN_STATUS_ALWAYS_COLOR);
                    CommandAvailability availability = Get_Command_Availability(button, obj, window, nullptr, false);

                    switch (availability) {
                        case COMMAND_AVAILABILITY_DISABLED:
                            window->Win_Enable(false);
                            break;
                        case COMMAND_AVAILABILITY_HIDDEN:
                            window->Win_Hide(true);
                            break;
                        case COMMAND_AVAILABILITY_NOT_READY:
                            window->Win_Enable(false);
                            window->Win_Set_Status(WIN_STATUS_NOT_READY);
                            break;
                        case COMMAND_AVAILABILITY_DISABLED_PERMANENTLY:
                            window->Win_Enable(false);
                            window->Win_Set_Status(WIN_STATUS_ALWAYS_COLOR);
                            break;
                        default:
                            window->Win_Enable(true);
                            break;
                    }

                    if (button->Get_Command() != GUI_COMMAND_EXIT_CONTAINER) {
                        Gadget_Button_Draw_Overlay_Image(
                            window, Calculate_Veterancy_Overlay_For_Thing(*button->Get_Template()));
                    }

                    if ((button->Get_Options() & WIN_STATUS_NO_FOCUS) != 0) {
                        captainslog_dbgassert((window->Win_Get_Status() & WIN_STATUS_CHECK_LIKE) != 0,
                            "Update_Context_Command: Error, gadget window for command '%s' is not check-like!",
                            button->Get_Name().Str());

                        if (availability == COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE) {
                            Gadget_Check_Like_Button_Set_Visual_Check(window, true);
                        } else {
                            Gadget_Check_Like_Button_Set_Visual_Check(window, false);
                        }
                    }
                }
            }
        }
    }
}

int Get_Rappeller_Count(Object *obj)
{
    int count = 0;

    const std::list<Object *> *list;

    if (obj->Get_Contain() != nullptr) {
        list = obj->Get_Contain()->Get_Contained_Items_List();
    } else {
        list = nullptr;
    }

    if (list != nullptr) {
        for (auto it = list->begin(); it != list->end(); it++) {
            if ((*it)->Is_KindOf(KINDOF_CAN_RAPPEL)) {
                count++;
            }
        }
    }

    return count;
}

CommandAvailability ControlBar::Get_Command_Availability(
    const CommandButton *button, Object *obj, GameWindow *window, GameWindow *window2, bool b)
{
    if (button->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT
        || button->Get_Command() == GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT) {
        if (g_thePlayerList != nullptr && g_thePlayerList->Get_Local_Player() != nullptr) {
            obj = g_thePlayerList->Get_Local_Player()->Find_Most_Ready_Shortcut_Special_Power_Of_Type(
                button->Get_Special_Power()->Get_Type());
        } else {
            obj = nullptr;
        }
    }

    if (window2 == nullptr) {
        window2 = window;
    }

    if (obj == nullptr) {
        return COMMAND_AVAILABILITY_HIDDEN;
    }

    Player *player = obj->Get_Controlling_Player();

    if (obj->Get_Script_Status(STATUS_ENABLED) || obj->Get_Script_Status(STATUS_POWERED)) {
        return COMMAND_AVAILABILITY_HIDDEN;
    }

    if (obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_UNMANNED)) {
        return COMMAND_AVAILABILITY_HIDDEN;
    }

    if (obj->Get_Single_Use_Command()) {
        return COMMAND_AVAILABILITY_DISABLED;
    }

    if ((button->Get_Options() & COMMAND_OPTION_MUST_BE_STOPPED) != 0) {
        AIUpdateInterface *update = obj->Get_AI_Update_Interface();

        if (update != nullptr) {
            if (update->Is_Moving()) {
                return COMMAND_AVAILABILITY_DISABLED;
            }
        }
    }

    bool disabled = obj->Is_Disabled();

    if (disabled && (button->Get_Options() & COMMAND_OPTION_IGNORES_UNDERPOWERED) != 0) {
        if (obj->Get_Disabled_State_Bits().Test(DISABLED_TYPE_DISABLED_UNDERPOWERED)) {
            if (obj->Get_Disabled_State_Bits().Count() == 1) {
                disabled = false;
            }
        }
    }

    GUICommand command = button->Get_Command();

    if (!disabled || b || command == GUI_COMMAND_SELL || command == GUI_COMMAND_EVACUATE
        || command == GUI_COMMAND_EXIT_CONTAINER || command == GUI_COMMAND_BEACON_DELETE
        || command == GUI_COMMAND_SET_RALLY_POINT || command == GUI_COMMAND_STOP || command == GUI_COMMAND_SWITCH_WEAPON) {
        if ((button->Get_Options() & COMMAND_OPTION_NEED_UPGRADE) != 0) {
            const UpgradeTemplate *upgrade = button->Get_Upgrade_Template();

            if (upgrade != nullptr) {
                if (upgrade->Get_Type() != UPGRADE_TYPE_PLAYER) {
                    if (upgrade->Get_Type() == UPGRADE_TYPE_OBJECT && !obj->Has_Upgrade(upgrade)) {
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                } else if (!player->Has_Upgrade_Complete(upgrade)) {
                    return COMMAND_AVAILABILITY_DISABLED;
                }
            }
        }

        ProductionUpdateInterface *production = obj->Get_Production_Update_Interface();

        if (production != nullptr && production->First_Production() != nullptr
            && (button->Get_Options() & COMMAND_OPTION_NOT_QUEUEABLE) != 0) {
            return COMMAND_AVAILABILITY_DISABLED;
        } else {
            bool production_queue_full;

            if (production != nullptr) {
                production_queue_full = production->Get_Production_Count() == QUEUE_ENTRY_COUNT;
            } else {
                production_queue_full = false;
            }

            switch (button->Get_Command()) {
                case GUI_COMMAND_DOZER_CONSTRUCT: {
                    const ThingTemplate *thing = *button->Get_Template();

                    if (thing != nullptr
                        && (thing->Get_Buildable() == BSTATUS_NO
                            || (thing->Get_Buildable() == BSTATUS_ONLY_AI
                                && obj->Get_Controlling_Player()->Get_Player_Type() != Player::PLAYER_COMPUTER))) {
                        return COMMAND_AVAILABILITY_HIDDEN;
                    } else if (obj->Is_KindOf(KINDOF_DOZER)) {
                        DozerAIInterface *dozer = nullptr;

                        if (obj->Get_AI_Update_Interface() != nullptr) {
                            dozer = obj->Get_AI_Update_Interface()->Get_Dozer_AI_Interface();
                            captainslog_dbgassert(
                                dozer != nullptr, "Something KINDOF_DOZER must have a Dozer-like AIUpdate");

                            if (dozer != nullptr) {
                                if (dozer->Is_Task_Pending(DOZER_TASK_BUILD)) {
                                    return COMMAND_AVAILABILITY_DISABLED;
                                } else if (player->Can_Build(thing)) {
                                    if (player->Can_Afford_Build(thing)) {
                                        return COMMAND_AVAILABILITY_ENABLED;
                                    }

                                    return COMMAND_AVAILABILITY_DISABLED;
                                } else {
                                    return COMMAND_AVAILABILITY_DISABLED;
                                }
                            } else {
                                return COMMAND_AVAILABILITY_DISABLED;
                            }
                        } else {
                            return COMMAND_AVAILABILITY_DISABLED;
                        }
                    } else {
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_UNIT_BUILD: {
                    const ThingTemplate *thing = *button->Get_Template();

                    if (thing != nullptr
                        && (thing->Get_Buildable() == BSTATUS_NO
                            || (thing->Get_Buildable() == BSTATUS_ONLY_AI
                                && obj->Get_Controlling_Player()->Get_Player_Type() != Player::PLAYER_COMPUTER))) {
                        return COMMAND_AVAILABILITY_HIDDEN;
                    } else if (production_queue_full) {
                        return COMMAND_AVAILABILITY_DISABLED;
                    } else {
                        if (player->Can_Build(thing)) {
                            CanMakeType can_make = g_theBuildAssistant->Can_Make_Unit(obj, thing);

                            if (can_make == CAN_MAKE_MAXIMUM_NUMBER || can_make == CAN_MAKE_PARKING_FULL) {
                                return COMMAND_AVAILABILITY_DISABLED;
                            } else {
                                if (can_make != CAN_MAKE_NOT_ENOUGH_MONEY) {
                                    return COMMAND_AVAILABILITY_ENABLED;
                                }

                                return COMMAND_AVAILABILITY_DISABLED;
                            }
                        } else {
                            return COMMAND_AVAILABILITY_DISABLED;
                        }
                    }
                }
                case GUI_COMMAND_PLAYER_UPGRADE: {
                    if (production_queue_full) {
                        return COMMAND_AVAILABILITY_DISABLED;
                    } else {
                        if (player->Has_Upgrade_Complete(button->Get_Upgrade_Template())
                            || player->Has_Upgrade_In_Production(button->Get_Upgrade_Template())) {
                            return COMMAND_AVAILABILITY_DISABLED_PERMANENTLY;
                        } else {
                            if (g_theUpgradeCenter->Can_Afford_Upgrade(player, button->Get_Upgrade_Template(), false)) {
                                for (unsigned int i = 0; i < button->Get_Sciences()->size(); i++) {
                                    if (!player->Has_Science((*button->Get_Sciences())[i])) {
                                        return COMMAND_AVAILABILITY_DISABLED;
                                    }
                                }

                                return COMMAND_AVAILABILITY_ENABLED;
                            } else {
                                return COMMAND_AVAILABILITY_DISABLED;
                            }
                        }
                    }
                }
                case GUI_COMMAND_OBJECT_UPGRADE: {
                    if (production_queue_full) {
                        return COMMAND_AVAILABILITY_DISABLED;
                    } else if (production != nullptr) {
                        if (obj->Has_Upgrade(button->Get_Upgrade_Template())
                            || production->Is_Upgrade_In_Queue(button->Get_Upgrade_Template())
                            || !obj->Affected_By_Upgrade(button->Get_Upgrade_Template())) {
                            return COMMAND_AVAILABILITY_DISABLED_PERMANENTLY;
                        } else {
                            if (g_theUpgradeCenter->Can_Afford_Upgrade(player, button->Get_Upgrade_Template(), false)) {
                                for (unsigned int i = 0;; i++) {

                                    if (i >= button->Get_Sciences()->size()) {
                                        return COMMAND_AVAILABILITY_ENABLED;
                                    }

                                    if (!player->Has_Science((*button->Get_Sciences())[i])) {
                                        break;
                                    }
                                }

                                return COMMAND_AVAILABILITY_DISABLED;
                            } else {
                                return COMMAND_AVAILABILITY_DISABLED;
                            }
                        }
                    } else {
                        captainslog_dbgassert(
                            false, "Objects that have Object-Level Upgrades must also have ProductionUpdate. Just cuz.");
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_STOP: {
                    if ((button->Get_Options() & COMMAND_OPTION_OPTION_ONE) != 0) {
                        static const NameKeyType key_BattlePlanUpdate =
                            g_theNameKeyGenerator->Name_To_Key("BattlePlanUpdate");
                        BattlePlanUpdate *plan =
                            static_cast<BattlePlanUpdate *>(obj->Find_Update_Module(key_BattlePlanUpdate));

                        if (plan == nullptr || plan->Get_Active_Battle_Plan() == BATTLE_PLAN_STATUS_BOMBARDMENT) {
                            return COMMAND_AVAILABILITY_ENABLED;
                        } else {
                            return COMMAND_AVAILABILITY_DISABLED;
                        }
                    } else {
                        return COMMAND_AVAILABILITY_ENABLED;
                    }
                }
                case GUI_COMMAND_EXIT_CONTAINER: {
                    if (!obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_SUBDUED)) {
                        return COMMAND_AVAILABILITY_ENABLED;
                    }

                    return COMMAND_AVAILABILITY_DISABLED;
                }
                case GUI_COMMAND_EVACUATE: {
                    if (obj->Get_Contain() != nullptr && obj->Get_Contain()->Get_Contain_Count() != 0) {
                        if (!obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_SUBDUED)) {
                            return COMMAND_AVAILABILITY_ENABLED;
                        }

                        return COMMAND_AVAILABILITY_DISABLED;
                    } else {
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_EXECUTE_RAILED_TRANSPORT: {
                    DockUpdateInterface *dock = obj->Get_Dock_Update_Interface();

                    if (dock != nullptr && dock->Is_Dock_Open()) {
                        return COMMAND_AVAILABILITY_ENABLED;
                    }

                    return COMMAND_AVAILABILITY_DISABLED;
                }
                case GUI_COMMAND_SELL: {
                    if (obj->Get_Script_Status(STATUS_UNSELLABLE)) {
                        return COMMAND_AVAILABILITY_HIDDEN;
                    } else {
                        if (!obj->Get_Disabled_State(DISABLED_TYPE_DISABLED_SUBDUED)) {
                            return COMMAND_AVAILABILITY_ENABLED;
                        }

                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_FIRE_WEAPON: {
                    if (obj->Get_AI_Update_Interface() != nullptr) {
                        const Weapon *weapon = obj->Get_Weapon_In_Weapon_Slot(button->Get_Weapon_Slot());
                        unsigned int frame = g_theGameLogic->Get_Frame();

                        if (weapon != nullptr && weapon->Get_Clip_Reload_Time(obj) == 0) {
                            return COMMAND_AVAILABILITY_ENABLED;
                        } else {
                            if (weapon != nullptr) {
                                if (weapon->Get_Status() == Weapon::READY_TO_FIRE) {
                                    if (weapon->Get_Next_Shot() != frame) {
                                        if (weapon->Get_Next_Shot() != frame - 1) {
                                            return COMMAND_AVAILABILITY_ENABLED;
                                        }
                                    }
                                }
                            }

                            if (weapon != nullptr) {
                                if (weapon->Get_Status() == Weapon::RELOADING_CLIP) {
                                    Gadget_Button_Draw_Inverse_Clock(
                                        window2, weapon->Get_Percent_Ready_To_Fire() * 100.0f, m_buildUpClockColor);
                                }

                                return COMMAND_AVAILABILITY_NOT_READY;
                            } else {
                                if ((button->Get_Options() & COMMAND_OPTION_USES_MINE_CLEARING_WEAPONSET) != 0
                                    && obj->Test_Weapon_Set_Flag(WEAPONSET_MINE_CLEARING_DETAIL)) {
                                    return COMMAND_AVAILABILITY_ENABLED;
                                } else {
                                    return COMMAND_AVAILABILITY_DISABLED;
                                }
                            }
                        }
                    } else {
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_SPECIAL_POWER:
                case GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT:
                case GUI_COMMAND_SPECIAL_POWER_CONSTRUCT:
                case GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT: {
                    captainslog_dbgassert(button->Get_Special_Power() != nullptr,
                        "The special power in the command '%s' is NULL",
                        button->Get_Name().Str());
                    SpecialPowerModuleInterface *power = obj->Get_Special_Power_Module(button->Get_Special_Power());

                    if (power == nullptr) {
                        captainslog_dbgassert(false,
                            "Object %s does not contain special power module (%s) to execute.  Did you forget to add it to "
                            "the object INI?",
                            obj->Get_Template()->Get_Name().Str(),
                            button->Get_Special_Power()->Get_Name().Str());
                        return COMMAND_AVAILABILITY_ENABLED;
                    }

                    if (power->Is_Ready()) {
                        SpecialAbilityUpdate *ability =
                            obj->Find_Special_Ability_Update(button->Get_Special_Power()->Get_Type());

                        if (ability != nullptr) {
                            if (!ability->Is_Power_Currently_In_Use(button)) {
                                return COMMAND_AVAILABILITY_ENABLED;
                            }

                            return COMMAND_AVAILABILITY_DISABLED;
                        } else {
                            if (power->Get_Special_Power_Template()->Get_Type() != SPECIAL_CHANGE_BATTLE_PLANS) {
                                return COMMAND_AVAILABILITY_ENABLED;
                            }

                            static const NameKeyType key_BattlePlanUpdate =
                                g_theNameKeyGenerator->Name_To_Key("BattlePlanUpdate");
                            BattlePlanUpdate *plan =
                                static_cast<BattlePlanUpdate *>(obj->Find_Update_Module(key_BattlePlanUpdate));

                            if (plan == nullptr) {
                                return COMMAND_AVAILABILITY_ENABLED;
                            }

                            if ((button->Get_Options() & plan->Get_Command_Option()) == 0) {
                                return COMMAND_AVAILABILITY_ENABLED;
                            }

                            return COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE;
                        }
                    } else {
                        Gadget_Button_Draw_Inverse_Clock(window2, power->Get_Percent_Ready() * 100.0f, m_buildUpClockColor);
                        return COMMAND_AVAILABILITY_NOT_READY;
                    }
                }
                case GUI_COMMAND_HACK_INTERNET: {
                    const AIUpdateInterface *update = obj->Get_AI_Update_Interface();

                    if (update != nullptr) {
                        const HackInternetAIInterface *hack = update->Get_Hack_Internet_AI_Interface();

                        if (hack != nullptr) {
                            if (hack->Is_Hacking_Packing_Or_Unpacking()) {
                                return COMMAND_AVAILABILITY_DISABLED;
                            }
                        }
                    }

                    return COMMAND_AVAILABILITY_ENABLED;
                }
                case GUI_COMMAND_TOGGLE_OVERCHARGE: {
                    for (BehaviorModule **module = obj->Get_All_Modules(); *module != nullptr; module++) {
                        OverchargeBehaviorInterface *overcharge = (*module)->Get_Overcharge_Behavior_Interface();

                        if (overcharge != nullptr) {
                            if (overcharge->Is_Overcharge_Active()) {
                                return COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE;
                            }
                        }
                    }

                    return COMMAND_AVAILABILITY_ENABLED;
                }
                case GUI_COMMAND_COMBATDROP: {
                    if (Get_Rappeller_Count(obj) > 0) {
                        return COMMAND_AVAILABILITY_ENABLED;
                    }

                    return COMMAND_AVAILABILITY_DISABLED;
                }
                case GUI_COMMAND_SWITCH_WEAPON: {
                    const Weapon *weapon = obj->Get_Weapon_In_Weapon_Slot(button->Get_Weapon_Slot());
                    captainslog_dbgassert(weapon != nullptr,
                        "Unit %s's CommandButton %s is trying to access weaponslot %d, but doesn't have a weapon there in "
                        "its FactionUnit ini entry.",
                        obj->Get_Template()->Get_Name().Str(),
                        button->Get_Name().Str(),
                        button->Get_Weapon_Slot());

                    if (weapon != nullptr) {
                        const std::list<Drawable *> *drawables = g_theInGameUI->Get_All_Selected_Drawables();

                        for (auto it = drawables->begin(); it != drawables->end(); it++) {
                            Drawable *drawable = *it;

                            if (drawable != nullptr) {
                                if (drawable->Get_Object() != nullptr) {
                                    if (drawable->Get_Object()->Is_Locally_Controlled()) {
                                        if (drawable->Get_Object()->Get_Current_Weapon(nullptr) != nullptr) {
                                            if (drawable->Get_Object()->Get_Current_Weapon(nullptr)->Get_Weapon_Slot()
                                                != button->Get_Weapon_Slot()) {
                                                return COMMAND_AVAILABILITY_ENABLED;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        return COMMAND_AVAILABILITY_ENABLED_AND_ACTIVE;
                    } else {
                        return COMMAND_AVAILABILITY_DISABLED;
                    }
                }
                case GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE: {
                    return COMMAND_AVAILABILITY_ENABLED;
                }
                default: {
                    return COMMAND_AVAILABILITY_ENABLED;
                }
            }
        }
    } else if (Get_Command_Availability(button, obj, window, window2, true) == COMMAND_AVAILABILITY_HIDDEN) {
        return COMMAND_AVAILABILITY_HIDDEN;
    } else {
        return COMMAND_AVAILABILITY_DISABLED;
    }
}
