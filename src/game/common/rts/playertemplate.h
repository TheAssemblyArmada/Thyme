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
#include "money.h"
#include "namekeygenerator.h"
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
    enum
    {
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
    Utf8String Get_Name() { return g_theNameKeyGenerator->Key_To_Name(m_nameKey); }
    Utf8String Get_Side_Name() { return m_side; }
    bool Check_Name_Key(NameKeyType key) { return key == m_nameKey; }
    Utf8String Get_Beacon_Name() const { return m_beaconName; }

    static void Parse_Production_Cost_Change(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Production_Time_Change(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Production_Veterancy_Level(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Start_Money(INI *ini, void *formal, void *store, const void *user_data);

private:
    NameKeyType m_nameKey;
    Utf16String m_displayName;
    Utf8String m_side;
    Utf8String m_baseSide;
    Handicap m_handicap;
    Money m_money;
    RGBColor m_preferredColor;
    Utf8String m_startingBuilding;
    Utf8String m_startingUnits[STARTING_UNIT_COUNT];
    std::map<const NameKeyType, float> m_productionCostChanges;
    std::map<const NameKeyType, float> m_productionTimeChanges;
    std::map<const NameKeyType, VeterancyLevel> m_productionVeterancyLevels;
    std::vector<ScienceType> m_intrinsicSciences;
    Utf8String m_purchaseCommandSetRankOne;
    Utf8String m_purchaseCommandSetRankThree;
    Utf8String m_purchaseCommandSetRankEight;
    Utf8String m_specialPowerShortcutCommandSet;
    Utf8String m_specialPowerShortcutWinName;
    int m_specialPowerShortcutButtonCount;
    Utf8String m_loadScreenMusic;
    Utf8String m_scoreScreenMusic;
    Utf8String m_armyTooltip;
    bool m_isObserver;
    bool m_isPlayableSide;
    bool m_oldFaction;
    int m_intrinsicSciencePurchasePoints;
    Utf8String m_scoreScreenImage;
    Utf8String m_loadScreenImage;
    Utf8String m_headWaterMark;
    Utf8String m_flagWaterMark;
    Utf8String m_enabledImage;
    Utf8String m_sideIconImage;
    Utf8String m_generalImage;
    Utf8String m_beaconName;
    Utf8String m_features;
    Utf8String m_medallionRegular;
    Utf8String m_medallionHilite;
    Utf8String m_medallionSelect;
};

class PlayerTemplateStore : public SubsystemInterface
{
public:
    PlayerTemplateStore() {}
    virtual ~PlayerTemplateStore() {}

    virtual void Init() override { m_playerTemplates.clear(); }
    virtual void Reset() override {}
    virtual void Update() override {}

    int Get_Template_Number_By_Name(Utf8String name);
    PlayerTemplate *Find_Player_Template(NameKeyType key);
    PlayerTemplate *Get_Nth_Player_Template(int index);
    void Get_All_Side_Strings(std::list<Utf8String> *list);

    static void Parse_Player_Template_Definition(INI *ini);

private:
    std::vector<PlayerTemplate> m_playerTemplates;
};

#ifdef GAME_DLL
extern PlayerTemplateStore *&g_thePlayerTemplateStore;
#else
extern PlayerTemplateStore *g_thePlayerTemplateStore;
#endif
