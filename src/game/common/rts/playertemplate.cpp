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
#include "playertemplate.h"
#include <algorithm>
#include <cstddef>

#ifndef GAME_DLL
PlayerTemplateStore *g_thePlayerTemplateStore = nullptr;
#else
#include "hooker.h"
#endif

PlayerTemplate::PlayerTemplate() :
    m_nameKey(NAMEKEY_INVALID),
    m_preferredColor{ 0.0f, 0.0f, 0.0f },
    m_specialPowerShortcutButtonCount(0),
    m_isObserver(false),
    m_isPlayableSide(false),
    m_oldFaction(false),
    m_intrinsicSciencePurchasePoints(0)
{
}

Image *PlayerTemplate::Get_Head_Watermark_Image()
{
    // Optimised out of original Windows binary as not used in ZH. Exists in macOS build.
    return nullptr;
}

Image *PlayerTemplate::Get_Flag_Watermark_Image()
{
#ifdef GAME_DLL
    return Call_Method<Image *, PlayerTemplate>(PICK_ADDRESS(0x004D2FB0, 0), this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_Side_Icon_Image()
{
#ifdef GAME_DLL
    return Call_Method<Image *, PlayerTemplate>(PICK_ADDRESS(0x004D2FD0, 0), this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_General_Image()
{
#ifdef GAME_DLL
    return Call_Method<Image *, PlayerTemplate>(PICK_ADDRESS(0x004D2FF0, 0), this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_Enabled_Image()
{
#ifdef GAME_DLL
    return Call_Method<Image *, PlayerTemplate>(PICK_ADDRESS(0x004D3010, 0), this);
#else
    return nullptr;
#endif
}

/**
 * @brief Parses modifiers for production cost into the internal map.
 *
 * 0x004D2B50
 */
void PlayerTemplate::Parse_Production_Cost_Change(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    float value = INI::Scan_PercentToReal(ini->Get_Next_Token());
    pt->m_productionCostChanges[key] = value;
}

/**
 * @brief Parses modifiers for production time into the internal map.
 *
 * 0x004D2BF0
 */
void PlayerTemplate::Parse_Production_Time_Change(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    float value = INI::Scan_PercentToReal(ini->Get_Next_Token());
    pt->m_productionTimeChanges[key] = value;
}

/**
 * @brief Parses modifiers for veterancy level into the internal map.
 *
 * 0x004D2C90
 */
void PlayerTemplate::Parse_Production_Veterancy_Level(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    VeterancyLevel value = VeterancyLevel(INI::Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames));
    pt->m_productionTimeChanges[key] = value;
}

/**
 * @brief Parses starting money amounts.
 *
 * 0x004D2DC0
 */
void PlayerTemplate::Parse_Start_Money(INI *ini, void *formal, void *store, const void *user_data)
{
    Money *money = static_cast<Money *>(store);
    unsigned value;
    INI::Parse_Int(ini, formal, &value, nullptr);
    money->Empty();
    money->Deposit(value, true);
}

/**
 * @brief Looks up the index in the vector for the first player template with the provided name.
 *
 * 0x004D3170
 */
int PlayerTemplateStore::Get_Template_Number_By_Name(Utf8String name)
{
    for (unsigned i = 0; i < m_playerTemplates.size(); ++i) {
        if (m_playerTemplates[i].Get_Name().Compare_No_Case(name) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * @brief Finds a player template from a key.
 *
 * 0x004D32D0
 */
PlayerTemplate *PlayerTemplateStore::Find_Player_Template(NameKeyType key)
{
    static const NameKeyType _american = g_theNameKeyGenerator->Name_To_Key("FactionAmerica");
    static const NameKeyType _a1 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaChooseAGeneral");
    static const NameKeyType _a2 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaTankCommand");
    static const NameKeyType _a3 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaSpecialForces");
    static const NameKeyType _a4 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaAirForce");
    static const NameKeyType _china = g_theNameKeyGenerator->Name_To_Key("FactionChina");
    static const NameKeyType _c1 = g_theNameKeyGenerator->Name_To_Key("FactionChinaChooseAGeneral");
    static const NameKeyType _c2 = g_theNameKeyGenerator->Name_To_Key("FactionChinaRedArmy");
    static const NameKeyType _c3 = g_theNameKeyGenerator->Name_To_Key("FactionChinaSpecialWeapons");
    static const NameKeyType _c4 = g_theNameKeyGenerator->Name_To_Key("FactionChinaSecretPolice");
    static const NameKeyType _gla = g_theNameKeyGenerator->Name_To_Key("FactionGLA");
    static const NameKeyType _g1 = g_theNameKeyGenerator->Name_To_Key("FactionGLAChooseAGeneral");
    static const NameKeyType _g2 = g_theNameKeyGenerator->Name_To_Key("FactionGLATerrorCell");
    static const NameKeyType _g3 = g_theNameKeyGenerator->Name_To_Key("FactionGLABiowarCommand");
    static const NameKeyType _g4 = g_theNameKeyGenerator->Name_To_Key("FactionGLAWarlordCommand");

    // Specialist sides are converted to base side for purposes of this lookup.
    if (key == _a1 || key == _a2 || key == _a3 || key == _a4) {
        key = _american;
    } else if (key == _c1 || key == _c2 || key == _c3 || key == _c4) {
        key = _china;
    } else if (key == _g1 || key == _g2 || key == _g3 || key == _g4) {
        key = _gla;
    }

    for (auto it = m_playerTemplates.begin(); it != m_playerTemplates.end(); ++it) {
        if (it->Check_Name_Key(key)) {
            return &(*it);
        }
    }

    return nullptr;
}

/**
 * @brief Retrieves a player template from its vector index.
 *
 * 0x004D35E0
 */
PlayerTemplate *PlayerTemplateStore::Get_Nth_Player_Template(int index)
{
    if (index < 0 || unsigned(index) > m_playerTemplates.size()) {
        return nullptr;
    }

    return &m_playerTemplates[index];
}

/**
 * @brief Populates a list of all sides used by players in the store.
 *
 * 0x004D3630
 */
void PlayerTemplateStore::Get_All_Side_Strings(std::list<Utf8String> *list)
{
    if (list == nullptr) {
        return;
    }

    std::list<Utf8String> tmp;

    // Go through the template vector and add all the sides present in it to the list.
    for (unsigned i = 0; i < m_playerTemplates.size(); ++i) {
        Utf8String side_name = m_playerTemplates[i].Get_Side_Name();
        auto found = std::find(tmp.begin(), tmp.end(), side_name);

        // If a matching entry isn't found already, add this side name to the list.
        if (found == tmp.end()) {
            tmp.push_back(side_name);
        }
    }

    *list = tmp;
}

/**
 * @brief Parses player template definitions into the store from an ini file.
 * Was originally called through INI::parsePlayerTemplateDefinition
 *
 * 0x004D3860
 */
void PlayerTemplateStore::Parse_Player_Template_Definition(INI *ini)
{
    // clang-format off
    static const FieldParse _parse_table[] = { 
        FIELD_PARSE_ASCIISTRING("Side", PlayerTemplate, m_side),
        FIELD_PARSE_ASCIISTRING("BaseSide", PlayerTemplate, m_baseSide),
        { "PlayableSide", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_isPlayableSide) },
        { "DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(PlayerTemplate, m_displayName) },
        { "StartMoney", &PlayerTemplate::Parse_Start_Money, nullptr, offsetof(PlayerTemplate, m_money) },
        { "PreferredColor", &INI::Parse_RGB_Color, nullptr, offsetof(PlayerTemplate, m_preferredColor) },
        FIELD_PARSE_ASCIISTRING("StartingBuilding", PlayerTemplate, m_startingBuilding),
        FIELD_PARSE_ASCIISTRING("StartingUnit0", PlayerTemplate, m_startingUnits[0]),
        FIELD_PARSE_ASCIISTRING("StartingUnit1", PlayerTemplate, m_startingUnits[1]),
        FIELD_PARSE_ASCIISTRING("StartingUnit2", PlayerTemplate, m_startingUnits[2]),
        FIELD_PARSE_ASCIISTRING("StartingUnit3", PlayerTemplate, m_startingUnits[3]),
        FIELD_PARSE_ASCIISTRING("StartingUnit4", PlayerTemplate, m_startingUnits[4]),
        FIELD_PARSE_ASCIISTRING("StartingUnit5", PlayerTemplate, m_startingUnits[5]),
        FIELD_PARSE_ASCIISTRING("StartingUnit6", PlayerTemplate, m_startingUnits[6]),
        FIELD_PARSE_ASCIISTRING("StartingUnit7", PlayerTemplate, m_startingUnits[7]),
        FIELD_PARSE_ASCIISTRING("StartingUnit8", PlayerTemplate, m_startingUnits[8]),
        FIELD_PARSE_ASCIISTRING("StartingUnit9", PlayerTemplate, m_startingUnits[9]),
        { "ProductionCostChange", &PlayerTemplate::Parse_Production_Cost_Change, nullptr, 0 },
        { "ProductionTimeChange", &PlayerTemplate::Parse_Production_Time_Change, nullptr, 0 },
        { "ProductionVeterancyLevel", &PlayerTemplate::Parse_Production_Veterancy_Level, nullptr, 0 },
        { "IntrinsicSciences", &ScienceStore::Parse_Science_Vector, nullptr, offsetof(PlayerTemplate, m_intrinsicSciences) },
        FIELD_PARSE_ASCIISTRING("PurchaseScienceCommandSetRank1", PlayerTemplate, m_purchaseCommandSetRankOne),
        FIELD_PARSE_ASCIISTRING("PurchaseScienceCommandSetRank3", PlayerTemplate, m_purchaseCommandSetRankThree),
        FIELD_PARSE_ASCIISTRING("PurchaseScienceCommandSetRank8", PlayerTemplate, m_purchaseCommandSetRankEight),
        FIELD_PARSE_ASCIISTRING("SpecialPowerShortcutCommandSet", PlayerTemplate, m_specialPowerShortcutCommandSet),
        FIELD_PARSE_ASCIISTRING("SpecialPowerShortcutWinName", PlayerTemplate, m_specialPowerShortcutWinName),
        { "SpecialPowerShortcutButtonCount",&INI::Parse_Int, nullptr, offsetof(PlayerTemplate, m_specialPowerShortcutButtonCount) },
        { "IsObserver", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_isObserver) },
        { "OldFaction", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_oldFaction) },
        { "IntrinsicSciencePurchasePoints", &INI::Parse_Int, nullptr, offsetof(PlayerTemplate, m_intrinsicSciencePurchasePoints) },
        FIELD_PARSE_ASCIISTRING("ScoreScreenImage", PlayerTemplate, m_scoreScreenImage),
        FIELD_PARSE_ASCIISTRING("LoadScreenImage", PlayerTemplate, m_loadScreenImage),
        FIELD_PARSE_ASCIISTRING("LoadScreenMusic", PlayerTemplate, m_loadScreenMusic),
        FIELD_PARSE_ASCIISTRING("ScoreScreenMusic", PlayerTemplate, m_scoreScreenMusic),
        FIELD_PARSE_ASCIISTRING("HeadWaterMark", PlayerTemplate, m_headWaterMark),
        FIELD_PARSE_ASCIISTRING("FlagWaterMark", PlayerTemplate, m_flagWaterMark),
        FIELD_PARSE_ASCIISTRING("EnabledImage", PlayerTemplate, m_enabledImage),
        FIELD_PARSE_ASCIISTRING("SideIconImage", PlayerTemplate, m_sideIconImage),
        FIELD_PARSE_ASCIISTRING("GeneralImage", PlayerTemplate, m_generalImage),
        FIELD_PARSE_ASCIISTRING("BeaconName", PlayerTemplate, m_beaconName),
        FIELD_PARSE_ASCIISTRING("ArmyTooltip", PlayerTemplate, m_armyTooltip),
        FIELD_PARSE_ASCIISTRING("Features", PlayerTemplate, m_features),
        FIELD_PARSE_ASCIISTRING("MedallionRegular", PlayerTemplate, m_medallionRegular),
        FIELD_PARSE_ASCIISTRING("MedallionHilite", PlayerTemplate, m_medallionHilite),
        FIELD_PARSE_ASCIISTRING("MedallionSelect", PlayerTemplate, m_medallionSelect),
        FIELD_PARSE_LAST 
    };
    // clang-format on
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    PlayerTemplate *temp = g_thePlayerTemplateStore->Find_Player_Template(key);

    if (temp != nullptr) {
        ini->Init_From_INI(temp, _parse_table);
        temp->m_nameKey = key;
    } else {
        PlayerTemplate new_temp;
        ini->Init_From_INI(&new_temp, _parse_table);
        new_temp.m_nameKey = key;
        g_thePlayerTemplateStore->m_playerTemplates.push_back(new_temp);
    }
}
