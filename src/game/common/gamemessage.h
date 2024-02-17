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
#pragma once

#include "always.h"
#include "asciistring.h"
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"

class GameMessageList;

enum ArgumentDataType
{
    ARGUMENTDATATYPE_INTEGER = 0,
    ARGUMENTDATATYPE_REAL,
    ARGUMENTDATATYPE_BOOLEAN,
    ARGUMENTDATATYPE_OBJECTID,
    ARGUMENTDATATYPE_DRAWABLEID,
    ARGUMENTDATATYPE_TEAMID,
    ARGUMENTDATATYPE_LOCATION,
    ARGUMENTDATATYPE_PIXEL,
    ARGUMENTDATATYPE_PIXELREGION,
    ARGUMENTDATATYPE_TIMESTAMP,
    ARGUMENTDATATYPE_WIDECHAR,
    ARGUMENTDATATYPE_UNKNOWN,
};

union ArgumentType
{
    ArgumentType() {}

    int integer;
    float real;
    bool boolean;
    ObjectID objectID;
    DrawableID drawableID;
    unsigned int teamID;
    Coord3D position;
    ICoord2D pixel;
    IRegion2D region;
    unsigned int timestamp;
    wchar_t widechar;
};

class GameMessageArgument : public MemoryPoolObject
{
    IMPLEMENT_POOL(GameMessageArgument);

protected:
    virtual ~GameMessageArgument() override {}

public:
    // #BUGFIX Initialize important members
    GameMessageArgument() : m_next(nullptr) {}

public:
    GameMessageArgument *m_next;
    ArgumentType m_data;
    ArgumentDataType m_type;
};

class GameMessage : public MemoryPoolObject
{
    IMPLEMENT_POOL(GameMessage);

    friend class GameMessageList;

public:
    enum MessageType // Verified.
    {
        MSG_INVALID = 0,
        MSG_FRAME_TICK = 1,
        MSG_RAW_MOUSE_BEGIN = 2,
        MSG_RAW_MOUSE_POSITION = 3,
        MSG_RAW_MOUSE_LEFT_BUTTON_DOWN = 4,
        MSG_RAW_MOUSE_LEFT_DOUBLE_CLICK = 5,
        MSG_RAW_MOUSE_LEFT_BUTTON_UP = 6,
        MSG_RAW_MOUSE_LEFT_CLICK = 7,
        MSG_RAW_MOUSE_LEFT_DRAG = 8,
        MSG_RAW_MOUSE_MIDDLE_BUTTON_DOWN = 9,
        MSG_RAW_MOUSE_MIDDLE_DOUBLE_CLICK = 10,
        MSG_RAW_MOUSE_MIDDLE_BUTTON_UP = 11,
        MSG_RAW_MOUSE_MIDDLE_DRAG = 12,
        MSG_RAW_MOUSE_RIGHT_BUTTON_DOWN = 13,
        MSG_RAW_MOUSE_RIGHT_DOUBLE_CLICK = 14,
        MSG_RAW_MOUSE_RIGHT_BUTTON_UP = 15,
        MSG_RAW_MOUSE_RIGHT_DRAG = 16,
        MSG_RAW_MOUSE_WHEEL = 17,
        MSG_RAW_MOUSE_END = 18,
        MSG_RAW_KEY_DOWN = 19,
        MSG_RAW_KEY_UP = 20,
        MSG_MOUSE_LEFT_CLICK = 21,
        MSG_MOUSE_LEFT_DOUBLE_CLICK = 22,
        MSG_MOUSE_MIDDLE_CLICK = 23,
        MSG_MOUSE_MIDDLE_DOUBLE_CLICK = 24,
        MSG_MOUSE_RIGHT_CLICK = 25,
        MSG_MOUSE_RIGHT_DOUBLE_CLICK = 26,
        MSG_CLEAR_GAME_DATA = 27,
        MSG_NEW_GAME = 28,
        MSG_BEGIN_META_MESSAGES = 29,
        MSG_META_SAVE_VIEW1 = 30,
        MSG_META_SAVE_VIEW2 = 31,
        MSG_META_SAVE_VIEW3 = 32,
        MSG_META_SAVE_VIEW4 = 33,
        MSG_META_SAVE_VIEW5 = 34,
        MSG_META_SAVE_VIEW6 = 35,
        MSG_META_SAVE_VIEW7 = 36,
        MSG_META_SAVE_VIEW8 = 37,
        MSG_META_VIEW_VIEW1 = 38,
        MSG_META_VIEW_VIEW2 = 39,
        MSG_META_VIEW_VIEW3 = 40,
        MSG_META_VIEW_VIEW4 = 41,
        MSG_META_VIEW_VIEW5 = 42,
        MSG_META_VIEW_VIEW6 = 43,
        MSG_META_VIEW_VIEW7 = 44,
        MSG_META_VIEW_VIEW8 = 45,
        MSG_META_CREATE_TEAM0 = 46,
        MSG_META_CREATE_TEAM1 = 47,
        MSG_META_CREATE_TEAM2 = 48,
        MSG_META_CREATE_TEAM3 = 49,
        MSG_META_CREATE_TEAM4 = 50,
        MSG_META_CREATE_TEAM5 = 51,
        MSG_META_CREATE_TEAM6 = 52,
        MSG_META_CREATE_TEAM7 = 53,
        MSG_META_CREATE_TEAM8 = 54,
        MSG_META_CREATE_TEAM9 = 55,
        MSG_META_SELECT_TEAM0 = 56,
        MSG_META_SELECT_TEAM1 = 57,
        MSG_META_SELECT_TEAM2 = 58,
        MSG_META_SELECT_TEAM3 = 59,
        MSG_META_SELECT_TEAM4 = 60,
        MSG_META_SELECT_TEAM5 = 61,
        MSG_META_SELECT_TEAM6 = 62,
        MSG_META_SELECT_TEAM7 = 63,
        MSG_META_SELECT_TEAM8 = 64,
        MSG_META_SELECT_TEAM9 = 65,
        MSG_META_ADD_TEAM0 = 66,
        MSG_META_ADD_TEAM1 = 67,
        MSG_META_ADD_TEAM2 = 68,
        MSG_META_ADD_TEAM3 = 69,
        MSG_META_ADD_TEAM4 = 70,
        MSG_META_ADD_TEAM5 = 71,
        MSG_META_ADD_TEAM6 = 72,
        MSG_META_ADD_TEAM7 = 73,
        MSG_META_ADD_TEAM8 = 74,
        MSG_META_ADD_TEAM9 = 75,
        MSG_META_VIEW_TEAM0 = 76,
        MSG_META_VIEW_TEAM1 = 77,
        MSG_META_VIEW_TEAM2 = 78,
        MSG_META_VIEW_TEAM3 = 79,
        MSG_META_VIEW_TEAM4 = 80,
        MSG_META_VIEW_TEAM5 = 81,
        MSG_META_VIEW_TEAM6 = 82,
        MSG_META_VIEW_TEAM7 = 83,
        MSG_META_VIEW_TEAM8 = 84,
        MSG_META_VIEW_TEAM9 = 85,
        MSG_META_SELECT_MATCHING_UNITS = 86,
        MSG_META_SELECT_NEXT_UNIT = 87,
        MSG_META_SELECT_PREV_UNIT = 88,
        MSG_META_SELECT_NEXT_WORKER = 89,
        MSG_META_SELECT_PREV_WORKER = 90,
        MSG_META_VIEW_COMMAND_CENTER = 91,
        MSG_META_VIEW_LAST_RADAR_EVENT = 92,
        MSG_META_SELECT_HERO = 93,
        MSG_META_SELECT_ALL = 94,
        MSG_META_SELECT_ALL_AIRCRAFT = 95,
        MSG_META_SCATTER = 96,
        MSG_META_STOP = 97,
        MSG_META_DEPLOY = 98,
        MSG_META_CREATE_FORMATION = 99,
        MSG_META_FOLLOW = 100,
        MSG_META_CHAT_PLAYERS = 101,
        MSG_META_CHAT_ALLIES = 102,
        MSG_META_CHAT_EVERYONE = 103,
        MSG_META_DIPLOMACY = 104,
        MSG_META_OPTIONS = 105,
        MSG_META_TOGGLE_LOWER_DETAILS = 106,
        MSG_META_TOGGLE_CONTROL_BAR = 107,
        MSG_META_BEGIN_PATH_BUILD = 108,
        MSG_META_END_PATH_BUILD = 109,
        MSG_META_BEGIN_FORCEATTACK = 110,
        MSG_META_END_FORCEATTACK = 111,
        MSG_META_BEGIN_FORCEMOVE = 112,
        MSG_META_END_FORCEMOVE = 113,
        MSG_META_BEGIN_WAYPOINTS = 114,
        MSG_META_END_WAYPOINTS = 115,
        MSG_META_BEGIN_PREFER_SELECTION = 116,
        MSG_META_END_PREFER_SELECTION = 117,
        MSG_META_TAKE_SCREENSHOT = 118,
        MSG_META_ALL_CHEER = 119,
        MSG_META_TOGGLE_ATTACKMOVE = 120,
        MSG_META_BEGIN_CAMERA_ROTATE_LEFT = 121,
        MSG_META_END_CAMERA_ROTATE_LEFT = 122,
        MSG_META_BEGIN_CAMERA_ROTATE_RIGHT = 123,
        MSG_META_END_CAMERA_ROTATE_RIGHT = 124,
        MSG_META_BEGIN_CAMERA_ZOOM_IN = 125,
        MSG_META_END_CAMERA_ZOOM_IN = 126,
        MSG_META_BEGIN_CAMERA_ZOOM_OUT = 127,
        MSG_META_END_CAMERA_ZOOM_OUT = 128,
        MSG_META_CAMERA_RESET = 129,
        MSG_META_TOGGLE_CAMERA_TRACKING_DRAWABLE = 130,
        MSG_META_TOGGLE_FAST_FORWARD_REPLAY = 131,
        MSG_META_DEMO_INSTANT_QUIT = 132,
        MSG_META_PLACE_BEACON = 133,
        MSG_META_REMOVE_BEACON = 134,
        MSG_END_META_MESSAGES = 135,
        MSG_MOUSEOVER_DRAWABLE_HINT = 136,
        MSG_MOUSEOVER_LOCATION_HINT = 137,
        MSG_VALID_GUICOMMAND_HINT = 138,
        MSG_INVALID_GUICOMMAND_HINT = 139,
        MSG_AREA_SELECTION_HINT = 140,
        MSG_DO_ATTACK_OBJECT_HINT = 141,
        MSG_UNK1 = 142,
        MSG_DO_FORCE_ATTACK_OBJECT_HINT = 143,
        MSG_DO_FORCE_ATTACK_GROUND_HINT = 144,
        MSG_GET_REPAIRED_HINT = 145,
        MSG_GET_HEALED_HINT = 146,
        MSG_DO_REPAIR_HINT = 147,
        MSG_RESUME_CONSTRUCTION_HINT = 148,
        MSG_ENTER_HINT = 149,
        MSG_DOCK_HINT = 150,
        MSG_DO_MOVETO_HINT = 151,
        MSG_DO_ATTACKMOVETO_HINT = 152,
        MSG_ADD_WAYPOINT_HINT = 153,
        MSG_HIJACK_HINT = 154,
        MSG_SABOTAGE_HINT = 155,
        MSG_FIREBOMB_HINT = 156,
        MSG_CONVERT_TO_CARBOMB_HINT = 157,
        MSG_CAPTUREBUILDING_HINT = 158,
        MSG_SNIPE_VEHICLE_HINT = 159,
        MSG_DEFECTOR_HINT = 160,
        MSG_SET_RALLY_POINT_HINT = 161,
        MSG_UNK2 = 162,
        MSG_DO_SALVAGE_HINT = 163,
        MSG_DO_INVALID_HINT = 164,
        MSG_DO_ATTACK_OBJECT_AFTER_MOVING_HINT = 165,
        MSG_HACK_HINT = 166,
        MSG_BEGIN_NETWORK_MESSAGES = 1000,
        MSG_CREATE_SELECTED_GROUP = 1001,
        MSG_CREATE_SELECTED_GROUP_NO_SOUND = 1002,
        MSG_DESTROY_SELECTED_GROUP = 1003,
        MSG_REMOVE_FROM_SELECTED_GROUP = 1004,
        MSG_SELECTED_GROUP_COMMAND = 1005,
        MSG_CREATE_TEAM0 = 1006,
        MSG_CREATE_TEAM1 = 1007,
        MSG_CREATE_TEAM2 = 1008,
        MSG_CREATE_TEAM3 = 1009,
        MSG_CREATE_TEAM4 = 1010,
        MSG_CREATE_TEAM5 = 1011,
        MSG_CREATE_TEAM6 = 1012,
        MSG_CREATE_TEAM7 = 1013,
        MSG_CREATE_TEAM8 = 1014,
        MSG_CREATE_TEAM9 = 1015,
        MSG_SELECT_TEAM0 = 1016,
        MSG_SELECT_TEAM1 = 1017,
        MSG_SELECT_TEAM2 = 1018,
        MSG_SELECT_TEAM3 = 1019,
        MSG_SELECT_TEAM4 = 1020,
        MSG_SELECT_TEAM5 = 1021,
        MSG_SELECT_TEAM6 = 1022,
        MSG_SELECT_TEAM7 = 1023,
        MSG_SELECT_TEAM8 = 1024,
        MSG_SELECT_TEAM9 = 1025,
        MSG_ADD_TEAM0 = 1026,
        MSG_ADD_TEAM1 = 1027,
        MSG_ADD_TEAM2 = 1028,
        MSG_ADD_TEAM3 = 1029,
        MSG_ADD_TEAM4 = 1030,
        MSG_ADD_TEAM5 = 1031,
        MSG_ADD_TEAM6 = 1032,
        MSG_ADD_TEAM7 = 1033,
        MSG_ADD_TEAM8 = 1034,
        MSG_ADD_TEAM9 = 1035,
        MSG_DO_ATTACKSQUAD = 1036,
        MSG_DO_WEAPON = 1037,
        MSG_DO_WEAPON_AT_LOCATION = 1038,
        MSG_DO_WEAPON_AT_OBJECT = 1039,
        MSG_DO_SPECIAL_POWER = 1040,
        MSG_DO_SPECIAL_POWER_AT_LOCATION = 1041,
        MSG_DO_SPECIAL_POWER_AT_OBJECT = 1042,
        MSG_SET_RALLY_POINT = 1043,
        MSG_PURCHASE_SCIENCE = 1044,
        MSG_QUEUE_UPGRADE = 1045,
        MSG_CANCEL_UPGRADE = 1046,
        MSG_QUEUE_UNIT_CREATE = 1047,
        MSG_CANCEL_UNIT_CREATE = 1048,
        MSG_DOZER_CONSTRUCT = 1049,
        MSG_DOZER_CONSTRUCT_LINE = 1050,
        MSG_DOZER_CANCEL_CONSTRUCT = 1051,
        MSG_SELL = 1052,
        MSG_EXIT = 1053,
        MSG_EVACUATE = 1054,
        MSG_EXECUTE_RAILED_TRANSPORT = 1055,
        MSG_COMBATDROP_AT_LOCATION = 1056,
        MSG_COMBATDROP_AT_OBJECT = 1057,
        MSG_AREA_SELECTION = 1058,
        MSG_DO_ATTACK_OBJECT = 1059,
        MSG_DO_FORCE_ATTACK_OBJECT = 1060,
        MSG_DO_FORCE_ATTACK_GROUND = 1061,
        MSG_GET_REPAIRED = 1062,
        MSG_GET_HEALED = 1063,
        MSG_DO_REPAIR = 1064,
        MSG_RESUME_CONSTRUCTION = 1065,
        MSG_ENTER = 1066,
        MSG_DOCK = 1067,
        MSG_DO_MOVETO = 1068,
        MSG_DO_ATTACKMOVETO = 1069,
        MSG_DO_FORCEMOVETO = 1070,
        MSG_ADD_WAYPOINT = 1071,
        MSG_DO_GUARD_POSITION = 1072,
        MSG_DO_GUARD_OBJECT = 1073,
        MSG_DO_STOP = 1074,
        MSG_DO_SCATTER = 1075,
        MSG_INTERNET_HACK = 1076,
        MSG_DO_CHEER = 1077,
        MSG_TOGGLE_OVERCHARGE = 1078,
        MSG_SWITCH_WEAPONS = 1079,
        MSG_CONVERT_TO_CARBOMB = 1080,
        MSG_CAPTUREBUILDING = 1081,
        MSG_DISABLEVEHICLE_HACK = 1082,
        MSG_STEALCASH_HACK = 1083,
        MSG_DISABLEBUILDING_HACK = 1084,
        MSG_SNIPE_VEHICLE = 1085,
        MSG_DO_SALVAGE = 1087,
        MSG_CLEAR_INGAME_POPUP_MESSAGE = 1088,
        MSG_PLACE_BEACON = 1089,
        MSG_REMOVE_BEACON = 1090,
        MSG_SET_BEACON_TEXT = 1091,
        MSG_SET_REPLAY_CAMERA = 1092,
        MSG_SELF_DESTRUCT = 1093,
        MSG_CREATE_FORMATION = 1094,
        MSG_LOGIC_CRC = 1095,
        MSG_SET_MINE_CLEARING_DETAIL = 1096,
        MSG_ENABLE_RETALIATION_MODE = 1097,
        MSG_END_NETWORK_MESSAGES = 1999,
        MSG_TIMESTAMP = 2000,
        MSG_OBJECT_CREATED = 2001,
        MSG_OBJECT_DESTROYED = 2002,
        MSG_OBJECT_POSITION = 2003,
        MSG_OBJECT_ORIENTATION = 2004,
        MSG_OBJECT_JOINED_TEAM = 2005,
    };

protected:
    virtual ~GameMessage() override;

public:
    GameMessage(MessageType type);

    GameMessageArgument *Allocate_Arg();
    ArgumentType *Get_Argument(int arg) const;
    int Get_Argument_Count() const { return m_argCount; }
    ArgumentDataType Get_Argument_Type(int arg);
    Utf8String Get_Command_As_Ascii(MessageType command);

    void Append_Int_Arg(int arg);
    void Append_Real_Arg(float arg);
    void Append_Bool_Arg(bool arg);
    void Append_ObjectID_Arg(ObjectID arg);
    void Append_DrawableID_Arg(DrawableID arg);
    void Append_TeamID_Arg(unsigned int arg);
    void Append_Location_Arg(Coord3D const &arg);
    void Append_Pixel_Arg(ICoord2D const &arg);
    void Append_Region_Arg(IRegion2D const &arg);
    void Append_Time_Stamp_Arg(unsigned int arg);
    void Append_Wide_Char_Arg(wchar_t arg);

    GameMessage *Get_Next() { return m_next; }
    GameMessage *Get_Prev() { return m_prev; }

    MessageType Get_Type() const { return m_type; }
    int Get_Player_Index() const { return m_playerIndex; }

private:
    GameMessage *m_next;
    GameMessage *m_prev;
    GameMessageList *m_list;
    MessageType m_type;
    int m_playerIndex;
    int8_t m_argCount;
    // 3 bytes padding
    GameMessageArgument *m_argList;
    GameMessageArgument *m_argTail;
};
