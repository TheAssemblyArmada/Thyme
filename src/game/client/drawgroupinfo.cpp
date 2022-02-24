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
#include "drawgroupinfo.h"
#include "color.h"
#include "colorspace.h"
#include "ini.h"
#include <cstddef>

#ifndef GAME_DLL
DrawGroupInfo *g_theDrawGroupInfo;
#endif

// clang-format off
const FieldParse DrawGroupInfo::s_parseTable[] = {
    {"UsePlayerColor", &INI::Parse_Bool, nullptr, offsetof(DrawGroupInfo, m_usePlayerColor)},
    {"ColorForText", &INI::Parse_Color_Int, nullptr, offsetof(DrawGroupInfo, m_colorForText)},
    {"ColorForTextDropShadow", &INI::Parse_Color_Int, nullptr, offsetof(DrawGroupInfo, m_colorForTextDropShadow)},
    {"FontName", &INI::Parse_Quoted_AsciiString, nullptr, offsetof(DrawGroupInfo, m_fontName)},
    {"FontSize", &INI::Parse_Int, nullptr, offsetof(DrawGroupInfo, m_fontSize)},
    {"FontIsBold", &INI::Parse_Bool, nullptr, offsetof(DrawGroupInfo, m_fontIsBold)},
    {"DropShadowOffsetX", &INI::Parse_Int, nullptr, offsetof(DrawGroupInfo, m_dropShadowOffsetX)},
    {"DropShadowOffsetY", &INI::Parse_Int, nullptr, offsetof(DrawGroupInfo, m_dropShadowOffsetY)},
    {"DrawPositionXPixel", &Parse_Int, reinterpret_cast<const void *>(0), 0},
    {"DrawPositionXPercent", &Parse_Percent_To_Real, reinterpret_cast<const void *>(0), 0},
    {"DrawPositionYPixel", &Parse_Int, reinterpret_cast<const void *>(1), 0},
    {"DrawPositionYPercent", &Parse_Percent_To_Real, reinterpret_cast<const void *>(1), 0},
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on

DrawGroupInfo::DrawGroupInfo() :
    m_fontName("Arial"),
    m_fontSize(10),
    m_fontIsBold(false),
    m_usePlayerColor(true),
    m_colorForText(Make_Color(255, 255, 255, 255)),
    m_colorForTextDropShadow(Make_Color(0, 0, 0, 255)),
    m_dropShadowOffsetX(-1),
    m_dropShadowOffsetY(-1),
    m_drawPositionXAbsolute(false),
    m_drawPositionYAbsolute(true)
{
    m_drawPositionX.relative = -0.050000001f;
    m_drawPositionY.absolute = -10;
}

/**
 * Custom int parser for DrawGroupInfo entries.
 * Was originally parseInt
 */
void DrawGroupInfo::Parse_Int(INI *ini, void *formal, void *store, const void *user_data)
{
    if (!user_data) {
        static_cast<DrawGroupInfo *>(store)->m_drawPositionXAbsolute = true;
        INI::Parse_Int(ini, nullptr, &static_cast<DrawGroupInfo *>(store)->m_drawPositionX, nullptr);
    } else {
        static_cast<DrawGroupInfo *>(store)->m_drawPositionYAbsolute = true;
        INI::Parse_Int(ini, nullptr, &static_cast<DrawGroupInfo *>(store)->m_drawPositionY, nullptr);
    }
}

/**
 * Custom %age parser for DrawGroupInfo entries.
 * Was originally parsePercentToReal
 */
void DrawGroupInfo::Parse_Percent_To_Real(INI *ini, void *formal, void *store, const void *user_data)
{
    if (!user_data) {
        static_cast<DrawGroupInfo *>(store)->m_drawPositionXAbsolute = false;
        INI::Parse_Percent_To_Real(ini, nullptr, &static_cast<DrawGroupInfo *>(store)->m_drawPositionX, nullptr);
    } else {
        static_cast<DrawGroupInfo *>(store)->m_drawPositionYAbsolute = false;
        INI::Parse_Percent_To_Real(ini, nullptr, &static_cast<DrawGroupInfo *>(store)->m_drawPositionY, nullptr);
    }
}
