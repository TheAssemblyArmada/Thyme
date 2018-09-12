/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Class holding player template information.
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
#include "gametype.h"
#include "handicap.h"
#include "namekeygenerator.h"
#include "money.h"
#include "science.h"
#include "subsysteminterface.h"
#include "unicodestring.h"
#include <list>
#include <map>
#include <vector>

class INI;
class Image;

class PlayerTemplate
{
    enum {
        STARTING_UNIT_COUNT = 10,
    };

    friend class PlayerTemplateStore;
public:
    PlayerTemplate();
    ~PlayerTemplate() {}

    Image *Get_Head_Watermark_Image();
    Image *Get_Flag_Watermark_Image();
    Image *Get_Side_Icon_Image();
    Image *Get_General_Image();
    Image *Get_Enabled_Image();
    AsciiString Get_Name() { return g_theNameKeyGenerator->Key_To_Name(m_nameKey); }
    AsciiString Get_Side_Name() { return m_side; }
    bool Check_Name_Key(NameKeyType key) { return key == m_nameKey; }

    static void Parse_Production_Cost_Change(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Production_Time_Change(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Production_Veterancy_Level(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Start_Money(INI *ini, void *formal, void *store, const void *user_data);

private:
    NameKeyType m_nameKey;
    Utf16String m_displayName;
    AsciiString m_side;
    AsciiString m_baseSide;
    Handicap m_handicap;
    Money m_money;
    RGBColor m_preferredColor;
    AsciiString m_startingBuilding;
    AsciiString m_startingUnits[STARTING_UNIT_COUNT];
    std::map<const NameKeyType, float> m_productionCostChanges;
    std::map<const NameKeyType, float> m_productionTimeChanges;
    std::map<const NameKeyType, VeterancyLevel> m_productionVeterancyLevels;
    std::vector<ScienceType> m_intrinsicSciences;
    AsciiString m_purchaseCommandSetRankOne;
    AsciiString m_purchaseCommandSetRankThree;
    AsciiString m_purchaseCommandSetRankEight;
    AsciiString m_specialPowerShortcutCommandSet;
    AsciiString m_specialPowerShortcutWinName;
    int m_specialPowerShortcutButtonCount;
    AsciiString m_loadScreenMusic;
    AsciiString m_scoreScreenMusic;
    AsciiString m_armyTooltip;
    bool m_isObserver;
    bool m_isPlayableSide;
    bool m_oldFaction;
    int m_intrinsicSciencePurchasePoints;
    AsciiString m_scoreScreenImage;
    AsciiString m_loadScreenImage;
    AsciiString m_headWaterMark;
    AsciiString m_flagWaterMark;
    AsciiString m_enabledImage;
    AsciiString m_sideIconImage;
    AsciiString m_generalImage;
    AsciiString m_beaconName;
    AsciiString m_features;
    AsciiString m_medallionRegular;
    AsciiString m_medallionHilite;
    AsciiString m_medallionSelect;
};

class PlayerTemplateStore : public SubsystemInterface
{
public:
    PlayerTemplateStore() {}
    virtual ~PlayerTemplateStore() {}

    virtual void Init() override { m_playerTemplates.clear(); }
    virtual void Reset() override {}
    virtual void Update() override {}

    int Get_Template_Number_By_Name(AsciiString name);
    PlayerTemplate *Find_Player_Template(NameKeyType key);
    PlayerTemplate *Get_Nth_Player_Template(int index);
    void Get_All_Side_Strings(std::list<AsciiString> *list);

    static void Parse_Player_Template_Definitions(INI *ini);

#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif

private:
    std::vector<PlayerTemplate> m_playerTemplates;
};

#ifndef THYME_STANDALONE
#include "hooker.h"

extern PlayerTemplateStore *&g_thePlayerTemplateStore;

inline void PlayerTemplateStore::Hook_Me()
{
    Hook_Method(0x004D3170, &Get_Template_Number_By_Name);
    Hook_Method(0x004D32D0, &Find_Player_Template);
    Hook_Method(0x004D35E0, &Get_Nth_Player_Template);
    Hook_Method(0x004D3630, &Get_All_Side_Strings);
    Hook_Function(0x004D3860, &Parse_Player_Template_Definitions);
}
#else
extern PlayerTemplateStore *g_thePlayerTemplateStore;
#endif
