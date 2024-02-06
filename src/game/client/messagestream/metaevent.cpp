/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Meta Map
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "metaevent.h"
#ifdef GAME_DLL
#include "hooker.h"
#endif

#ifndef GAME_DLL
MetaMap *g_theMetaMap;
#endif

static const LookupListRec s_gameMessageMetaTypeNames[] = { { "SAVE_VIEW1", GameMessage::MSG_META_SAVE_VIEW1 },
    { "SAVE_VIEW2", GameMessage::MSG_META_SAVE_VIEW2 },
    { "SAVE_VIEW3", GameMessage::MSG_META_SAVE_VIEW3 },
    { "SAVE_VIEW4", GameMessage::MSG_META_SAVE_VIEW4 },
    { "SAVE_VIEW5", GameMessage::MSG_META_SAVE_VIEW5 },
    { "SAVE_VIEW6", GameMessage::MSG_META_SAVE_VIEW6 },
    { "SAVE_VIEW7", GameMessage::MSG_META_SAVE_VIEW7 },
    { "SAVE_VIEW8", GameMessage::MSG_META_SAVE_VIEW8 },
    { "VIEW_VIEW1", GameMessage::MSG_META_VIEW_VIEW1 },
    { "VIEW_VIEW2", GameMessage::MSG_META_VIEW_VIEW2 },
    { "VIEW_VIEW3", GameMessage::MSG_META_VIEW_VIEW3 },
    { "VIEW_VIEW4", GameMessage::MSG_META_VIEW_VIEW4 },
    { "VIEW_VIEW5", GameMessage::MSG_META_VIEW_VIEW5 },
    { "VIEW_VIEW6", GameMessage::MSG_META_VIEW_VIEW6 },
    { "VIEW_VIEW7", GameMessage::MSG_META_VIEW_VIEW7 },
    { "VIEW_VIEW8", GameMessage::MSG_META_VIEW_VIEW8 },
    { "CREATE_TEAM0", GameMessage::MSG_META_CREATE_TEAM0 },
    { "CREATE_TEAM1", GameMessage::MSG_META_CREATE_TEAM1 },
    { "CREATE_TEAM2", GameMessage::MSG_META_CREATE_TEAM2 },
    { "CREATE_TEAM3", GameMessage::MSG_META_CREATE_TEAM3 },
    { "CREATE_TEAM4", GameMessage::MSG_META_CREATE_TEAM4 },
    { "CREATE_TEAM5", GameMessage::MSG_META_CREATE_TEAM5 },
    { "CREATE_TEAM6", GameMessage::MSG_META_CREATE_TEAM6 },
    { "CREATE_TEAM7", GameMessage::MSG_META_CREATE_TEAM7 },
    { "CREATE_TEAM8", GameMessage::MSG_META_CREATE_TEAM8 },
    { "CREATE_TEAM9", GameMessage::MSG_META_CREATE_TEAM9 },
    { "SELECT_TEAM0", GameMessage::MSG_META_SELECT_TEAM0 },
    { "SELECT_TEAM1", GameMessage::MSG_META_SELECT_TEAM1 },
    { "SELECT_TEAM2", GameMessage::MSG_META_SELECT_TEAM2 },
    { "SELECT_TEAM3", GameMessage::MSG_META_SELECT_TEAM3 },
    { "SELECT_TEAM4", GameMessage::MSG_META_SELECT_TEAM4 },
    { "SELECT_TEAM5", GameMessage::MSG_META_SELECT_TEAM5 },
    { "SELECT_TEAM6", GameMessage::MSG_META_SELECT_TEAM6 },
    { "SELECT_TEAM7", GameMessage::MSG_META_SELECT_TEAM7 },
    { "SELECT_TEAM8", GameMessage::MSG_META_SELECT_TEAM8 },
    { "SELECT_TEAM9", GameMessage::MSG_META_SELECT_TEAM9 },
    { "ADD_TEAM0", GameMessage::MSG_META_ADD_TEAM0 },
    { "ADD_TEAM1", GameMessage::MSG_META_ADD_TEAM1 },
    { "ADD_TEAM2", GameMessage::MSG_META_ADD_TEAM2 },
    { "ADD_TEAM3", GameMessage::MSG_META_ADD_TEAM3 },
    { "ADD_TEAM4", GameMessage::MSG_META_ADD_TEAM4 },
    { "ADD_TEAM5", GameMessage::MSG_META_ADD_TEAM5 },
    { "ADD_TEAM6", GameMessage::MSG_META_ADD_TEAM6 },
    { "ADD_TEAM7", GameMessage::MSG_META_ADD_TEAM7 },
    { "ADD_TEAM8", GameMessage::MSG_META_ADD_TEAM8 },
    { "ADD_TEAM9", GameMessage::MSG_META_ADD_TEAM9 },
    { "VIEW_TEAM0", GameMessage::MSG_META_VIEW_TEAM0 },
    { "VIEW_TEAM1", GameMessage::MSG_META_VIEW_TEAM1 },
    { "VIEW_TEAM2", GameMessage::MSG_META_VIEW_TEAM2 },
    { "VIEW_TEAM3", GameMessage::MSG_META_VIEW_TEAM3 },
    { "VIEW_TEAM4", GameMessage::MSG_META_VIEW_TEAM4 },
    { "VIEW_TEAM5", GameMessage::MSG_META_VIEW_TEAM5 },
    { "VIEW_TEAM6", GameMessage::MSG_META_VIEW_TEAM6 },
    { "VIEW_TEAM7", GameMessage::MSG_META_VIEW_TEAM7 },
    { "VIEW_TEAM8", GameMessage::MSG_META_VIEW_TEAM8 },
    { "VIEW_TEAM9", GameMessage::MSG_META_VIEW_TEAM9 },
    { "SELECT_MATCHING_UNITS", GameMessage::MSG_META_SELECT_MATCHING_UNITS },
    { "SELECT_NEXT_UNIT", GameMessage::MSG_META_SELECT_NEXT_UNIT },
    { "SELECT_PREV_UNIT", GameMessage::MSG_META_SELECT_PREV_UNIT },
    { "SELECT_NEXT_WORKER", GameMessage::MSG_META_SELECT_NEXT_WORKER },
    { "SELECT_PREV_WORKER", GameMessage::MSG_META_SELECT_PREV_WORKER },
    { "SELECT_HERO", GameMessage::MSG_META_SELECT_HERO },
    { "SELECT_ALL", GameMessage::MSG_META_SELECT_ALL },
    { "SELECT_ALL_AIRCRAFT", GameMessage::MSG_META_SELECT_ALL_AIRCRAFT },
    { "VIEW_COMMAND_CENTER", GameMessage::MSG_META_VIEW_COMMAND_CENTER },
    { "VIEW_LAST_RADAR_EVENT", GameMessage::MSG_META_VIEW_LAST_RADAR_EVENT },
    { "SCATTER", GameMessage::MSG_META_SCATTER },
    { "STOP", GameMessage::MSG_META_STOP },
    { "DEPLOY", GameMessage::MSG_META_DEPLOY },
    { "CREATE_FORMATION", GameMessage::MSG_META_CREATE_FORMATION },
    { "FOLLOW", GameMessage::MSG_META_FOLLOW },
    { "CHAT_PLAYERS", GameMessage::MSG_META_CHAT_PLAYERS },
    { "CHAT_ALLIES", GameMessage::MSG_META_CHAT_ALLIES },
    { "CHAT_EVERYONE", GameMessage::MSG_META_CHAT_EVERYONE },
    { "DIPLOMACY", GameMessage::MSG_META_DIPLOMACY },
    { "PLACE_BEACON", GameMessage::MSG_META_PLACE_BEACON },
    { "DELETE_BEACON", GameMessage::MSG_META_REMOVE_BEACON },
    { "OPTIONS", GameMessage::MSG_META_OPTIONS },
    { "TOGGLE_LOWER_DETAILS", GameMessage::MSG_META_TOGGLE_LOWER_DETAILS },
    { "TOGGLE_CONTROL_BAR", GameMessage::MSG_META_TOGGLE_CONTROL_BAR },
    { "BEGIN_PATH_BUILD", GameMessage::MSG_META_BEGIN_PATH_BUILD },
    { "END_PATH_BUILD", GameMessage::MSG_META_END_PATH_BUILD },
    { "BEGIN_FORCEATTACK", GameMessage::MSG_META_BEGIN_FORCEATTACK },
    { "END_FORCEATTACK", GameMessage::MSG_META_END_FORCEATTACK },
    { "BEGIN_FORCEMOVE", GameMessage::MSG_META_BEGIN_FORCEMOVE },
    { "END_FORCEMOVE", GameMessage::MSG_META_END_FORCEMOVE },
    { "BEGIN_WAYPOINTS", GameMessage::MSG_META_BEGIN_WAYPOINTS },
    { "END_WAYPOINTS", GameMessage::MSG_META_END_WAYPOINTS },
    { "BEGIN_PREFER_SELECTION", GameMessage::MSG_META_BEGIN_PREFER_SELECTION },
    { "END_PREFER_SELECTION", GameMessage::MSG_META_END_PREFER_SELECTION },
    { "TAKE_SCREENSHOT", GameMessage::MSG_META_TAKE_SCREENSHOT },
    { "ALL_CHEER", GameMessage::MSG_META_ALL_CHEER },
    { "BEGIN_CAMERA_ROTATE_LEFT", GameMessage::MSG_META_BEGIN_CAMERA_ROTATE_LEFT },
    { "END_CAMERA_ROTATE_LEFT", GameMessage::MSG_META_END_CAMERA_ROTATE_LEFT },
    { "BEGIN_CAMERA_ROTATE_RIGHT", GameMessage::MSG_META_BEGIN_CAMERA_ROTATE_RIGHT },
    { "END_CAMERA_ROTATE_RIGHT", GameMessage::MSG_META_END_CAMERA_ROTATE_RIGHT },
    { "BEGIN_CAMERA_ZOOM_IN", GameMessage::MSG_META_BEGIN_CAMERA_ZOOM_IN },
    { "END_CAMERA_ZOOM_IN", GameMessage::MSG_META_END_CAMERA_ZOOM_IN },
    { "BEGIN_CAMERA_ZOOM_OUT", GameMessage::MSG_META_BEGIN_CAMERA_ZOOM_OUT },
    { "END_CAMERA_ZOOM_OUT", GameMessage::MSG_META_END_CAMERA_ZOOM_OUT },
    { "CAMERA_RESET", GameMessage::MSG_META_CAMERA_RESET },
    { "TOGGLE_CAMERA_TRACKING_DRAWABLE", GameMessage::MSG_META_TOGGLE_CAMERA_TRACKING_DRAWABLE },
    { "TOGGLE_FAST_FORWARD_REPLAY", GameMessage::MSG_META_TOGGLE_FAST_FORWARD_REPLAY },
    { "DEMO_INSTANT_QUIT", GameMessage::MSG_META_DEMO_INSTANT_QUIT },
    { nullptr, 0 } };

static const LookupListRec s_keyNames[] = { { "KEY_ESC", MK_ESC },
    { "KEY_BACKSPACE", MK_BACKSPACE },
    { "KEY_ENTER", MK_ENTER },
    { "KEY_SPACE", MK_SPACE },
    { "KEY_TAB", MK_TAB },
    { "KEY_F1", MK_F1 },
    { "KEY_F2", MK_F2 },
    { "KEY_F3", MK_F3 },
    { "KEY_F4", MK_F4 },
    { "KEY_F5", MK_F5 },
    { "KEY_F6", MK_F6 },
    { "KEY_F7", MK_F7 },
    { "KEY_F8", MK_F8 },
    { "KEY_F9", MK_F9 },
    { "KEY_F10", MK_F10 },
    { "KEY_F11", MK_F11 },
    { "KEY_F12", MK_F12 },
    { "KEY_A", MK_A },
    { "KEY_B", MK_B },
    { "KEY_C", MK_C },
    { "KEY_D", MK_D },
    { "KEY_E", MK_E },
    { "KEY_F", MK_F },
    { "KEY_G", MK_G },
    { "KEY_H", MK_H },
    { "KEY_I", MK_I },
    { "KEY_J", MK_J },
    { "KEY_K", MK_K },
    { "KEY_L", MK_L },
    { "KEY_M", MK_M },
    { "KEY_N", MK_N },
    { "KEY_O", MK_O },
    { "KEY_P", MK_P },
    { "KEY_Q", MK_Q },
    { "KEY_R", MK_R },
    { "KEY_S", MK_S },
    { "KEY_T", MK_T },
    { "KEY_U", MK_U },
    { "KEY_V", MK_V },
    { "KEY_W", MK_W },
    { "KEY_X", MK_X },
    { "KEY_Y", MK_Y },
    { "KEY_Z", MK_Z },
    { "KEY_1", MK_1 },
    { "KEY_2", MK_2 },
    { "KEY_3", MK_3 },
    { "KEY_4", MK_4 },
    { "KEY_5", MK_5 },
    { "KEY_6", MK_6 },
    { "KEY_7", MK_7 },
    { "KEY_8", MK_8 },
    { "KEY_9", MK_9 },
    { "KEY_0", MK_0 },
    { "KEY_KP1", MK_KP1 },
    { "KEY_KP2", MK_KP2 },
    { "KEY_KP3", MK_KP3 },
    { "KEY_KP4", MK_KP4 },
    { "KEY_KP5", MK_KP5 },
    { "KEY_KP6", MK_KP6 },
    { "KEY_KP7", MK_KP7 },
    { "KEY_KP8", MK_KP8 },
    { "KEY_KP9", MK_KP9 },
    { "KEY_KP0", MK_KP0 },
    { "KEY_MINUS", MK_MINUS },
    { "KEY_EQUAL", MK_EQUAL },
    { "KEY_LBRACKET", MK_LBRACKET },
    { "KEY_RBRACKET", MK_RBRACKET },
    { "KEY_SEMICOLON", MK_SEMICOLON },
    { "KEY_APOSTROPHE", MK_APOSTROPHE },
    { "KEY_TICK", MK_TICK },
    { "KEY_BACKSLASH", MK_BACKSLASH },
    { "KEY_COMMA", MK_COMMA },
    { "KEY_PERIOD", MK_PERIOD },
    { "KEY_SLASH", MK_SLASH },
    { "KEY_UP", MK_UP },
    { "KEY_DOWN", MK_DOWN },
    { "KEY_LEFT", MK_LEFT },
    { "KEY_RIGHT", MK_RIGHT },
    { "KEY_HOME", MK_HOME },
    { "KEY_END", MK_END },
    { "KEY_PGUP", MK_PGUP },
    { "KEY_PGDN", MK_PGDN },
    { "KEY_INS", MK_INS },
    { "KEY_DEL", MK_DEL },
    { "KEY_KPSLASH", MK_KPSPLASH },
    { "KEY_NONE", MK_NONE },
    { nullptr, 0 } };

static const LookupListRec s_transitionNames[] = {
    { "DOWN", DOWN }, { "UP", UP }, { "DOUBLEDOWN", DOUBLEDOWN }, { nullptr, 0 }
};

static const LookupListRec s_modifierNames[] = { { "NONE", NONE },
    { "CTRL", CTRL },
    { "ALT", ALT },
    { "SHIFT", SHIFT },
    { "CTRL_ALT", CTRL_ALT },
    { "SHIFT_CTRL", SHIFT_CTRL },
    { "SHIFT_ALT", SHIFT_ALT },
    { "SHIFT_ALT_CTRL", SHIFT_ALT_CTRL },
    { nullptr, 0 } };

static const char *s_theCommandUsableNames[] = { "SHELL", "GAME", nullptr };

static const LookupListRec s_categoryListName[] = { { "CONTROL", CATEGORY_CONTROL },
    { "INFORMATION", CATEGORY_INFORMATION },
    { "INTERFACE", CATEGORY_INTERFACE },
    { "SELECTION", CATEGORY_SELECTION },
    { "TAUNT", CATEGORY_TAUNT },
    { "TEAM", CATEGORY_TEAM },
    { "MISC", CATEGORY_MISC },
    { "DEBUG", CATEGORY_DEBUG },
    { nullptr, 0 } };

// clang-format off
static const FieldParse s_metaMapFieldParseTable[] = {
    {"Key", &INI::Parse_Lookup_List, s_keyNames, offsetof(MetaMapRec, m_key)},
    {"Transition", &INI::Parse_Lookup_List, s_transitionNames, offsetof(MetaMapRec, m_transition)},
    {"Modifiers", &INI::Parse_Lookup_List, s_modifierNames, offsetof(MetaMapRec, m_modState)},
    {"UseableIn", &INI::Parse_Bitstring32, s_theCommandUsableNames, offsetof(MetaMapRec, m_usableIn)},
    {"Category", &INI::Parse_Lookup_List, s_categoryListName, offsetof(MetaMapRec, m_category)},
    {"Description", &INI::Parse_And_Translate_Label, nullptr, offsetof(MetaMapRec, m_description)},
    {"DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(MetaMapRec, m_displayName)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

MetaMap::~MetaMap()
{
    while (m_metaMaps != nullptr) {
        MetaMapRec *map = m_metaMaps->m_next;
        m_metaMaps->Delete_Instance();
        m_metaMaps = map;
    }
}

MetaEventTranslator::MetaEventTranslator() : m_lastKeyDown(0), m_lastModState(0)
{
    for (int i = 0; i < 3; i++) {
        m_lastButtonPos[i].x = 0;
        m_lastButtonPos[i].y = 0;
        m_lastButtonState[i] = false;
    }
}

GameMessageDisposition MetaEventTranslator::Translate_Game_Message(const GameMessage *msg)
{
#ifdef GAME_DLL
    return Call_Method<GameMessageDisposition, MetaEventTranslator, const GameMessage *>(
        PICK_ADDRESS(0x00497DA0, 0x009C0D65), this, msg);
#else
    return KEEP_MESSAGE;
#endif
}

GameMessage::MessageType MetaMap::Find_Game_Message_Meta_Type(const char *type)
{
    for (const LookupListRec *list = s_gameMessageMetaTypeNames; list->name != nullptr; list++) {
        if (strcasecmp(list->name, type) == 0) {
            return static_cast<GameMessage::MessageType>(list->value);
        }
    }

    captainslog_dbgassert(
        false, "MetaTypeName %s not found -- did you remember to add it to s_gameMessageMetaTypeNames[] ?", type);
    return GameMessage::MSG_INVALID;
}

MetaMapRec *MetaMap::Get_Meta_Map_Rec(GameMessage::MessageType t)
{
    for (MetaMapRec *map = m_metaMaps; map != nullptr; map = map->m_next) {
        if (map->m_meta == t) {
            return map;
        }
    }

    MetaMapRec *map = new MetaMapRec();
    map->m_meta = t;
    map->m_key = MK_NONE;
    map->m_transition = DOWN;
    map->m_modState = NONE;
    map->m_usableIn = COMMANDUSABLE_NONE;
    map->m_category = CATEGORY_MISC;
    map->m_description.Clear();
    map->m_displayName.Clear();
    map->m_next = m_metaMaps;
    m_metaMaps = map;
    return map;
}

void MetaMap::Parse_Meta_Map(INI *ini)
{
    GameMessage::MessageType type = g_theMetaMap->Find_Game_Message_Meta_Type(ini->Get_Next_Token());

    if (type == GameMessage::MSG_INVALID) {
        throw CODE_06;
    }

    MetaMapRec *map = g_theMetaMap->Get_Meta_Map_Rec(type);

    if (map == nullptr) {
        throw CODE_06;
    }

    ini->Init_From_INI(map, s_metaMapFieldParseTable);
}
