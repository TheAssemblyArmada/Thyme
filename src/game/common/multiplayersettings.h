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

#include "always.h"
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

    static void Parse_Multiplayer_Color_Definition(INI *ini);
    Utf8String Get_Tooltip_Name() const { return m_tooltipName; }
    RGBColor Get_RGB_Value() const { return m_rgbValue; }
    int Get_Color() const { return m_color; }
    RGBColor Get_RGB_Night_Value() const { return m_rgbNightValue; }
    int Get_Night_Color() const { return m_nightColor; }
    static const FieldParse *Get_Field_Parse() { return s_colorFieldParsetable; }

private:
    Utf8String m_tooltipName;
    RGBColor m_rgbValue;
    int m_color;
    RGBColor m_rgbNightValue;
    int m_nightColor;

    static const FieldParse s_colorFieldParsetable[];
};

inline MultiplayerColorDefinition::MultiplayerColorDefinition() : m_color(-1), m_nightColor(-1)
{
    m_tooltipName.Clear();
    m_rgbValue.Set_From_Int(-1);
    m_rgbNightValue = m_rgbValue;
}

inline MultiplayerColorDefinition &MultiplayerColorDefinition::operator=(MultiplayerColorDefinition &rval)
{
    if (this != &rval) {
        m_tooltipName = rval.Get_Tooltip_Name();
        m_rgbValue = rval.Get_RGB_Value();
        m_color = rval.Get_Color();
        m_rgbNightValue = rval.Get_RGB_Night_Value();
        m_nightColor = rval.Get_Night_Color();
    }

    return *this;
}

class MultiplayerSettings : public SubsystemInterface
{
public:
    MultiplayerSettings();

#ifdef GAME_DLL
    MultiplayerSettings *Hook_Ctor() { return new (this) MultiplayerSettings(); }
#endif

    // Subsystem interface implementation.
    virtual ~MultiplayerSettings() override {}
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    MultiplayerColorDefinition *Find_Multiplayer_Color_Definition_By_Name(Utf8String name);
    MultiplayerColorDefinition *New_Multiplayer_Color_Definition(Utf8String name);
    void Add_Starting_Money_Choice(const Money &money, bool is_default);

    static void Parse_Multiplayer_Starting_Money_Choice_Definition(INI *ini);
    static void Parse_Multiplayer_Settings_Definition(INI *ini);
    static const FieldParse *Get_Field_Parse() { return s_multiplayerSettingsFieldParseTable; }

    int Get_Max_Beacons_Per_Player() const { return m_maxBeaconsPerPlayer; }
    const std::vector<Money> *Get_Money_Vector() const { return &m_moneyVec; }
    int Get_Start_Countdown_Timer() const { return m_startCountdownTimer; }
    bool Is_Use_Shroud() const { return m_useShroud; }
    bool Is_Show_Random_Player_Template() const { return m_showRandomPlayerTemplate; }
    bool Is_Show_Random_Color() const { return m_showRandomColor; }
    bool Is_Show_Random_Start_Pos() const { return m_showRandomStartPos; }

    int Get_Num_Colors()
    {
        if (m_numColors == 0) {
            m_numColors = m_colorList.size();
        }

        return m_numColors;
    }

    Money *Get_Starting_Money()
    {
        captainslog_dbgassert(m_moneyDefault, "You must specify a default starting money amount in multiplayer.ini");
        return &m_startingMoney;
    }

    MultiplayerColorDefinition *Get_Color(int color)
    {
        if (color == -1) {
            return &m_colorDef2;
        }

        if (color == -2) {
            return &m_colorDef1;
        }

        if (color >= 0) {
            if (color < Get_Num_Colors()) {
                return &m_colorList[color];
            }
        }

        return nullptr;
    }

private:
    int m_initialCreditsMin;
    int m_initialCreditsMax;
    int m_startCountdownTimer;
    int m_maxBeaconsPerPlayer;
    bool m_useShroud;
    bool m_showRandomPlayerTemplate;
    bool m_showRandomStartPos;
    bool m_showRandomColor;
    std::map<int, MultiplayerColorDefinition> m_colorList;
    int m_numColors;
    MultiplayerColorDefinition m_colorDef1;
    MultiplayerColorDefinition m_colorDef2;
    std::vector<Money> m_moneyVec;
    Money m_startingMoney;
    bool m_moneyDefault;

    static const FieldParse s_multiplayerSettingsFieldParseTable[];
};

#ifdef GAME_DLL
extern MultiplayerSettings *&g_theMultiplayerSettings;
#else
extern MultiplayerSettings *g_theMultiplayerSettings;
#endif