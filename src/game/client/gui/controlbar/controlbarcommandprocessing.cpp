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
#include "audioeventrts.h"
#include "audiomanager.h"
#include "behaviormodule.h"
#include "buildassistant.h"
#include "commandxlat.h"
#include "controlbar.h"
#include "drawable.h"
#include "eva.h"
#include "gadgetpushbutton.h"
#include "messagestream.h"
#include "object.h"
#include "player.h"
#include "playerlist.h"
#include "specialpower.h"
#include "upgrade.h"

struct SelectObjectStruct
{
    const ThingTemplate *thing;
    GameMessage *message;
};

void Select_Object_Of_Type(Object *obj, void *user_data)
{
    SelectObjectStruct *data = static_cast<SelectObjectStruct *>(user_data);

    if (obj != nullptr) {
        if (data != nullptr) {
            if (obj->Get_Template()->Is_Equivalent_To(data->thing)) {
                data->message->Append_ObjectID_Arg(obj->Get_ID());
                Drawable *drawable = obj->Get_Drawable();

                if (drawable != nullptr) {
                    g_theInGameUI->Select_Drawable(drawable);
                }
            }
        }
    }
}

CBCommandStatus ControlBar::Process_Command_UI(GameWindow *control, GadgetGameMessage gadget_message)
{
    CommandButton *button = static_cast<CommandButton *>(Gadget_Button_Get_Data(control));

    if (button != nullptr) {
        if (m_currContext == CB_CONTEXT_MULTI_SELECT || button->Get_Command() == GUI_COMMAND_PURCHASE_SCIENCE
            || button->Get_Command() == GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT
            || button->Get_Command() == GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT
            || button->Get_Command() == GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE
            || (m_currentSelectedDrawable != nullptr && m_currentSelectedDrawable->Get_Object() != nullptr)) {

            if (control != nullptr) {
                if (control->Win_Get_Input_Func() == Gadget_Push_Button_Input) {
                    button->Set_Cameo_Flash_Time(0);
                    g_theControlBar->Set_Cameo_Flash(false);

                    if (button->Get_Command() != GUI_COMMAND_EXIT_CONTAINER) {
                        Gadget_Button_Set_Enabled_Image(control, button->Get_Button_Image());
                    }

                    Object *obj = nullptr;

                    if (m_currContext != CB_CONTEXT_MULTI_SELECT && button->Get_Command() != GUI_COMMAND_PURCHASE_SCIENCE
                        && button->Get_Command() != GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT
                        && button->Get_Command() != GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT
                        && button->Get_Command() != GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE) {
                        obj = m_currentSelectedDrawable->Get_Object();
                    }

                    if (obj != nullptr && (button->Get_Options() & COMMAND_OPTION_SINGLE_USE_COMMAND) != 0) {
                        obj->Set_Single_Use_Command();
                    }

                    g_theInGameUI->Place_Build_Available(nullptr, nullptr);

                    Player *player = g_thePlayerList->Get_Local_Player();

                    if (player != nullptr) {
                        AudioEventRTS sound(*button->Get_Unit_Specific_Sound());
                        sound.Set_Player_Index(player->Get_Player_Index());
                        g_theAudio->Add_Audio_Event(&sound);
                    }

                    if ((button->Get_Options()
                            & (COMMAND_OPTION_CONTEXTMODE_COMMAND | COMMAND_OPTION_NEED_TARGET_POS
                                | COMMAND_OPTION_NEED_TARGET_OBJECT))
                        != 0) {
                        if ((button->Get_Options() & COMMAND_OPTION_USES_MINE_CLEARING_WEAPONSET) != 0) {
                            g_theMessageStream->Append_Message(GameMessage::MSG_SET_MINE_CLEARING_DETAIL);
                        }

                        g_theInGameUI->Set_GUI_Command(button);
                        return CBC_COMMAND_USED;
                    } else {
                        switch (button->Get_Command()) {
                            case GUI_COMMAND_DOZER_CONSTRUCT:
                                if (m_currentSelectedDrawable != nullptr) {
                                    switch (g_theBuildAssistant->Can_Make_Unit(obj, *button->Get_Template())) {
                                        case CAN_MAKE_NOT_ENOUGH_MONEY:
                                            g_theEva->Set_Should_Play(EVA_MESSAGE_INSUFFICIENTFUNDS);
                                            g_theInGameUI->Message("GUI:NotEnoughMoneyToBuild");
                                            break;
                                        case CAN_MAKE_QUEUE_FULL:
                                            g_theInGameUI->Message("GUI:ProductionQueueFull");
                                            break;
                                        case CAN_MAKE_PARKING_FULL:
                                            g_theInGameUI->Message("GUI:ParkingPlacesFull");
                                            break;
                                        case CAN_MAKE_MAXIMUM_NUMBER:
                                            g_theInGameUI->Message("GUI:UnitMaxedOut");
                                            break;
                                        default:
                                            g_theInGameUI->Place_Build_Available(
                                                *button->Get_Template(), m_currentSelectedDrawable);
                                            break;
                                    }
                                }

                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_DOZER_CONSTRUCT_CANCEL:
                                if (obj != nullptr) {
                                    if (obj->Get_Controlling_Player() == g_thePlayerList->Get_Local_Player()) {
                                        g_theMessageStream->Append_Message(GameMessage::MSG_DOZER_CANCEL_CONSTRUCT);
                                    }
                                }

                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_UNIT_BUILD: {
                                const ThingTemplate *thing = *button->Get_Template();

                                if (obj != nullptr) {
                                    captainslog_dbgassert(thing != nullptr,
                                        "Undefined BUILD command for object '%s'",
                                        button->Get_Template()->Get_Name().Str());
                                    CanMakeType type = g_theBuildAssistant->Can_Make_Unit(obj, thing);

                                    switch (type) {
                                        case CAN_MAKE_NOT_ENOUGH_MONEY:
                                            g_theEva->Set_Should_Play(EVA_MESSAGE_INSUFFICIENTFUNDS);
                                            g_theInGameUI->Message("GUI:NotEnoughMoneyToBuild");
                                            break;
                                        case CAN_MAKE_QUEUE_FULL:
                                            g_theInGameUI->Message("GUI:ProductionQueueFull");
                                            break;
                                        case CAN_MAKE_PARKING_FULL:
                                            g_theInGameUI->Message("GUI:ParkingPlacesFull");
                                            break;
                                        case CAN_MAKE_MAXIMUM_NUMBER:
                                            g_theInGameUI->Message("GUI:UnitMaxedOut");
                                            break;
                                        default:
                                            if (type == CAN_MAKE_SUCCESS) {
                                                ProductionUpdateInterface *production =
                                                    obj->Get_Production_Update_Interface();

                                                if (production != nullptr) {
                                                    ProductionID id = production->Request_Unique_Unit_ID();
                                                    GameMessage *message = g_theMessageStream->Append_Message(
                                                        GameMessage::MSG_QUEUE_UNIT_CREATE);
                                                    message->Append_Int_Arg(thing->Get_Template_ID());
                                                    message->Append_Int_Arg(id);
                                                } else {
                                                    captainslog_dbgassert(false,
                                                        "Cannot create '%s' because the factory object '%s' is not capable "
                                                        "of producting units",
                                                        thing->Get_Name().Str(),
                                                        obj->Get_Template()->Get_Name().Str());
                                                }
                                            } else {
                                                captainslog_dbgassert(false,
                                                    "Cannot create '%s' because the factory object '%s' returns false for "
                                                    "Can_Make_Unit",
                                                    thing->Get_Name().Str(),
                                                    obj->Get_Template()->Get_Name().Str());
                                            }

                                            break;
                                    }
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_CANCEL_UNIT_BUILD: {
                                int i;

                                for (i = 0; i < QUEUE_ENTRY_COUNT && m_queueData[i].control != control; i++) {
                                }

                                if (i == 9) {
                                    captainslog_dbgassert(false, "Control not found in build queue data");
                                } else if (m_queueData[i].type == ProductionEntry::PRODUCTION_UNIT) {
                                    if (obj != nullptr) {
                                        if (obj->Get_Controlling_Player() == g_thePlayerList->Get_Local_Player()) {
                                            GameMessage *message =
                                                g_theMessageStream->Append_Message(GameMessage::MSG_CANCEL_UNIT_CREATE);
                                            message->Append_Int_Arg(m_queueData[i].production_id);
                                        }
                                    }
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_PLAYER_UPGRADE: {
                                const UpgradeTemplate *upgrade = button->Get_Upgrade_Template();
                                captainslog_dbgassert(
                                    upgrade != nullptr, "Undefined upgrade '%s' in player upgrade command", "UNKNOWN");

                                if (obj != nullptr) {
                                    if (upgrade != nullptr) {
                                        if (g_theUpgradeCenter->Can_Afford_Upgrade(
                                                g_thePlayerList->Get_Local_Player(), upgrade, true)) {
                                            ProductionUpdateInterface *production;

                                            if (obj != nullptr) {
                                                production = obj->Get_Production_Update_Interface();
                                            } else {
                                                production = nullptr;
                                            }

                                            if (production != nullptr
                                                && production->Can_Queue_Upgrade(upgrade) == CAN_MAKE_QUEUE_FULL) {
                                                g_theInGameUI->Message("GUI:ProductionQueueFull");
                                            } else {
                                                GameMessage *message =
                                                    g_theMessageStream->Append_Message(GameMessage::MSG_QUEUE_UPGRADE);
                                                message->Append_ObjectID_Arg(obj->Get_ID());
                                                message->Append_Int_Arg(upgrade->Get_Name_Key());
                                            }
                                        }
                                    }
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_OBJECT_UPGRADE: {
                                const UpgradeTemplate *upgrade = button->Get_Upgrade_Template();
                                captainslog_dbgassert(
                                    upgrade != nullptr, "Undefined upgrade '%s' in player upgrade command", "UNKNOWN");

                                if (upgrade != nullptr) {
                                    if (g_theUpgradeCenter->Can_Afford_Upgrade(
                                            g_thePlayerList->Get_Local_Player(), upgrade, true)) {
                                        ProductionUpdateInterface *production;

                                        if (obj != nullptr) {
                                            production = obj->Get_Production_Update_Interface();
                                        } else {
                                            production = nullptr;
                                        }

                                        if (production != nullptr
                                            && production->Can_Queue_Upgrade(upgrade) == CAN_MAKE_QUEUE_FULL) {
                                            g_theInGameUI->Message("GUI:ProductionQueueFull");
                                        } else {
                                            ObjectID obj_id = INVALID_OBJECT_ID;

                                            if (obj != nullptr) {
                                                obj_id = obj->Get_ID();
                                            }

                                            if (obj == nullptr
                                                || (!obj->Has_Upgrade(upgrade) && obj->Affected_By_Upgrade(upgrade))) {
                                                GameMessage *message =
                                                    g_theMessageStream->Append_Message(GameMessage::MSG_QUEUE_UPGRADE);
                                                message->Append_ObjectID_Arg(obj_id);
                                                message->Append_Int_Arg(upgrade->Get_Name_Key());
                                            }
                                        }
                                    }
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_CANCEL_UPGRADE: {
                                int i;

                                for (i = 0; i < QUEUE_ENTRY_COUNT && m_queueData[i].control != control; i++) {
                                }

                                if (i == 9) {
                                    captainslog_dbgassert(false, "Control not found in build queue data");
                                } else if (m_queueData[i].type == ProductionEntry::PRODUCTION_UPGRADE) {
                                    const UpgradeTemplate *upgrade = m_queueData[i].upgrade_template;

                                    if (upgrade != nullptr) {
                                        if (obj != nullptr) {
                                            GameMessage *message =
                                                g_theMessageStream->Append_Message(GameMessage::MSG_CANCEL_UPGRADE);
                                            message->Append_Int_Arg(upgrade->Get_Name_Key());
                                        }
                                    }
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_ATTACK_MOVE:
                                g_theMessageStream->Append_Message(GameMessage::MSG_META_TOGGLE_ATTACKMOVE);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_GUARD:
                            case GUI_COMMAND_GUARD_WITHOUT_PURSUIT:
                            case GUI_COMMAND_GUARD_FLYING_UNITS_ONLY:
                            case GUI_COMMAND_COMBATDROP:
                                captainslog_dbgassert(false, "hmm, should never occur");
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_STOP:
                                g_theMessageStream->Append_Message(GameMessage::MSG_DO_STOP);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_WAYPOINTS:
                            case GUI_COMMAND_BEACON_DELETE:
                            case GUI_COMMAND_SET_RALLY_POINT:
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_EXIT_CONTAINER: {
                                ObjectID obj_id;
                                int i;

                                for (i = 0; i < CommandSet::MAX_COMMAND_BUTTONS; i++) {
                                    if (s_containData[i].button == control) {
                                        obj_id = s_containData[i].id;
                                    }
                                }

                                Object *object = g_theGameLogic->Find_Object_By_ID(obj_id);

                                if (object != nullptr) {
                                    GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_EXIT);
                                    message->Append_ObjectID_Arg(object->Get_ID());
                                } else {
                                    s_containData[i].button = nullptr;
                                    s_containData[i].id = INVALID_OBJECT_ID;
                                }
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_EVACUATE:
                                g_theInGameUI->Set_GUI_Command(nullptr);
                                if ((button->Get_Options() & COMMAND_OPTION_NEED_TARGET_POS) == 0) {
                                    Pick_And_Play_Unit_Voice_Response(
                                        g_theInGameUI->Get_All_Selected_Drawables(), GameMessage::MSG_EVACUATE, nullptr);
                                    g_theMessageStream->Append_Message(GameMessage::MSG_EVACUATE);
                                }
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_EXECUTE_RAILED_TRANSPORT:
                                g_theMessageStream->Append_Message(GameMessage::MSG_EXECUTE_RAILED_TRANSPORT);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_SELL:
                                g_theMessageStream->Append_Message(GameMessage::MSG_SELL);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_FIRE_WEAPON: {
                                GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_DO_WEAPON);
                                message->Append_Int_Arg(button->Get_Weapon_Slot());
                                message->Append_Int_Arg(button->Get_Max_Shots_To_Fire());
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_SPECIAL_POWER: {
                                GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_DO_SPECIAL_POWER);
                                message->Append_Int_Arg(button->Get_Special_Power()->Get_ID());
                                message->Append_Int_Arg(button->Get_Options());
                                message->Append_ObjectID_Arg(INVALID_OBJECT_ID);
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_PURCHASE_SCIENCE: {
                                ScienceType science = SCIENCE_INVALID;
                                Player *local_player = g_thePlayerList->Get_Local_Player();

                                for (unsigned int i = 0; i < button->Get_Sciences()->size(); i++) {
                                    science = (*button->Get_Sciences())[i];

                                    if (!local_player->Has_Science(science)) {
                                        if (g_theScienceStore->Player_Has_Prereqs_For_Science(local_player, science)) {
                                            if (g_theScienceStore->Get_Science_Purchase_Cost(science)
                                                <= local_player->Get_Science_Purchase_Points()) {
                                                break;
                                            }
                                        }
                                    }
                                }

                                if (science == SCIENCE_INVALID) {
                                    Switch_To_Context(CB_CONTEXT_NONE, nullptr);
                                } else {
                                    GameMessage *message =
                                        g_theMessageStream->Append_Message(GameMessage::MSG_PURCHASE_SCIENCE);
                                    message->Append_Int_Arg(science);
                                    Mark_UI_Dirty();
                                }

                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_HACK_INTERNET:
                                Pick_And_Play_Unit_Voice_Response(
                                    g_theInGameUI->Get_All_Selected_Drawables(), GameMessage::MSG_INTERNET_HACK, nullptr);
                                g_theMessageStream->Append_Message(GameMessage::MSG_INTERNET_HACK);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_TOGGLE_OVERCHARGE:
                                g_theMessageStream->Append_Message(GameMessage::MSG_TOGGLE_OVERCHARGE);
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_SWITCH_WEAPON: {
                                GameMessage *message = g_theMessageStream->Append_Message(GameMessage::MSG_SWITCH_WEAPONS);
                                PickAndPlayInfo info;
                                WeaponSlotType type = button->Get_Weapon_Slot();
                                info.weapon_slot_type = &type;
                                Pick_And_Play_Unit_Voice_Response(
                                    g_theInGameUI->Get_All_Selected_Drawables(), GameMessage::MSG_SWITCH_WEAPONS, &info);
                                message->Append_Int_Arg(button->Get_Weapon_Slot());
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_SPECIAL_POWER_FROM_SHORTCUT: {
                                const SpecialPowerTemplate *power = button->Get_Special_Power();
                                Object *object =
                                    g_thePlayerList->Get_Local_Player()->Find_Most_Ready_Shortcut_Special_Power_Of_Type(
                                        power->Get_Type());

                                if (object != nullptr) {
                                    GameMessage *message =
                                        g_theMessageStream->Append_Message(GameMessage::MSG_DO_SPECIAL_POWER);
                                    message->Append_Int_Arg(power->Get_ID());
                                    message->Append_Int_Arg(button->Get_Options());
                                    message->Append_ObjectID_Arg(object->Get_ID());
                                }
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_SPECIAL_POWER_CONSTRUCT:
                                if (m_currentSelectedDrawable != nullptr) {
                                    switch (g_theBuildAssistant->Can_Make_Unit(obj, *button->Get_Template())) {
                                        case CAN_MAKE_NOT_ENOUGH_MONEY:
                                            g_theEva->Set_Should_Play(EVA_MESSAGE_INSUFFICIENTFUNDS);
                                            g_theInGameUI->Message("GUI:NotEnoughMoneyToBuild");
                                            break;
                                        case CAN_MAKE_QUEUE_FULL:
                                            g_theInGameUI->Message("GUI:ProductionQueueFull");
                                            break;
                                        case CAN_MAKE_PARKING_FULL:
                                            g_theInGameUI->Message("GUI:ParkingPlacesFull");
                                            break;
                                        case CAN_MAKE_MAXIMUM_NUMBER:
                                            g_theInGameUI->Message("GUI:UnitMaxedOut");
                                            break;
                                        default:
                                            g_theInGameUI->Place_Build_Available(
                                                *button->Get_Template(), m_currentSelectedDrawable);

                                            ProductionUpdateInterface *production = obj->Get_Production_Update_Interface();

                                            if (production != nullptr) {
                                                production->Set_Special_Power_Construction_Command_Button(button);
                                            }
                                            break;
                                    }
                                }
                                return CBC_COMMAND_USED;
                            case GUI_COMMAND_SPECIAL_POWER_CONSTRUCT_FROM_SHORTCUT: {
                                const SpecialPowerTemplate *power = button->Get_Special_Power();
                                Object *object =
                                    g_thePlayerList->Get_Local_Player()->Find_Most_Ready_Shortcut_Special_Power_Of_Type(
                                        power->Get_Type());

                                if (object != nullptr) {
                                    Drawable *drawable = object->Get_Drawable();

                                    switch (g_theBuildAssistant->Can_Make_Unit(object, *button->Get_Template())) {
                                        case CAN_MAKE_NOT_ENOUGH_MONEY:
                                            g_theEva->Set_Should_Play(EVA_MESSAGE_INSUFFICIENTFUNDS);
                                            g_theInGameUI->Message("GUI:NotEnoughMoneyToBuild");
                                            break;
                                        case CAN_MAKE_QUEUE_FULL:
                                            g_theInGameUI->Message("GUI:ProductionQueueFull");
                                            break;
                                        case CAN_MAKE_PARKING_FULL:
                                            g_theInGameUI->Message("GUI:ParkingPlacesFull");
                                            break;
                                        case CAN_MAKE_MAXIMUM_NUMBER:
                                            g_theInGameUI->Message("GUI:UnitMaxedOut");
                                            break;
                                        default:
                                            g_theInGameUI->Place_Build_Available(*button->Get_Template(), drawable);

                                            ProductionUpdateInterface *production =
                                                object->Get_Production_Update_Interface();

                                            if (production != nullptr) {
                                                production->Set_Special_Power_Construction_Command_Button(button);
                                            }
                                            break;
                                    }
                                }
                                return CBC_COMMAND_USED;
                            }
                            case GUI_COMMAND_SELECT_ALL_UNITS_OF_TYPE: {
                                Player *local_player = g_thePlayerList->Get_Local_Player();

                                if (local_player != nullptr) {
                                    const ThingTemplate *thing = *button->Get_Template();

                                    if (thing != nullptr) {
                                        g_theInGameUI->Deselect_All_Drawables(true);
                                        GameMessage *message =
                                            g_theMessageStream->Append_Message(GameMessage::MSG_CREATE_SELECTED_GROUP);
                                        message->Append_Bool_Arg(true);
                                        SelectObjectStruct select;
                                        select.thing = thing;
                                        select.message = message;
                                        local_player->Iterate_Objects(Select_Object_Of_Type, &select);
                                    }
                                }
                                return CBC_COMMAND_USED;
                            }
                            default:
                                captainslog_dbgassert(false, "Unknown command '%d'", button->Get_Command());
                                return CBC_COMMAND_NOT_USED;
                        }
                    }
                } else {
                    return CBC_COMMAND_NOT_USED;
                }
            } else {
                return CBC_COMMAND_NOT_USED;
            }
        } else {
            if (m_currContext != CB_CONTEXT_NONE) {
                Switch_To_Context(CB_CONTEXT_NONE, nullptr);
            }

            return CBC_COMMAND_NOT_USED;
        }
    } else {
        captainslog_dbgassert(false, "ControlBar::Process_Command_UI() -- Button activated has no data. Ignoring...");
        return CBC_COMMAND_NOT_USED;
    }
}

CBCommandStatus ControlBar::Process_Command_Transition_UI(GameWindow *control, GadgetGameMessage gadget_message)
{
    if (m_currContext == CB_CONTEXT_MULTI_SELECT
        || (m_currentSelectedDrawable != nullptr && m_currentSelectedDrawable->Get_Object())) {
        return CBC_COMMAND_USED;
    }

    if (m_currContext != CB_CONTEXT_NONE && m_currContext != CB_CONTEXT_UNK && m_currContext != CB_CONTEXT_OBSERVER) {
        Switch_To_Context(CB_CONTEXT_NONE, nullptr);
    }

    return CBC_COMMAND_NOT_USED;
}
