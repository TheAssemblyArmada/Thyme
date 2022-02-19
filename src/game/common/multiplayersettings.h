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
#pragma once

#ifdef GAME_DLL
#include "hooker.h"
#endif

#include "asciistring.h"
#include "color.h"
#include "ini.h"
#include "money.h"
#include "subsysteminterface.h"
#include <map>
#include <vector>

class MultiplayerColorDefinition
{
public:
    MultiplayerColorDefinition();
    MultiplayerColorDefinition &operator=(MultiplayerColorDefinition &rval);
    void Set_Color(RGBColor rgb);
    void Set_Night_Color(RGBColor rgb);

    static void Parse_Color_Definition(INI *ini);

private:
    Utf8String m_tooltipName;
    RGBColor m_rgbValue;
    int32_t m_color;
    RGBColor m_rgbNightValue;
    int32_t m_nightColor;

    static FieldParse s_colorFieldParsetable[];
};

inline MultiplayerColorDefinition::MultiplayerColorDefinition() :
    m_tooltipName(), m_rgbValue{ 1.0f, 1.0f, 1.0f }, m_color(-1), m_rgbNightValue{ 1.0f, 1.0f, 1.0f }, m_nightColor(-1)
{
}

inline MultiplayerColorDefinition &MultiplayerColorDefinition::operator=(MultiplayerColorDefinition &rval)
{
    if (this != &rval) {
        m_tooltipName = rval.m_tooltipName;
        m_rgbValue = rval.m_rgbValue;
        m_color = rval.m_color;
        m_rgbNightValue = rval.m_rgbNightValue;
        m_nightColor = rval.m_nightColor;
    }

    return *this;
}

class MultiplayerSettings : public SubsystemInterface
{
public:
    MultiplayerSettings();

    // Subsystem interface implementation.
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    MultiplayerColorDefinition *Find_Color_Definition(Utf8String name);
    MultiplayerColorDefinition *New_Color_Definition(Utf8String name);

private:
    int32_t m_initialCreditsMin;
    int32_t m_initialCreditsMax;
    int32_t m_startCountdownTimer;
    int32_t m_maxBeaconsPerPlayer;
    bool m_useShroud;
    bool m_showRandomPlayerTemplate;
    bool m_showRandomStartPos;
    bool m_showRandomColor;
    std::map<Utf8String, MultiplayerColorDefinition> m_colorList;
    int32_t m_numColors;
    MultiplayerColorDefinition m_colorDef1;
    MultiplayerColorDefinition m_colorDef2;
    std::vector<Money> m_moneyVec;
    Money m_startingMoney;
    bool m_moneyDefault;
};

#ifdef GAME_DLL
extern MultiplayerSettings *&g_theMultiplayerSettings;
#else
extern MultiplayerSettings *g_theMultiplayerSettings;
#endif