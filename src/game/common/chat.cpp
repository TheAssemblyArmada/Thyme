/**
 * @file
 *
 * @author feliwir
 *
 * @brief Class for chat handling.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "chat.h"

const FieldParse OnlineChatColors::s_colorFieldParsetable[] = {
    { "Default", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_default) },
    { "CurrentRoom", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_currentRoom) },
    { "ChatRoom", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatRoom) },
    { "Game", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_game) },
    { "GameFull", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_gameFull) },
    { "GameCRCMismatch", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_gameCRCMismatch) },
    { "PlayerNormal", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_playerNormal) },
    { "PlayerOwner", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_playerOwner) },
    { "PlayerBuddy", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_playerBuddy) },
    { "PlayerSelf", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_playerSelf) },
    { "PlayerIgnored", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_playerIgnored) },
    { "ChatNormal", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatNormal) },
    { "ChatEmote", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatEmote) },
    { "ChatOwner", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatOwner) },
    { "ChatOwnerEmote", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatOwnerEmote) },
    { "ChatPriv", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatPriv) },
    { "ChatPrivEmote", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatPrivEmote) },
    { "ChatPrivOwner", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatPrivOwner) },
    { "ChatPrivOwnerEmote", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatPrivOwnerEmote) },
    { "ChatBuddy", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatBuddy) },
    { "ChatSelf", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_chatSelf) },
    { "AcceptTrue", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_acceptTrue) },
    { "AcceptFalse", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_acceptFalse) },
    { "MapSelected", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_mapSelected) },
    { "MapUnselected", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_mapUnselected) },
    { "MOTD", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_motd) },
    { "MOTDHeading", &INI::Parse_RGB_Color, nullptr, offsetof(OnlineChatColors, m_motdHeading) },
    { nullptr, nullptr, nullptr, 0 }
};

// Was originally INI::parseMultiplayerColorDefinition
void OnlineChatColors::Parse_Online_Chat_Colors_Definition(INI *ini)
{
    // TODO: this is probably some global
    OnlineChatColors *def = new OnlineChatColors();

    ini->Init_From_INI(def, Get_Field_Parse());

    delete def;
}