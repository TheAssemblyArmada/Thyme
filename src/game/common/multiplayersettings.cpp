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

#ifndef GAME_DLL
MultiplayerSettings *g_theMultiplayerSettings = nullptr;
#endif

const FieldParse MultiplayerColorDefinition::s_colorFieldParsetable[] = {
    { "TooltipName", &INI::Parse_AsciiString, nullptr, offsetof(MultiplayerColorDefinition, m_tooltipName) },
    { "RGBColor", &INI::Parse_RGB_Color, nullptr, offsetof(MultiplayerColorDefinition, m_rgbValue) },
    { "RGBNightColor", &INI::Parse_RGB_Color, nullptr, offsetof(MultiplayerColorDefinition, m_rgbNightValue) },
    { nullptr, nullptr, nullptr, 0 }
};

void MultiplayerColorDefinition::Set_Color(RGBColor rgb)
{
    m_color = rgb.Get_As_Int() | 0xFF000000;
}

void MultiplayerColorDefinition::Set_Night_Color(RGBColor rgb)
{
    m_nightColor = rgb.Get_As_Int() | 0xFF000000;
}

// Was originally INI::parseMultiplayerColorDefinition
void MultiplayerColorDefinition::Parse_Multiplayer_Color_Definition(INI *ini)
{
    Utf8String token;
    token.Set(ini->Get_Next_Token());
    MultiplayerColorDefinition *def = g_theMultiplayerSettings->Find_Multiplayer_Color_Definition_By_Name(token);

    if (def == nullptr) {
        def = g_theMultiplayerSettings->New_Multiplayer_Color_Definition(token);
    }

    ini->Init_From_INI(def, Get_Field_Parse());
    def->Set_Color(def->Get_RGB_Value());
    def->Set_Night_Color(def->Get_RGB_Night_Value());
}

MultiplayerSettings::MultiplayerSettings() :
    m_initialCreditsMin(0),
    m_initialCreditsMax(0),
    m_startCountdownTimer(0),
    m_maxBeaconsPerPlayer(3),
    m_useShroud(true),
    m_showRandomPlayerTemplate(true),
    m_showRandomStartPos(true),
    m_showRandomColor(true),
    m_numColors(0),
    m_moneyDefault(false)
{
}

MultiplayerColorDefinition *MultiplayerSettings::Find_Multiplayer_Color_Definition_By_Name(Utf8String name)
{
    for (auto i = m_colorList.begin(); i != m_colorList.end(); i++) {
        if (i->second.Get_Tooltip_Name() == name) {
            return &i->second;
        }
    }

    return nullptr;
}

MultiplayerColorDefinition *MultiplayerSettings::New_Multiplayer_Color_Definition(Utf8String name)
{
    MultiplayerColorDefinition def;
    int count = Get_Num_Colors();
    m_colorList[count] = def;
    m_numColors = m_colorList.size();
    return &m_colorList[count];
}

void MultiplayerSettings::Add_Starting_Money_Choice(const Money &money, bool is_default)
{
    m_moneyVec.push_back(money);

    if (is_default) {
        captainslog_dbgassert(!m_moneyDefault, "Cannot have more than one default MultiplayerStartingMoneyChoice");
        m_startingMoney = money;
        m_moneyDefault = true;
    }
}

class StartingMoney : public Money
{
private:
    bool m_default = false;
    friend class MultiplayerSettings;
};

void MultiplayerSettings::Parse_Multiplayer_Starting_Money_Choice_Definition(INI *ini)
{
    static const FieldParse s_startingMoneyFieldParseTable[] = { { "Value", &StartingMoney::Parse_Money_Amount, nullptr, 0 },
        { "Default", &INI::Parse_Bool, nullptr, offsetof(StartingMoney, m_default) },
        { nullptr, nullptr, nullptr, 0 } };

    captainslog_dbgassert(
        ini->Get_Load_Type() != INI_LOAD_CREATE_OVERRIDES, "Overrides not supported for MultiplayerStartingMoneyChoice");
    StartingMoney money;
    ini->Init_From_INI(&money, s_startingMoneyFieldParseTable);
    g_theMultiplayerSettings->Add_Starting_Money_Choice(money, money.m_default);
}

void MultiplayerSettings::Parse_Multiplayer_Settings_Definition(INI *ini)
{
    if (g_theMultiplayerSettings != nullptr) {
        captainslog_dbgassert(
            ini->Get_Load_Type() != INI_LOAD_CREATE_OVERRIDES, "Creating an override of MultiplayerSettings!");
    } else {
        g_theMultiplayerSettings = new MultiplayerSettings;
    }

    ini->Init_From_INI(g_theMultiplayerSettings, MultiplayerSettings::Get_Field_Parse());
}

const FieldParse MultiplayerSettings::s_multiplayerSettingsFieldParseTable[] = {
    { "StartCountdownTimer", &INI::Parse_Int, nullptr, offsetof(MultiplayerSettings, m_startCountdownTimer) },
    { "MaxBeaconsPerPlayer", &INI::Parse_Int, nullptr, offsetof(MultiplayerSettings, m_maxBeaconsPerPlayer) },
    { "UseShroud", &INI::Parse_Bool, nullptr, offsetof(MultiplayerSettings, m_useShroud) },
    { "ShowRandomPlayerTemplate", &INI::Parse_Bool, nullptr, offsetof(MultiplayerSettings, m_showRandomPlayerTemplate) },
    { "ShowRandomStartPos", &INI::Parse_Bool, nullptr, offsetof(MultiplayerSettings, m_showRandomStartPos) },
    { "ShowRandomColor", &INI::Parse_Bool, nullptr, offsetof(MultiplayerSettings, m_showRandomColor) },
    { nullptr, nullptr, nullptr, 0 }
};
