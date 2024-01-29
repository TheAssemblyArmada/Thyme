/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Message object.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamemessage.h"
#include "gamemessagelist.h"
#include "playerlist.h"

GameMessage::GameMessage(MessageType type) :
    m_next(nullptr),
    m_prev(nullptr),
    m_list(nullptr),
    m_type(type),
    m_playerIndex(g_thePlayerList->Get_Local_Player()->Get_Player_Index()), // g_thePlayerList->m_local->m_playerIndex
    m_argCount(0),
    m_argList(nullptr),
    m_argTail(nullptr)
{
}

GameMessage::~GameMessage()
{
    GameMessageArgument *argobj = m_argList;

    while (argobj != nullptr) {
        GameMessageArgument *tmp = argobj;
        argobj = argobj->m_next;
        tmp->Delete_Instance();
    }

    if (m_list != nullptr) {
        m_list->Remove_Message(this);
    }
}

GameMessageArgument *GameMessage::Allocate_Arg()
{
    GameMessageArgument *arg = NEW_POOL_OBJ(GameMessageArgument);

    if (m_argTail != nullptr) {
        m_argTail->m_next = arg;
    } else {
        m_argList = arg;
    }

    arg->m_next = nullptr;
    m_argTail = arg;
    ++m_argCount;

    return arg;
}

ArgumentType *GameMessage::Get_Argument(int arg) const
{
    static ArgumentType junkconst;

    GameMessageArgument *argobj = m_argList;
    int i = 0;

    while (argobj != nullptr) {
        if (i == arg) {
            return &argobj->m_data;
        }

        ++i;
        argobj = argobj->m_next;
    }

    return &junkconst;
}

ArgumentDataType GameMessage::Get_Argument_Type(int arg)
{
    if (arg >= m_argCount) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    GameMessageArgument *argobj = m_argList;

    for (int i = 0; i < arg; ++i) {
        if (argobj == nullptr) {
            return ARGUMENTDATATYPE_UNKNOWN;
        }

        argobj = argobj->m_next;
    }

    if (argobj == nullptr) {
        return ARGUMENTDATATYPE_UNKNOWN;
    }

    return argobj->m_type;
}

Utf8String GameMessage::Get_Command_As_Ascii(MessageType command)
{
    switch (command) {
        case MSG_INVALID:
            return "MSG_INVALID";
        case MSG_FRAME_TICK:
            return "MSG_FRAME_TICK";
        case MSG_RAW_MOUSE_BEGIN:
            return "MSG_RAW_MOUSE_BEGIN";
        case MSG_RAW_MOUSE_POSITION:
            return "MSG_RAW_MOUSE_POSITION";
        case MSG_RAW_MOUSE_LEFT_BUTTON_DOWN:
            return "MSG_RAW_MOUSE_LEFT_BUTTON_DOWN";
        case MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK:
            return "MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK";
        case MSG_RAW_MOUSE_LEFT_BUTTON_UP:
            return "MSG_RAW_MOUSE_LEFT_BUTTON_UP";
        case MSG_RAW_MOUSE_LEFT_CLICK:
            return "MSG_RAW_MOUSE_LEFT_CLICK";
        case MSG_RAW_MOUSE_LEFT_DRAG:
            return "MSG_RAW_MOUSE_LEFT_DRAG";
        case MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN:
            return "MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN";
        case MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK:
            return "MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK";
        case MSG_RAW_MOUSE_MIDDLE_BUTTON_UP:
            return "MSG_RAW_MOUSE_MIDDLE_BUTTON_UP";
        case MSG_RAW_MOUSE_MIDDLE_DRAG:
            return "MSG_RAW_MOUSE_MIDDLE_DRAG";
        case MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN:
            return "MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN";
        case MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK:
            return "MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK";
        case MSG_RAW_MOUSE_RIGHT_BUTTON_UP:
            return "MSG_RAW_MOUSE_RIGHT_BUTTON_UP";
        case MSG_RAW_MOUSE_RIGHT_DRAG:
            return "MSG_RAW_MOUSE_RIGHT_DRAG";
        case MSG_RAW_MOUSE_WHEEL:
            return "MSG_RAW_MOUSE_WHEEL";
        case MSG_RAW_MOUSE_END:
            return "MSG_RAW_MOUSE_END";
        case MSG_RAW_KEY_DOWN:
            return "MSG_RAW_KEY_DOWN";
        case MSG_RAW_KEY_UP:
            return "MSG_RAW_KEY_UP";
        case MSG_MOUSE_LEFT_CLICK:
            return "MSG_MOUSE_LEFT_CLICK";
        case MSG_MOUSE_LEFT_DOUBLE_CLICK:
            return "MSG_MOUSE_LEFT_DOUBLE_CLICK";
        case MSG_MOUSE_MIDDLE_CLICK:
            return "MSG_MOUSE_MIDDLE_CLICK";
        case MSG_MOUSE_MIDDLE_DOUBLE_CLICK:
            return "MSG_MOUSE_MIDDLE_DOUBLE_CLICK";
        case MSG_MOUSE_RIGHT_CLICK:
            return "MSG_MOUSE_RIGHT_CLICK";
        case MSG_MOUSE_RIGHT_DOUBLE_CLICK:
            return "MSG_MOUSE_RIGHT_DOUBLE_CLICK";
        case MSG_CLEAR_GAME_DATA:
            return "MSG_CLEAR_GAME_DATA";
        case MSG_NEW_GAME:
            return "MSG_NEW_GAME";
        case MSG_BEGIN_META_MESSAGES:
            return "MSG_BEGIN_META_MESSAGES";
        case MSG_META_SAVE_VIEW1:
            return "MSG_META_SAVE_VIEW1";
        case MSG_META_SAVE_VIEW2:
            return "MSG_META_SAVE_VIEW2";
        case MSG_META_SAVE_VIEW3:
            return "MSG_META_SAVE_VIEW3";
        case MSG_META_SAVE_VIEW4:
            return "MSG_META_SAVE_VIEW4";
        case MSG_META_SAVE_VIEW5:
            return "MSG_META_SAVE_VIEW5";
        case MSG_META_SAVE_VIEW6:
            return "MSG_META_SAVE_VIEW6";
        case MSG_META_SAVE_VIEW7:
            return "MSG_META_SAVE_VIEW7";
        case MSG_META_SAVE_VIEW8:
            return "MSG_META_SAVE_VIEW8";
        case MSG_META_VIEW_VIEW1:
            return "MSG_META_VIEW_VIEW1";
        case MSG_META_VIEW_VIEW2:
            return "MSG_META_VIEW_VIEW2";
        case MSG_META_VIEW_VIEW3:
            return "MSG_META_VIEW_VIEW3";
        case MSG_META_VIEW_VIEW4:
            return "MSG_META_VIEW_VIEW4";
        case MSG_META_VIEW_VIEW5:
            return "MSG_META_VIEW_VIEW5";
        case MSG_META_VIEW_VIEW6:
            return "MSG_META_VIEW_VIEW6";
        case MSG_META_VIEW_VIEW7:
            return "MSG_META_VIEW_VIEW7";
        case MSG_META_VIEW_VIEW8:
            return "MSG_META_VIEW_VIEW8";
        case MSG_META_CREATE_TEAM0:
            return "MSG_META_CREATE_TEAM0";
        case MSG_META_CREATE_TEAM1:
            return "MSG_META_CREATE_TEAM1";
        case MSG_META_CREATE_TEAM2:
            return "MSG_META_CREATE_TEAM2";
        case MSG_META_CREATE_TEAM3:
            return "MSG_META_CREATE_TEAM3";
        case MSG_META_CREATE_TEAM4:
            return "MSG_META_CREATE_TEAM4";
        case MSG_META_CREATE_TEAM5:
            return "MSG_META_CREATE_TEAM5";
        case MSG_META_CREATE_TEAM6:
            return "MSG_META_CREATE_TEAM6";
        case MSG_META_CREATE_TEAM7:
            return "MSG_META_CREATE_TEAM7";
        case MSG_META_CREATE_TEAM8:
            return "MSG_META_CREATE_TEAM8";
        case MSG_META_CREATE_TEAM9:
            return "MSG_META_CREATE_TEAM9";
        case MSG_META_SELECT_TEAM0:
            return "MSG_META_SELECT_TEAM0";
        case MSG_META_SELECT_TEAM1:
            return "MSG_META_SELECT_TEAM1";
        case MSG_META_SELECT_TEAM2:
            return "MSG_META_SELECT_TEAM2";
        case MSG_META_SELECT_TEAM3:
            return "MSG_META_SELECT_TEAM3";
        case MSG_META_SELECT_TEAM4:
            return "MSG_META_SELECT_TEAM4";
        case MSG_META_SELECT_TEAM5:
            return "MSG_META_SELECT_TEAM5";
        case MSG_META_SELECT_TEAM6:
            return "MSG_META_SELECT_TEAM6";
        case MSG_META_SELECT_TEAM7:
            return "MSG_META_SELECT_TEAM7";
        case MSG_META_SELECT_TEAM8:
            return "MSG_META_SELECT_TEAM8";
        case MSG_META_SELECT_TEAM9:
            return "MSG_META_SELECT_TEAM9";
        case MSG_META_ADD_TEAM0:
            return "MSG_META_ADD_TEAM0";
        case MSG_META_ADD_TEAM1:
            return "MSG_META_ADD_TEAM1";
        case MSG_META_ADD_TEAM2:
            return "MSG_META_ADD_TEAM2";
        case MSG_META_ADD_TEAM3:
            return "MSG_META_ADD_TEAM3";
        case MSG_META_ADD_TEAM4:
            return "MSG_META_ADD_TEAM4";
        case MSG_META_ADD_TEAM5:
            return "MSG_META_ADD_TEAM5";
        case MSG_META_ADD_TEAM6:
            return "MSG_META_ADD_TEAM6";
        case MSG_META_ADD_TEAM7:
            return "MSG_META_ADD_TEAM7";
        case MSG_META_ADD_TEAM8:
            return "MSG_META_ADD_TEAM8";
        case MSG_META_ADD_TEAM9:
            return "MSG_META_ADD_TEAM9";
        case MSG_META_VIEW_TEAM0:
            return "MSG_META_VIEW_TEAM0";
        case MSG_META_VIEW_TEAM1:
            return "MSG_META_VIEW_TEAM1";
        case MSG_META_VIEW_TEAM2:
            return "MSG_META_VIEW_TEAM2";
        case MSG_META_VIEW_TEAM3:
            return "MSG_META_VIEW_TEAM3";
        case MSG_META_VIEW_TEAM4:
            return "MSG_META_VIEW_TEAM4";
        case MSG_META_VIEW_TEAM5:
            return "MSG_META_VIEW_TEAM5";
        case MSG_META_VIEW_TEAM6:
            return "MSG_META_VIEW_TEAM6";
        case MSG_META_VIEW_TEAM7:
            return "MSG_META_VIEW_TEAM7";
        case MSG_META_VIEW_TEAM8:
            return "MSG_META_VIEW_TEAM8";
        case MSG_META_VIEW_TEAM9:
            return "MSG_META_VIEW_TEAM9";
        case MSG_META_SELECT_MATCHING_UNITS:
            return "MSG_META_SELECT_MATCHING_UNITS";
        case MSG_META_SELECT_NEXT_UNIT:
            return "MSG_META_SELECT_NEXT_UNIT";
        case MSG_META_SELECT_PREV_UNIT:
            return "MSG_META_SELECT_PREV_UNIT";
        case MSG_META_SELECT_NEXT_WORKER:
            return "MSG_META_SELECT_NEXT_WORKER";
        case MSG_META_SELECT_PREV_WORKER:
            return "MSG_META_SELECT_PREV_WORKER";
        case MSG_META_VIEW_COMMAND_CENTER:
            return "MSG_META_VIEW_COMMAND_CENTER";
        case MSG_META_VIEW_LAST_RADAR_EVENT:
            return "MSG_META_VIEW_LAST_RADAR_EVENT";
        case MSG_META_SELECT_HERO:
            return "MSG_META_SELECT_HERO";
        case MSG_META_SELECT_ALL:
            return "MSG_META_SELECT_ALL";
        case MSG_META_SELECT_ALL_AIRCRAFT:
            return "MSG_META_SELECT_ALL_AIRCRAFT";
        case MSG_META_SCATTER:
            return "MSG_META_SCATTER";
        case MSG_META_STOP:
            return "MSG_META_STOP";
        case MSG_META_DEPLOY:
            return "MSG_META_DEPLOY";
        case MSG_META_CREATE_FORMATION:
            return "MSG_META_CREATE_FORMATION";
        case MSG_META_FOLLOW:
            return "MSG_META_FOLLOW";
        case MSG_META_CHAT_PLAYERS:
            return "MSG_META_CHAT_PLAYERS";
        case MSG_META_CHAT_ALLIES:
            return "MSG_META_CHAT_ALLIES";
        case MSG_META_CHAT_EVERYONE:
            return "MSG_META_CHAT_EVERYONE";
        case MSG_META_DIPLOMACY:
            return "MSG_META_DIPLOMACY";
        case MSG_META_OPTIONS:
            return "MSG_META_OPTIONS";
        case MSG_META_TOGGLE_LOWER_DETAILS:
            return "MSG_META_TOGGLE_LOWER_DETAILS";
        case MSG_META_TOGGLE_CONTROL_BAR:
            return "MSG_META_TOGGLE_CONTROL_BAR";
        case MSG_META_BEGIN_PATH_BUILD:
            return "MSG_META_BEGIN_PATH_BUILD";
        case MSG_META_END_PATH_BUILD:
            return "MSG_META_END_PATH_BUILD";
        case MSG_META_BEGIN_FORCEATTACK:
            return "MSG_META_BEGIN_FORCEATTACK";
        case MSG_META_END_FORCEATTACK:
            return "MSG_META_END_FORCEATTACK";
        case MSG_META_BEGIN_FORCEMOVE:
            return "MSG_META_BEGIN_FORCEMOVE";
        case MSG_META_END_FORCEMOVE:
            return "MSG_META_END_FORCEMOVE";
        case MSG_META_BEGIN_WAYPOINTS:
            return "MSG_META_BEGIN_WAYPOINTS";
        case MSG_META_END_WAYPOINTS:
            return "MSG_META_END_WAYPOINTS";
        case MSG_META_BEGIN_PREFER_SELECTION:
            return "MSG_META_BEGIN_PREFER_SELECTION";
        case MSG_META_END_PREFER_SELECTION:
            return "MSG_META_END_PREFER_SELECTION";
        case MSG_META_TAKE_SCREENSHOT:
            return "MSG_META_TAKE_SCREENSHOT";
        case MSG_META_ALL_CHEER:
            return "MSG_META_ALL_CHEER";
        case MSG_META_TOGGLE_ATTACKMOVE:
            return "MSG_META_TOGGLE_ATTACKMOVE";
        case MSG_META_BEGIN_CAMERA_ROTATE_LEFT:
            return "MSG_META_BEGIN_CAMERA_ROTATE_LEFT";
        case MSG_META_END_CAMERA_ROTATE_LEFT:
            return "MSG_META_END_CAMERA_ROTATE_LEFT";
        case MSG_META_BEGIN_CAMERA_ROTATE_RIGHT:
            return "MSG_META_BEGIN_CAMERA_ROTATE_RIGHT";
        case MSG_META_END_CAMERA_ROTATE_RIGHT:
            return "MSG_META_END_CAMERA_ROTATE_RIGHT";
        case MSG_META_BEGIN_CAMERA_ZOOM_IN:
            return "MSG_META_BEGIN_CAMERA_ZOOM_IN";
        case MSG_META_END_CAMERA_ZOOM_IN:
            return "MSG_META_END_CAMERA_ZOOM_IN";
        case MSG_META_BEGIN_CAMERA_ZOOM_OUT:
            return "MSG_META_BEGIN_CAMERA_ZOOM_OUT";
        case MSG_META_END_CAMERA_ZOOM_OUT:
            return "MSG_META_END_CAMERA_ZOOM_OUT";
        case MSG_META_CAMERA_RESET:
            return "MSG_META_CAMERA_RESET";
        case MSG_META_TOGGLE_CAMERA_TRACKING_DRAWABLE:
            return "MSG_META_TOGGLE_CAMERA_TRACKING_DRAWABLE";
        case MSG_META_TOGGLE_FAST_FORWARD_REPLAY:
            return "MSG_META_TOGGLE_FAST_FORWARD_REPLAY";
        case MSG_META_DEMO_INSTANT_QUIT:
            return "MSG_META_DEMO_INSTANT_QUIT";
        case MSG_META_PLACE_BEACON:
            return "MSG_META_PLACE_BEACON";
        case MSG_META_REMOVE_BEACON:
            return "MSG_META_REMOVE_BEACON";
        case MSG_END_META_MESSAGES:
            return "MSG_END_META_MESSAGES";
        case MSG_MOUSEOVER_DRAWABLE_HINT:
            return "MSG_MOUSEOVER_DRAWABLE_HINT";
        case MSG_MOUSEOVER_LOCATION_HINT:
            return "MSG_MOUSEOVER_LOCATION_HINT";
        case MSG_VALID_GUICOMMAND_HINT:
            return "MSG_VALID_GUICOMMAND_HINT";
        case MSG_INVALID_GUICOMMAND_HINT:
            return "MSG_INVALID_GUICOMMAND_HINT";
        case MSG_AREA_SELECTION_HINT:
            return "MSG_AREA_SELECTION_HINT";
        case MSG_DO_ATTACK_OBJECT_HINT:
            return "MSG_DO_ATTACK_OBJECT_HINT";
        case MSG_DO_FORCE_ATTACK_OBJECT_HINT:
            return "MSG_DO_FORCE_ATTACK_OBJECT_HINT";
        case MSG_DO_FORCE_ATTACK_GROUND_HINT:
            return "MSG_DO_FORCE_ATTACK_GROUND_HINT";
        case MSG_GET_REPAIRED_HINT:
            return "MSG_GET_REPAIRED_HINT";
        case MSG_GET_HEALED_HINT:
            return "MSG_GET_HEALED_HINT";
        case MSG_DO_REPAIR_HINT:
            return "MSG_DO_REPAIR_HINT";
        case MSG_RESUME_CONSTRUCTION_HINT:
            return "MSG_RESUME_CONSTRUCTION_HINT";
        case MSG_ENTER_HINT:
            return "MSG_ENTER_HINT";
        case MSG_DOCK_HINT:
            return "MSG_DOCK_HINT";
        case MSG_DO_MOVETO_HINT:
            return "MSG_DO_MOVETO_HINT";
        case MSG_DO_ATTACKMOVETO_HINT:
            return "MSG_DO_ATTACKMOVETO_HINT";
        case MSG_ADD_WAYPOINT_HINT:
            return "MSG_ADD_WAYPOINT_HINT";
        case MSG_HIJACK_HINT:
            return "MSG_HIJACK_HINT";
        case MSG_SABOTAGE_HINT:
            return "MSG_SABOTAGE_HINT";
        case MSG_FIREBOMB_HINT:
            return "MSG_FIREBOMB_HINT";
        case MSG_CONVERT_TO_CARBOMB_HINT:
            return "MSG_CONVERT_TO_CARBOMB_HINT";
        case MSG_CAPTUREBUILDING_HINT:
            return "MSG_CAPTUREBUILDING_HINT";
        case MSG_SNIPE_VEHICLE_HINT:
            return "MSG_SNIPE_VEHICLE_HINT";
        case MSG_DEFECTOR_HINT:
            return "MSG_DEFECTOR_HINT";
        case MSG_SET_RALLY_POINT_HINT:
            return "MSG_SET_RALLY_POINT_HINT";
        case MSG_DO_SALVAGE_HINT:
            return "MSG_DO_SALVAGE_HINT";
        case MSG_DO_INVALID_HINT:
            return "MSG_DO_INVALID_HINT";
        case MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT:
            return "MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT";
        case MSG_HACK_HINT:
            return "MSG_HACK_HINT";
        case MSG_BEGIN_NETWORK_MESSAGES:
            return "MSG_BEGIN_NETWORK_MESSAGES";
        case MSG_CREATE_SELECTED_GROUP:
            return "MSG_CREATE_SELECTED_GROUP";
        case MSG_CREATE_SELECTED_GROUP_NO_SOUND:
            return "MSG_CREATE_SELECTED_GROUP_NO_SOUND";
        case MSG_DESTROY_SELECTED_GROUP:
            return "MSG_DESTROY_SELECTED_GROUP";
        case MSG_REMOVE_FROM_SELECTED_GROUP:
            return "MSG_REMOVE_FROM_SELECTED_GROUP";
        case MSG_SELECTED_GROUP_COMMAND:
            return "MSG_SELECTED_GROUP_COMMAND";
        case MSG_CREATE_TEAM0:
            return "MSG_CREATE_TEAM0";
        case MSG_CREATE_TEAM1:
            return "MSG_CREATE_TEAM1";
        case MSG_CREATE_TEAM2:
            return "MSG_CREATE_TEAM2";
        case MSG_CREATE_TEAM3:
            return "MSG_CREATE_TEAM3";
        case MSG_CREATE_TEAM4:
            return "MSG_CREATE_TEAM4";
        case MSG_CREATE_TEAM5:
            return "MSG_CREATE_TEAM5";
        case MSG_CREATE_TEAM6:
            return "MSG_CREATE_TEAM6";
        case MSG_CREATE_TEAM7:
            return "MSG_CREATE_TEAM7";
        case MSG_CREATE_TEAM8:
            return "MSG_CREATE_TEAM8";
        case MSG_CREATE_TEAM9:
            return "MSG_CREATE_TEAM9";
        case MSG_SELECT_TEAM0:
            return "MSG_SELECT_TEAM0";
        case MSG_SELECT_TEAM1:
            return "MSG_SELECT_TEAM1";
        case MSG_SELECT_TEAM2:
            return "MSG_SELECT_TEAM2";
        case MSG_SELECT_TEAM3:
            return "MSG_SELECT_TEAM3";
        case MSG_SELECT_TEAM4:
            return "MSG_SELECT_TEAM4";
        case MSG_SELECT_TEAM5:
            return "MSG_SELECT_TEAM5";
        case MSG_SELECT_TEAM6:
            return "MSG_SELECT_TEAM6";
        case MSG_SELECT_TEAM7:
            return "MSG_SELECT_TEAM7";
        case MSG_SELECT_TEAM8:
            return "MSG_SELECT_TEAM8";
        case MSG_SELECT_TEAM9:
            return "MSG_SELECT_TEAM9";
        case MSG_ADD_TEAM0:
            return "MSG_ADD_TEAM0";
        case MSG_ADD_TEAM1:
            return "MSG_ADD_TEAM1";
        case MSG_ADD_TEAM2:
            return "MSG_ADD_TEAM2";
        case MSG_ADD_TEAM3:
            return "MSG_ADD_TEAM3";
        case MSG_ADD_TEAM4:
            return "MSG_ADD_TEAM4";
        case MSG_ADD_TEAM5:
            return "MSG_ADD_TEAM5";
        case MSG_ADD_TEAM6:
            return "MSG_ADD_TEAM6";
        case MSG_ADD_TEAM7:
            return "MSG_ADD_TEAM7";
        case MSG_ADD_TEAM8:
            return "MSG_ADD_TEAM8";
        case MSG_ADD_TEAM9:
            return "MSG_ADD_TEAM9";
        case MSG_DO_ATTACKSQUAD:
            return "MSG_DO_ATTACKSQUAD";
        case MSG_DO_WEAPON:
            return "MSG_DO_WEAPON";
        case MSG_DO_WEAPON_AT_LOCATION:
            return "MSG_DO_WEAPON_AT_LOCATION";
        case MSG_DO_WEAPON_AT_OBJECT:
            return "MSG_DO_WEAPON_AT_OBJECT";
        case MSG_DO_SPECIAL_POWER:
            return "MSG_DO_SPECIAL_POWER";
        case MSG_DO_SPECIAL_POWER_AT_LOCATION:
            return "MSG_DO_SPECIAL_POWER_AT_LOCATION";
        case MSG_DO_SPECIAL_POWER_AT_OBJECT:
            return "MSG_DO_SPECIAL_POWER_AT_OBJECT";
        case MSG_SET_RALLY_POINT:
            return "MSG_SET_RALLY_POINT";
        case MSG_PURCHASE_SCIENCE:
            return "MSG_PURCHASE_SCIENCE";
        case MSG_QUEUE_UPGRADE:
            return "MSG_QUEUE_UPGRADE";
        case MSG_CANCEL_UPGRADE:
            return "MSG_CANCEL_UPGRADE";
        case MSG_QUEUE_UNIT_CREATE:
            return "MSG_QUEUE_UNIT_CREATE";
        case MSG_CANCEL_UNIT_CREATE:
            return "MSG_CANCEL_UNIT_CREATE";
        case MSG_DOZER_CONSTRUCT:
            return "MSG_DOZER_CONSTRUCT";
        case MSG_DOZER_CONSTRUCT_LINE:
            return "MSG_DOZER_CONSTRUCT_LINE";
        case MSG_DOZER_CANCEL_CONSTRUCT:
            return "MSG_DOZER_CANCEL_CONSTRUCT";
        case MSG_SELL:
            return "MSG_SELL";
        case MSG_EXIT:
            return "MSG_EXIT";
        case MSG_EVACUATE:
            return "MSG_EVACUATE";
        case MSG_EXECUTE_RAILED_TRANSPORT:
            return "MSG_EXECUTE_RAILED_TRANSPORT";
        case MSG_COMBATDROP_AT_LOCATION:
            return "MSG_COMBATDROP_AT_LOCATION";
        case MSG_COMBATDROP_AT_OBJECT:
            return "MSG_COMBATDROP_AT_OBJECT";
        case MSG_AREA_SELECTION:
            return "MSG_AREA_SELECTION";
        case MSG_DO_ATTACK_OBJECT:
            return "MSG_DO_ATTACK_OBJECT";
        case MSG_DO_FORCE_ATTACK_OBJECT:
            return "MSG_DO_FORCE_ATTACK_OBJECT";
        case MSG_DO_FORCE_ATTACK_GROUND:
            return "MSG_DO_FORCE_ATTACK_GROUND";
        case MSG_GET_REPAIRED:
            return "MSG_GET_REPAIRED";
        case MSG_GET_HEALED:
            return "MSG_GET_HEALED";
        case MSG_DO_REPAIR:
            return "MSG_DO_REPAIR";
        case MSG_RESUME_CONSTRUCTION:
            return "MSG_RESUME_CONSTRUCTION";
        case MSG_ENTER:
            return "MSG_ENTER";
        case MSG_DOCK:
            return "MSG_DOCK";
        case MSG_DO_MOVETO:
            return "MSG_DO_MOVETO";
        case MSG_DO_ATTACKMOVETO:
            return "MSG_DO_ATTACKMOVETO";
        case MSG_DO_FORCEMOVETO:
            return "MSG_DO_FORCEMOVETO";
        case MSG_ADD_WAYPOINT:
            return "MSG_ADD_WAYPOINT";
        case MSG_DO_GUARD_POSITION:
            return "MSG_DO_GUARD_POSITION";
        case MSG_DO_GUARD_OBJECT:
            return "MSG_DO_GUARD_OBJECT";
        case MSG_DO_STOP:
            return "MSG_DO_STOP";
        case MSG_DO_SCATTER:
            return "MSG_DO_SCATTER";
        case MSG_INTERNET_HACK:
            return "MSG_INTERNET_HACK";
        case MSG_DO_CHEER:
            return "MSG_DO_CHEER";
        case MSG_TOGGLE_OVERCHARGE:
            return "MSG_TOGGLE_OVERCHARGE";
        case MSG_SWITCH_WEAPONS:
            return "MSG_SWITCH_WEAPONS";
        case MSG_CONVERT_TO_CARBOMB:
            return "MSG_CONVERT_TO_CARBOMB";
        case MSG_CAPTUREBUILDING:
            return "MSG_CAPTUREBUILDING";
        case MSG_DISABLEVEHICLE_HACK:
            return "MSG_DISABLEVEHICLE_HACK";
        case MSG_STEALCASH_HACK:
            return "MSG_STEALCASH_HACK";
        case MSG_DISABLEBUILDING_HACK:
            return "MSG_DISABLEBUILDING_HACK";
        case MSG_SNIPE_VEHICLE:
            return "MSG_SNIPE_VEHICLE";
        case MSG_DO_SALVAGE:
            return "MSG_DO_SALVAGE";
        case MSG_CLEAR_INGAME_POPUP_MESSAGE:
            return "MSG_CLEAR_INGAME_POPUP_MESSAGE";
        case MSG_PLACE_BEACON:
            return "MSG_PLACE_BEACON";
        case MSG_REMOVE_BEACON:
            return "MSG_REMOVE_BEACON";
        case MSG_SET_BEACON_TEXT:
            return "MSG_SET_BEACON_TEXT";
        case MSG_SET_REPLAY_CAMERA:
            return "MSG_SET_REPLAY_CAMERA";
        case MSG_SELF_DESTRUCT:
            return "MSG_SELF_DESTRUCT";
        case MSG_CREATE_FORMATION:
            return "MSG_CREATE_FORMATION";
        case MSG_LOGIC_CRC:
            return "MSG_LOGIC_CRC";
        case MSG_SET_MINE_CLEARING_DETAIL:
            return "MSG_SET_MINE_CLEARING_DETAIL";
        case MSG_ENABLE_RETALIATION_MODE:
            return "MSG_ENABLE_RETALIATION_MODE";
        case MSG_END_NETWORK_MESSAGES:
            return "MSG_END_NETWORK_MESSAGES";
        case MSG_TIMESTAMP:
            return "MSG_TIMESTAMP";
        case MSG_OBJECT_CREATED:
            return "MSG_OBJECT_CREATED";
        case MSG_OBJECT_DESTROYED:
            return "MSG_OBJECT_DESTROYED";
        case MSG_OBJECT_POSITION:
            return "MSG_OBJECT_POSITION";
        case MSG_OBJECT_ORIENTATION:
            return "MSG_OBJECT_ORIENTATION";
        case MSG_OBJECT_JOINED_TEAM:
            return "MSG_OBJECT_JOINED_TEAM";
        default:
            return "Invalid Command";
    }
}

void GameMessage::Append_Int_Arg(int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.integer = arg;
    argobj->m_type = ARGUMENTDATATYPE_INTEGER;
}

void GameMessage::Append_Real_Arg(float arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.real = arg;
    argobj->m_type = ARGUMENTDATATYPE_REAL;
}

void GameMessage::Append_Bool_Arg(bool arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.boolean = arg;
    argobj->m_type = ARGUMENTDATATYPE_BOOLEAN;
}

void GameMessage::Append_ObjectID_Arg(ObjectID arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.objectID = arg;
    argobj->m_type = ARGUMENTDATATYPE_OBJECTID;
}

void GameMessage::Append_DrawableID_Arg(DrawableID arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.drawableID = arg;
    argobj->m_type = ARGUMENTDATATYPE_DRAWABLEID;
}

void GameMessage::Append_TeamID_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.teamID = arg;
    argobj->m_type = ARGUMENTDATATYPE_TEAMID;
}

void GameMessage::Append_Location_Arg(Coord3D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.position = arg;
    argobj->m_type = ARGUMENTDATATYPE_LOCATION;
}

void GameMessage::Append_Pixel_Arg(ICoord2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.pixel = arg;
    argobj->m_type = ARGUMENTDATATYPE_PIXEL;
}

void GameMessage::Append_Region_Arg(IRegion2D const &arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.region = arg;
    argobj->m_type = ARGUMENTDATATYPE_PIXELREGION;
}

void GameMessage::Append_Time_Stamp_Arg(unsigned int arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.timestamp = arg;
    argobj->m_type = ARGUMENTDATATYPE_TIMESTAMP;
}

void GameMessage::Append_Wide_Char_Arg(wchar_t arg)
{
    GameMessageArgument *argobj = Allocate_Arg();
    argobj->m_data.widechar = arg;
    argobj->m_type = ARGUMENTDATATYPE_WIDECHAR;
}
