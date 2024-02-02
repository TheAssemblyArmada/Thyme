/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Information for drawing groups.
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

struct FieldParse;
class INI;

class DrawGroupInfo
{
public:
    DrawGroupInfo();

    static const FieldParse *Get_Parse_Table() { return s_parseTable; }

private:
    static void Parse_Int(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Percent_To_Real(INI *ini, void *formal, void *store, const void *user_data);

private:
    Utf8String m_fontName;
    int32_t m_fontSize;
    bool m_fontIsBold;
    bool m_usePlayerColor;
    int32_t m_colorForText;
    int32_t m_colorForTextDropShadow;
    int32_t m_dropShadowOffsetX;
    int32_t m_dropShadowOffsetY;
    union
    {
        int32_t absolute;
        float relative;
    } m_drawPositionX;
    bool m_drawPositionXAbsolute;
    union
    {
        int32_t absolute;
        float relative;
    } m_drawPositionY;
    bool m_drawPositionYAbsolute;

    static const FieldParse s_parseTable[];
    friend class Drawable;
    friend class W3DDisplayStringManager;
    friend class GameClient;
};

#ifdef GAME_DLL
extern DrawGroupInfo *&g_theDrawGroupInfo;
#else
extern DrawGroupInfo *g_theDrawGroupInfo;
#endif
