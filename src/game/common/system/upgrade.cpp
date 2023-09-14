/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Upgrade
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "upgrade.h"
#include "ingameui.h"
#include "player.h"

#ifndef GAME_DLL
UpgradeCenter *g_theUpgradeCenter;
#endif

UpgradeCenter::UpgradeCenter() : m_upgradeList(nullptr), m_upgradeCount(0), m_buttonImagesCached(false) {}

UpgradeCenter::~UpgradeCenter()
{
    while (m_upgradeList != nullptr) {
        UpgradeTemplate *tmplate = m_upgradeList->Friend_Get_Next();
        m_upgradeList->Delete_Instance();
        m_upgradeList = tmplate;
    }
}

void UpgradeCenter::Init()
{
    New_Upgrade("")->Friend_Make_Veterancy_Upgrade(VETERANCY_VETERAN);
    New_Upgrade("")->Friend_Make_Veterancy_Upgrade(VETERANCY_ELITE);
    New_Upgrade("")->Friend_Make_Veterancy_Upgrade(VETERANCY_HEROIC);
}

void UpgradeCenter::Reset()
{
    if (g_theMappedImageCollection != nullptr && !m_buttonImagesCached) {
        for (UpgradeTemplate *upgrade = m_upgradeList; upgrade != nullptr; upgrade = upgrade->Friend_Get_Next()) {
            upgrade->Cache_Button_Image();
        }

        m_buttonImagesCached = true;
    }
}

const UpgradeTemplate *UpgradeCenter::Find_Upgrade(const Utf8String &name)
{
    return Find_Upgrade_By_Key(g_theNameKeyGenerator->Name_To_Key(name.Str()));
}

Utf8String Get_Vet_Upgrade_Name(VeterancyLevel level)
{
    Utf8String str;
    str = "Upgrade_Veterancy_";
    str.Concat(g_veterancyNames[level]);
    return str;
}

const UpgradeTemplate *UpgradeCenter::Find_Veterancy_Upgrade(VeterancyLevel level)
{
    return Find_Upgrade(Get_Vet_Upgrade_Name(level));
}

const UpgradeTemplate *UpgradeCenter::Find_Upgrade_By_Key(NameKeyType key)
{
    for (UpgradeTemplate *tmplate = m_upgradeList; tmplate != nullptr; tmplate = tmplate->Friend_Get_Next()) {
        if (tmplate->Get_Name_Key() == key) {
            return tmplate;
        }
    }

    return nullptr;
}

UpgradeTemplate *UpgradeCenter::Find_Non_Const_Upgrade_By_Key(NameKeyType key)
{
    for (UpgradeTemplate *tmplate = m_upgradeList; tmplate != nullptr; tmplate = tmplate->Friend_Get_Next()) {
        if (tmplate->Get_Name_Key() == key) {
            return tmplate;
        }
    }

    return nullptr;
}

Upgrade::Upgrade(const UpgradeTemplate *upgrade_template) :
    m_template(upgrade_template), m_status(UPGRADE_STATUS_INVALID), m_next(nullptr), m_prev(nullptr)
{
}

void Upgrade::Xfer_Snapshot(Xfer *xfer)
{
    unsigned char version = 1;
    xfer->xferVersion(&version, 1);
    xfer->xferUser(&m_status, sizeof(m_status));
}

bool UpgradeCenter::Can_Afford_Upgrade(Player *player, const UpgradeTemplate *upgrade, bool show_message)
{
    if (player == nullptr || upgrade == nullptr) {
        return false;
    }

    if (player->Get_Money()->Count_Money() >= static_cast<unsigned int>(upgrade->Calc_Cost_To_Build(player))) {
        return true;
    }

    if (show_message) {
        g_theInGameUI->Message("GUI:NotEnoughMoneyToUpgrade");
    }

    return false;
}

UpgradeTemplate *UpgradeCenter::New_Upgrade(const Utf8String &name)
{
    UpgradeTemplate *upgrade = new UpgradeTemplate();
    const UpgradeTemplate *default_upgrade = Find_Upgrade("DefaultUpgrade");

    if (default_upgrade != nullptr) {
        *upgrade = *default_upgrade;
    }

    upgrade->Set_Name(name);
    upgrade->Set_Name_Key(g_theNameKeyGenerator->Name_To_Key(name.Str()));
    BitFlags<128> mask;
    mask.Set(m_upgradeCount, true);
    m_upgradeCount++;
    captainslog_dbgassert(m_upgradeCount < 128, "Can't have over %d types of Upgrades and have a Bitfield function.", 128);
    upgrade->Set_Upgrade_Mask(mask);
    Link_Upgrade(upgrade);
    return upgrade;
}

void UpgradeCenter::Link_Upgrade(UpgradeTemplate *upgrade)
{
    if (upgrade != nullptr) {
        upgrade->Friend_Set_Prev(nullptr);
        upgrade->Friend_Set_Next(m_upgradeList);

        if (m_upgradeList != nullptr) {
            m_upgradeList->Friend_Set_Prev(upgrade);
        }

        m_upgradeList = upgrade;
    }
}

void UpgradeCenter::Unlink_Upgrade(UpgradeTemplate *upgrade)
{
    if (upgrade != nullptr) {
        if (upgrade->Friend_Get_Next() != nullptr) {
            upgrade->Friend_Get_Next()->Friend_Set_Prev(upgrade->Friend_Get_Prev());
        }

        if (upgrade->Friend_Get_Prev() != nullptr) {
            upgrade->Friend_Get_Prev()->Friend_Set_Next(upgrade->Friend_Get_Next());
        } else {
            m_upgradeList = upgrade->Friend_Get_Next();
        }
    }
}

std::vector<Utf8String> UpgradeCenter::Get_Upgrade_Names()
{
    std::vector<Utf8String> names;

    for (UpgradeTemplate *tmplate = m_upgradeList; tmplate != nullptr; tmplate = tmplate->Friend_Get_Next()) {
        names.push_back(tmplate->Get_Name());
    }

    return names;
}

void UpgradeCenter::Parse_Upgrade_Definition(INI *ini)
{
    Utf8String str = ini->Get_Next_Token();
    UpgradeTemplate *upgrade =
        g_theUpgradeCenter->Find_Non_Const_Upgrade_By_Key(g_theNameKeyGenerator->Name_To_Key(str.Str()));

    if (upgrade == nullptr) {
        upgrade = g_theUpgradeCenter->New_Upgrade(str);
    }

    captainslog_dbgassert(upgrade != nullptr, "Parse_Upgrade_Definition: Unable to allocate upgrade '%s'", str.Str());
    ini->Init_From_INI(upgrade, UpgradeTemplate::Get_Field_Parse());
}

UpgradeTemplate::UpgradeTemplate() :
    m_type(UPGRADE_TYPE_PLAYER),
    m_nameKey(NAMEKEY_INVALID),
    m_buildTime(0.0f),
    m_cost(0),
    m_academyClassify(ACT_NONE),
    m_next(nullptr),
    m_prev(nullptr),
    m_buttonImage(nullptr)
{
}

int UpgradeTemplate::Calc_Time_To_Build(Player *player) const
{
#ifdef GAME_DEBUG_STRUCTS
    if (player->Is_Instant_Build()) {
        return 1;
    } else
#endif
    {
        return m_buildTime * 30.0f;
    }
}

void UpgradeTemplate::Friend_Make_Veterancy_Upgrade(VeterancyLevel level)
{
    m_type = UPGRADE_TYPE_OBJECT;
    m_name = Get_Vet_Upgrade_Name(level);
    m_nameKey = g_theNameKeyGenerator->Name_To_Key(m_name.Str());
    m_displayNameLabel.Clear();
    m_buildTime = 0.0f;
    m_cost = 0;
}

void UpgradeTemplate::Cache_Button_Image()
{
    if (m_buttonImageName.Is_Not_Empty()) {
        m_buttonImage = g_theMappedImageCollection->Find_Image_By_Name(m_buttonImageName);
        captainslog_dbgassert(m_buttonImage,
            "UpgradeTemplate: %s is looking for button image %s but can't find it. Skipping...",
            m_name.Str(),
            m_buttonImageName.Str());
        m_buttonImageName.Clear();
    }
}

static const char *s_upgradeTypeNames[] = { "PLAYER", "OBJECT", nullptr };
static const char *s_academyClassificationTypeNames[] = { "ACT_NONE", "ACT_UPGRADE_RADAR", "ACT_SUPERPOWER", nullptr };

// clang-format off
const FieldParse UpgradeTemplate::s_upgradeFieldParseTable[] = {
    { "DisplayName", &INI::Parse_AsciiString, nullptr, offsetof(UpgradeTemplate, m_displayNameLabel) },
    { "Type", &INI::Parse_Index_List, s_upgradeTypeNames, offsetof(UpgradeTemplate, m_type) },
    { "BuildTime", &INI::Parse_Real, nullptr, offsetof(UpgradeTemplate, m_buildTime) },
    { "BuildCost", &INI::Parse_Int, nullptr, offsetof(UpgradeTemplate, m_cost) },
    { "ButtonImage", &INI::Parse_AsciiString, nullptr, offsetof(UpgradeTemplate, m_buttonImageName) },
    { "ResearchSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(UpgradeTemplate, m_researchSound) },
    { "UnitSpecificSound", &INI::Parse_Audio_Event_RTS, nullptr, offsetof(UpgradeTemplate, m_unitSpecificSound) },
    { "AcademyClassify", &INI::Parse_Index_List, s_academyClassificationTypeNames, offsetof(UpgradeTemplate, m_academyClassify) },
    {nullptr, nullptr, nullptr, 0}
};
// clang-format on
