/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Gamespy stuff
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "chat.h"
#include "ini.h"

#ifndef GAME_DLL
int g_gameSpyColor[27];
#endif

// clang-format off
// These are meant to use sizeof here because they are array indexes
const FieldParse g_gameSpyColorFieldParse[] = {
    {"Default", &INI::Parse_Color_Int, nullptr, 0 * sizeof(int)},
    {"CurrentRoom", &INI::Parse_Color_Int, nullptr, 1 * sizeof(int)},
    {"ChatRoom", &INI::Parse_Color_Int, nullptr, 2 * sizeof(int)},
    {"Game", &INI::Parse_Color_Int, nullptr, 3 * sizeof(int)},
    {"GameFull", &INI::Parse_Color_Int, nullptr, 4 * sizeof(int)},
    {"GameCRCMismatch", &INI::Parse_Color_Int, nullptr, 5 * sizeof(int)},
    {"PlayerNormal", &INI::Parse_Color_Int, nullptr, 6 * sizeof(int)},
    {"PlayerOwner", &INI::Parse_Color_Int, nullptr, 7 * sizeof(int)},
    {"PlayerBuddy", &INI::Parse_Color_Int, nullptr, 8 * sizeof(int)},
    {"PlayerSelf", &INI::Parse_Color_Int, nullptr, 9 * sizeof(int)},
    {"PlayerIgnored", &INI::Parse_Color_Int, nullptr, 10 * sizeof(int)},
    {"ChatNormal", &INI::Parse_Color_Int, nullptr, 11 * sizeof(int)},
    {"ChatEmote", &INI::Parse_Color_Int, nullptr, 12 * sizeof(int)},
    {"ChatOwner", &INI::Parse_Color_Int, nullptr, 13 * sizeof(int)},
    {"ChatOwnerEmote", &INI::Parse_Color_Int, nullptr, 14 * sizeof(int)},
    {"ChatPriv", &INI::Parse_Color_Int, nullptr, 15 * sizeof(int)},
    {"ChatPrivEmote", &INI::Parse_Color_Int, nullptr, 16 * sizeof(int)},
    {"ChatPrivOwner", &INI::Parse_Color_Int, nullptr, 17 * sizeof(int)},
    {"ChatPrivOwnerEmote", &INI::Parse_Color_Int, nullptr, 18 * sizeof(int)},
    {"ChatBuddy", &INI::Parse_Color_Int, nullptr, 19 * sizeof(int)},
    {"ChatSelf", &INI::Parse_Color_Int, nullptr, 20 * sizeof(int)},
    {"AcceptTrue", &INI::Parse_Color_Int, nullptr, 21 * sizeof(int)},
    {"AcceptFalse", &INI::Parse_Color_Int, nullptr, 22 * sizeof(int)},
    {"MapSelected", &INI::Parse_Color_Int, nullptr, 23 * sizeof(int)},
    {"MapUnselected", &INI::Parse_Color_Int, nullptr, 24 * sizeof(int)},
    {"MOTD", &INI::Parse_Color_Int, nullptr, 25 * sizeof(int)},
    {"MOTDHeading", &INI::Parse_Color_Int, nullptr, 26 * sizeof(int)},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

void INI::Parse_Online_Chat_Color_Definition(INI *ini)
{
    ini->Init_From_INI(g_gameSpyColor, g_gameSpyColorFieldParse);
}
