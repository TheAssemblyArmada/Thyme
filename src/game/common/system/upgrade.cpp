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
#ifdef GAME_DLL
    Call_Method<void, UpgradeCenter>(PICK_ADDRESS(0x004AEF70, 0x00728CDD), this);
#endif
}

void UpgradeCenter::Reset()
{
#ifdef GAME_DLL
    Call_Method<void, UpgradeCenter>(PICK_ADDRESS(0x004AF190, 0x00728DBD), this);
#endif
}

UpgradeTemplate *UpgradeCenter::Find_Upgrade(const Utf8String &name)
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

UpgradeTemplate *UpgradeCenter::Find_Veterancy_Upgrade(VeterancyLevel level)
{
    return Find_Upgrade(Get_Vet_Upgrade_Name(level));
}

UpgradeTemplate *UpgradeCenter::Find_Upgrade_By_Key(NameKeyType key)
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

    if (player->Get_Money()->Count_Money() >= upgrade->Calc_Cost_To_Build(player)) {
        return true;
    }

    if (show_message) {
        g_theInGameUI->Message("GUI:NotEnoughMoneyToUpgrade");
    }

    return false;
}
