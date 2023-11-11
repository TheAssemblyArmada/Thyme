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

    const Image *Get_Head_Watermark_Image() const;
    const Image *Get_Flag_Watermark_Image() const;
    const Image *Get_Side_Icon_Image() const;
    const Image *Get_General_Image() const;
    const Image *Get_Enabled_Image() const;
    Utf8String Get_Starting_Unit(int unit) const;

    Utf8String Get_Name() const { return g_theNameKeyGenerator->Key_To_Name(m_nameKey); }
    Utf8String Get_Side() const { return m_side; }
    NameKeyType Get_Name_Key() const { return m_nameKey; }
    void Set_Name_Key(NameKeyType key) { m_nameKey = key; }
    Utf8String Get_Beacon_Name() const { return m_beaconName; }
    Utf8String Get_Purchase_Command_Set_Rank_One() const { return m_purchaseCommandSetRankOne; }
    Utf8String Get_Purchase_Command_Set_Rank_Three() const { return m_purchaseCommandSetRankThree; }
    Utf8String Get_Purchase_Command_Set_Rank_Eight() const { return m_purchaseCommandSetRankEight; }
    bool Get_Old_Faction() const { return m_oldFaction; }
    Utf16String Get_Display_Name() const { return m_displayName; }
    Utf8String Get_Starting_Building() const { return m_startingBuilding; }
    Utf8String Get_Base_Side() const { return m_baseSide; }
    const RGBColor *Get_Preferred_Color() const { return &m_preferredColor; }
    const Money *Get_Money() const { return &m_money; }
    const Handicap *Get_Handicap() const { return &m_handicap; }
    const std::map<NameKeyType, float> *Get_Production_Cost_Changes() const { return &m_productionCostChanges; }
    const std::map<NameKeyType, float> *Get_Production_Time_Changes() const { return &m_productionTimeChanges; }
    const std::vector<ScienceType> *Get_Intrinsinc_Sciences() const { return &m_intrinsicSciences; }
    int Get_Intrinsic_Science_Purchase_Points() const { return m_intrinsicSciencePurchasePoints; }
    Utf8String Get_Special_Power_Shortcut_Win_Name() const { return m_specialPowerShortcutWinName; }
    int Get_Special_Power_Shortcut_Button_Count() const { return m_specialPowerShortcutButtonCount; }
    Utf8String Get_Special_Power_Shortcut_Command_Set() const { return m_specialPowerShortcutCommandSet; }

    const std::map<NameKeyType, VeterancyLevel> *Get_Production_Veterancy_Levels() const
    {
        return &m_productionVeterancyLevels;
    }

    bool Is_Playable_Side() const { return m_isPlayableSide; }
    bool Is_Observer() const { return m_isObserver; }

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
    std::map<NameKeyType, float> m_productionCostChanges;
    std::map<NameKeyType, float> m_productionTimeChanges;
    std::map<NameKeyType, VeterancyLevel> m_productionVeterancyLevels;
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
    int Get_Player_Template_Count() const { return m_playerTemplates.size(); }

    static void Parse_Player_Template_Definition(INI *ini);

private:
    std::vector<PlayerTemplate> m_playerTemplates;
};

#ifdef GAME_DLL
extern PlayerTemplateStore *&g_thePlayerTemplateStore;
#else
extern PlayerTemplateStore *g_thePlayerTemplateStore;
#endif
