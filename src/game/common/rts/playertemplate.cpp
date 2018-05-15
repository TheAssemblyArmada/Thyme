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

#ifndef THYME_STANDALONE
PlayerTemplateStore *&g_thePlayerTemplateStore = Make_Global<PlayerTemplateStore *>(0x00A2BDF4);
#else
PlayerTemplateStore *g_thePlayerTemplateStore = nullptr;
#endif

PlayerTemplate::PlayerTemplate() :
    m_nameKey(NAMEKEY_INVALID),
    m_preferredColor{0.0f, 0.0f, 0.0f},
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
#ifndef THYME_STANDALONE
    return Call_Method<Image *, PlayerTemplate>(0x004D2FB0, this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_Side_Icon_Image()
{
#ifndef THYME_STANDALONE
    return Call_Method<Image *, PlayerTemplate>(0x004D2FD0, this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_General_Image()
{
#ifndef THYME_STANDALONE
    return Call_Method<Image *, PlayerTemplate>(0x004D2FF0, this);
#else
    return nullptr;
#endif
}

Image *PlayerTemplate::Get_Enabled_Image()
{
#ifndef THYME_STANDALONE
    return Call_Method<Image *, PlayerTemplate>(0x004D3010, this);
#else
    return nullptr;
#endif
}

void PlayerTemplate::Parse_Production_Cost_Change(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    float value = INI::Scan_PercentToReal(ini->Get_Next_Token());
    pt->m_productionCostChanges[key] = value;
}

void PlayerTemplate::Parse_Production_Time_Change(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    float value = INI::Scan_PercentToReal(ini->Get_Next_Token());
    pt->m_productionTimeChanges[key] = value;
}

void PlayerTemplate::Parse_Production_Veterancy_Level(INI *ini, void *formal, void *store, const void *user_data)
{
    PlayerTemplate *pt = static_cast<PlayerTemplate *>(formal);
    NameKeyType key = g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token());
    VeterancyLevel value = VeterancyLevel(INI::Scan_IndexList(ini->Get_Next_Token(), g_veterancyNames));
    pt->m_productionTimeChanges[key] = value;
}

void PlayerTemplate::Parse_Start_Money(INI *ini, void *formal, void *store, const void *user_data)
{
    Money *money = static_cast<Money *>(store);
    unsigned value;
    INI::Parse_Int(ini, formal, &value, nullptr);
    money->Empty();
    money->Deposit(value, true);
}

int PlayerTemplateStore::Get_Template_Number_By_Name(AsciiString name)
{
    for (int i = 0; i < m_playerTemplates.size(); ++i) {
        if (m_playerTemplates[i].Get_Name().Compare_No_Case(name) == 0) {
            return i;
        }
    }

    return -1;
}

PlayerTemplate *PlayerTemplateStore::Find_Player_Template(NameKeyType key)
{
    static const NameKeyType _a0 = g_theNameKeyGenerator->Name_To_Key("FactionAmerica");
    static const NameKeyType _a1 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaChooseAGeneral");
    static const NameKeyType _a2 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaTankCommand");
    static const NameKeyType _a3 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaSpecialForces");
    static const NameKeyType _a4 = g_theNameKeyGenerator->Name_To_Key("FactionAmericaAirForce");
    static const NameKeyType _c0 = g_theNameKeyGenerator->Name_To_Key("FactionChina");
    static const NameKeyType _c1 = g_theNameKeyGenerator->Name_To_Key("FactionChinaChooseAGeneral");
    static const NameKeyType _c2 = g_theNameKeyGenerator->Name_To_Key("FactionChinaRedArmy");
    static const NameKeyType _c3 = g_theNameKeyGenerator->Name_To_Key("FactionChinaSpecialWeapons");
    static const NameKeyType _c4 = g_theNameKeyGenerator->Name_To_Key("FactionChinaSecretPolice");
    static const NameKeyType _g0 = g_theNameKeyGenerator->Name_To_Key("FactionGLA");
    static const NameKeyType _g1 = g_theNameKeyGenerator->Name_To_Key("FactionGLAChooseAGeneral");
    static const NameKeyType _g2 = g_theNameKeyGenerator->Name_To_Key("FactionGLATerrorCell");
    static const NameKeyType _g3 = g_theNameKeyGenerator->Name_To_Key("FactionGLABiowarCommand");
    static const NameKeyType _g4 = g_theNameKeyGenerator->Name_To_Key("FactionGLAWarlordCommand");

    if (key == _a1 || key == _a2 || key == _a3 || key == _a4) {
        key = _a0;
    } else if (key == _c1 || key == _c2 || key == _c3 || key == _c4) {
        key = _c0;
    } else if (key == _g1 || key == _g2 || key == _g3 || key == _g4) {
        key = _g0;
    }

    for (auto it = m_playerTemplates.begin(); it != m_playerTemplates.end(); ++it) {
        if (it->Check_Name_Key(key)) {
            return &(*it);
        }
    }

    return nullptr;
}

PlayerTemplate *PlayerTemplateStore::Get_Nth_Player_Template(int index)
{
    if (index < 0 || index > m_playerTemplates.size()) {
        return nullptr;
    }

    return &m_playerTemplates[index];
}

void PlayerTemplateStore::Get_All_Side_Strings(std::list<AsciiString> *list)
{
    if (list == nullptr) {
        return;
    }

    std::list<AsciiString> tmp;

    for (int i = 0; i < m_playerTemplates.size(); ++i) {
        AsciiString side_name = m_playerTemplates[i].Get_Side_Name();
        auto found = std::find(tmp.begin(), tmp.end(), side_name);

        if (found == tmp.end()) {
            tmp.push_back(side_name);
        }
    }

    *list = tmp;
}

void PlayerTemplateStore::Parse_Player_Template_Definitions(INI *ini)
{
    static FieldParse _parse_table[] = {
        { "Side", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_side) },
        { "BaseSide", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_baseSide) },
        { "PlayableSide", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_isPlayableSide) },
        { "DisplayName", &INI::Parse_And_Translate_Label, nullptr, offsetof(PlayerTemplate, m_displayName) },
        { "StartMoney", &PlayerTemplate::Parse_Start_Money, nullptr, offsetof(PlayerTemplate, m_money) },
        { "PreferredColor", &INI::Parse_RGB_Color, nullptr, offsetof(PlayerTemplate, m_preferredColor) },
        { "StartingBuilding", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingBuilding) },
        { "StartingUnit0", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[0]) },
        { "StartingUnit1", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[1]) },
        { "StartingUnit2", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[2]) },
        { "StartingUnit3", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[3]) },
        { "StartingUnit4", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[4]) },
        { "StartingUnit5", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[5]) },
        { "StartingUnit6", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[6]) },
        { "StartingUnit7", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[7]) },
        { "StartingUnit8", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[8]) },
        { "StartingUnit9", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_startingUnits[9]) },
        { "ProductionCostChange", &PlayerTemplate::Parse_Production_Cost_Change, nullptr, 0 },
        { "ProductionTimeChange", &PlayerTemplate::Parse_Production_Time_Change, nullptr, 0 },
        { "ProductionVeterancyLevel", &PlayerTemplate::Parse_Production_Veterancy_Level, nullptr, 0 },
        { "IntrinsicSciences", &INI::Parse_Science_Vector, nullptr, offsetof(PlayerTemplate, m_intrinsicSciences) },
        { "PurchaseScienceCommandSetRank1", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_purchaseCommandSetRankOne) },
        { "PurchaseScienceCommandSetRank3", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_purchaseCommandSetRankThree) },
        { "PurchaseScienceCommandSetRank8", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_purchaseCommandSetRankEight) },
        { "SpecialPowerShortcutCommandSet", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_specialPowerShortcutCommandSet) },
        { "SpecialPowerShortcutWinName", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_specialPowerShortcutWinName) },
        { "SpecialPowerShortcutButtonCount", &INI::Parse_Int, nullptr, offsetof(PlayerTemplate, m_specialPowerShortcutButtonCount) },
        { "IsObserver", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_isObserver) },
        { "OldFaction", &INI::Parse_Bool, nullptr, offsetof(PlayerTemplate, m_oldFaction) },
        { "IntrinsicSciencePurchasePoints", &INI::Parse_Int, nullptr, offsetof(PlayerTemplate, m_intrinsicSciencePurchasePoints) },
        { "ScoreScreenImage", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_scoreScreenImage) },
        { "LoadScreenImage", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_loadScreenImage) },
        { "LoadScreenMusic", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_loadScreenMusic) },
        { "ScoreScreenMusic", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_scoreScreenMusic) },
        { "HeadWaterMark", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_headWaterMark) },
        { "FlagWaterMark", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_flagWaterMark) },
        { "EnabledImage", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_enabledImage) },
        { "SideIconImage", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_sideIconImage) },
        { "GeneralImage", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_generalImage) },
        { "BeaconName", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_beaconName) },
        { "ArmyTooltip", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_armyTooltip) },
        { "Features", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_features) },
        { "MedallionRegular", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_medallionRegular) },
        { "MedallionHilite", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_medallionHilite) },
        { "MedallionSelect", &INI::Parse_AsciiString, nullptr, offsetof(PlayerTemplate, m_medallionSelect) },
        { nullptr, nullptr, nullptr, 0 }
    };

    PlayerTemplate *temp = g_thePlayerTemplateStore->Find_Player_Template(g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token()));

    if (temp != nullptr) {
        ini->Init_From_INI(temp, _parse_table);
    } else {
        PlayerTemplate new_temp;
        ini->Init_From_INI(&new_temp, _parse_table);
        g_thePlayerTemplateStore->m_playerTemplates.push_back(new_temp);
    }
}
