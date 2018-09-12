/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Multiplayer settings handling class.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "multiplayersettings.h"
#include "gamemath.h"
#include <cstddef>

using GameMath::Floor;

#ifndef THYME_STANDALONE
MultiplayerSettings *&g_theMultiplayerSettings = Make_Global<MultiplayerSettings *>(0x00A2BE58);
#else
MultiplayerSettings *g_theMultiplayerSettings = nullptr;
#endif

FieldParse MultiplayerColorDefinition::s_colorFieldParsetable[] = {
    {"TooltipName", &INI::Parse_AsciiString, nullptr, offsetof(MultiplayerColorDefinition, m_tooltipName)},
    {"RGBColor", &INI::Parse_RGB_Color, nullptr, offsetof(MultiplayerColorDefinition, m_rgbValue)},
    {"RGBNightColor", &INI::Parse_RGB_Color, nullptr, offsetof(MultiplayerColorDefinition, m_rgbNightValue)},
    {nullptr, nullptr, nullptr, 0}
};

void MultiplayerColorDefinition::Set_Color(RGBColor rgb)
{
    m_color = (int)Floor(rgb.blue * 255.0f) | ((int)Floor(rgb.green * 255.0f) << 8) | ((int)Floor(rgb.red * 255.0f) << 16)
        | 0xFF000000;
}

void MultiplayerColorDefinition::Set_Night_Color(RGBColor rgb)
{
    m_nightColor = (int)Floor(rgb.blue * 255.0f) | ((int)Floor(rgb.green * 255.0f) << 8)
        | ((int)Floor(rgb.red * 255.0f) << 16) | 0xFF000000;
}

void MultiplayerColorDefinition::Parse_Color_Definition(INI *ini)
{
    Utf8String token = ini->Get_Next_Token();
    MultiplayerColorDefinition *def = g_theMultiplayerSettings->Find_Color_Definition(token);

    if (def == nullptr) {
        def = g_theMultiplayerSettings->New_Color_Definition(token);
    }

    ini->Init_From_INI(def, s_colorFieldParsetable);
    def->Set_Color(def->m_rgbValue);
    def->Set_Night_Color(def->m_rgbNightValue);
}

MultiplayerSettings::MultiplayerSettings() :
    m_initialCreditsMin(0),
    m_initialCreditsMax(0),
    m_startCountdownTimer(0),
    m_maxBeaconsPerPlayer(0),
    m_useShroud(true),
    m_showRandomPlayerTemplate(true),
    m_showRandomStartPos(true),
    m_showRandomColor(true),
    m_colorList(),
    m_numColors(0),
    m_colorDef1(),
    m_colorDef2(),
    m_moneyVec(),
    m_startingMoney(),
    m_moneyDefault(false)
{
}

MultiplayerColorDefinition *MultiplayerSettings::Find_Color_Definition(Utf8String name)
{
    return nullptr;
}

MultiplayerColorDefinition *MultiplayerSettings::New_Color_Definition(Utf8String name)
{
    return nullptr;
}
