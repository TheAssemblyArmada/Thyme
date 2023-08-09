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
#pragma once

#include "always.h"

#include "asciistring.h"
#include "color.h"
#include "ini.h"

// TODO: NOT VALIDATED
class OnlineChatColors
{
public:
    OnlineChatColors();
    OnlineChatColors &operator=(OnlineChatColors &rval);

    static void Parse_Online_Chat_Colors_Definition(INI *ini);

    RGBColor Get_Default_Value() const { return m_default; }
    static const FieldParse *Get_Field_Parse() { return s_colorFieldParsetable; }

private:
    RGBColor m_default;
    RGBColor m_currentRoom;
    RGBColor m_chatRoom;
    RGBColor m_game;
    RGBColor m_gameFull;
    RGBColor m_gameCRCMismatch;
    RGBColor m_playerNormal;
    RGBColor m_playerOwner;
    RGBColor m_playerBuddy;
    RGBColor m_playerSelf;
    RGBColor m_playerIgnored;
    RGBColor m_chatNormal;
    RGBColor m_chatEmote;
    RGBColor m_chatOwner;
    RGBColor m_chatOwnerEmote;
    RGBColor m_chatPriv;
    RGBColor m_chatPrivEmote;
    RGBColor m_chatPrivOwner;
    RGBColor m_chatPrivOwnerEmote;
    RGBColor m_chatBuddy;
    RGBColor m_chatSelf;
    RGBColor m_acceptTrue;
    RGBColor m_acceptFalse;
    RGBColor m_mapSelected;
    RGBColor m_mapUnselected;
    RGBColor m_motd;
    RGBColor m_motdHeading;

    static const FieldParse s_colorFieldParsetable[];
};

inline OnlineChatColors::OnlineChatColors()
{
    m_default.Set_From_Int(-1);
}

inline OnlineChatColors &OnlineChatColors::operator=(OnlineChatColors &rval)
{
    if (this != &rval) {
        m_default = rval.Get_Default_Value();
    }

    return *this;
}