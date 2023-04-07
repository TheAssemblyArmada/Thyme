/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Templated information for object weapon sets.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "weapontemplateset.h"
#include "weapon.h"
#include <cstddef>

template<>
const char *BitFlags<WEAPONSET_COUNT>::s_bitNamesList[] = { "VETERAN",
    "ELITE",
    "HERO",
    "PLAYER_UPGRADE",
    "CRATEUPGRADE_ONE",
    "CRATEUPGRADE_TWO",
    "VEHICLE_HIJACK",
    "CARBOMB",
    "MINE_CLEARING_DETAIL",
    "WEAPON_RIDER1",
    "WEAPON_RIDER2",
    "WEAPON_RIDER3",
    "WEAPON_RIDER4",
    "WEAPON_RIDER5",
    "WEAPON_RIDER6",
    "WEAPON_RIDER7",
    "WEAPON_RIDER8",
    nullptr };

static const char *s_theWeaponSlotTypeNames[] = { "PRIMARY", "SECONDARY", "TERTIARY", nullptr };

/**
 * @brief Clears the template set.
 *
 * 0x00605E00
 */
void WeaponTemplateSet::Clear()
{
    m_shareWeaponReloadTime = false;
    m_weaponLockSharedAcrossSets = false;
    m_conditions.Clear();

    for (int i = 0; i < WEAPONSLOT_COUNT; ++i) {
        m_preferredAgainst[i].Clear();
        m_autoChooseMask[i] = 0xFFFFFFFF;
        m_template[i] = nullptr;
    }
}

/**
 * @brief Checks if the set contains any weapons.
 *
 * 0x00605E40
 */
bool WeaponTemplateSet::Has_Any_Weapons() const
{
    for (int i = 0; i < WEAPONSLOT_COUNT; ++i) {
        if (m_template[i] != nullptr) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Check if this set matches a given weapon condition?
 *
 * 0x00606190
 */
bool WeaponTemplateSet::Test_Weapon_Set_Flag(WeaponSetType set) const
{
    return m_conditions.Test(set);
}

/**
 * @brief Parse a weapon template set from an ini file.
 *
 * 0x006060B0
 */
void WeaponTemplateSet::Parse_Weapon_Template_Set(INI *ini, const ThingTemplate *type)
{
    static const FieldParse _parse_table[] = {
        { "Conditions", &BitFlags<WEAPONSET_COUNT>::Parse_From_INI, nullptr, offsetof(WeaponTemplateSet, m_conditions) },
        { "Weapon", &WeaponTemplateSet::Parse_Weapon, nullptr, 0 },
        { "AutoChooseSources", &WeaponTemplateSet::Parse_Auto_Choose, nullptr, 0 },
        { "PreferredAgainst", &WeaponTemplateSet::Parse_Preferred_Against, nullptr, 0 },
        { "ShareWeaponReloadTime", &INI::Parse_Bool, nullptr, offsetof(WeaponTemplateSet, m_shareWeaponReloadTime) },
        { "WeaponLockSharedAcrossSets",
            &INI::Parse_Bool,
            nullptr,
            offsetof(WeaponTemplateSet, m_weaponLockSharedAcrossSets) },
        { nullptr, nullptr, nullptr, 0 }
    };

    ini->Init_From_INI(this, _parse_table);
    m_type = type;
}

/**
 * @brief Parse a weapon from an ini file.
 *
 * 0x00605E60
 */
void WeaponTemplateSet::Parse_Weapon(INI *ini, void *formal, void *store, const void *user_data)
{
    int index = ini->Scan_IndexList(ini->Get_Next_Token(), s_theWeaponSlotTypeNames);
    WeaponTemplateSet *tmplate = static_cast<WeaponTemplateSet *>(formal);
    WeaponStore::Parse_Weapon_Template(ini, tmplate, &tmplate->m_template[index], nullptr);
}

/**
 * @brief Parse auto choose mask from an ini file.
 *
 * 0x00605E90
 */
void WeaponTemplateSet::Parse_Auto_Choose(INI *ini, void *formal, void *store, const void *user_data)
{
    int index = INI::Scan_IndexList(ini->Get_Next_Token(), g_weaponSlotNames);
    INI::Parse_Bitstring32(
        ini, formal, &static_cast<WeaponTemplateSet *>(formal)->m_autoChooseMask[index], g_commandSourceMaskNames);
}

/**
 * @brief Parse preferred against KindOf from an ini file.
 *
 * 0x00605ED0
 */
void WeaponTemplateSet::Parse_Preferred_Against(INI *ini, void *formal, void *store, const void *user_data)
{
    int index = INI::Scan_IndexList(ini->Get_Next_Token(), g_weaponSlotNames);
    BitFlags<KINDOF_COUNT>::Parse_From_INI(
        ini, formal, &static_cast<WeaponTemplateSet *>(formal)->m_preferredAgainst[index], user_data);
}
