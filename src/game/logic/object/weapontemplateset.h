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
#pragma once

#include "always.h"
#include "bitflags.h"
#include "gametype.h"
#include "ini.h"
#include "kindof.h"

class ThingTemplate;
class WeaponTemplate;

enum WeaponSetType
{
    WEAPONSET_VETERAN,
    WEAPONSET_ELITE,
    WEAPONSET_HERO,
    WEAPONSET_PLAYER_UPGRADE,
    WEAPONSET_CRATEUPGRADE_ONE,
    WEAPONSET_CRATEUPGRADE_TWO,
    WEAPONSET_VEHICLE_HIJACK,
    WEAPONSET_CARBOMB,
    WEAPONSET_MINE_CLEARING_DETAIL,
    WEAPONSET_WEAPON_RIDER1,
    WEAPONSET_WEAPON_RIDER2,
    WEAPONSET_WEAPON_RIDER3,
    WEAPONSET_WEAPON_RIDER4,
    WEAPONSET_WEAPON_RIDER5,
    WEAPONSET_WEAPON_RIDER6,
    WEAPONSET_WEAPON_RIDER7,
    WEAPONSET_WEAPON_RIDER8,
    WEAPONSET_COUNT,
};

class WeaponTemplateSet
{
public:
    WeaponTemplateSet() : m_type(nullptr) { Clear(); }
    void Clear();
    bool Has_Any_Weapons() const;
    bool Test_Weapon_Set_Flag(WeaponSetType set) const;
    void Parse_Weapon_Template_Set(INI *ini, const ThingTemplate *type);

    static void Parse_Weapon(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Auto_Choose(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Preferred_Against(INI *ini, void *formal, void *store, const void *user_data);

    const BitFlags<WEAPONSET_COUNT> &Get_Conditions_Yes(int condition_idx) const { return m_conditions; }
    const BitFlags<WEAPONSET_COUNT> &Get_Conditions() const { return m_conditions; }
    int Get_Conditions_Count() const { return 1; }
    // BUGFIX : Original returned ArmorTemplateSet here, clearly a bug
    Utf8String Get_Definition() const { return "WeaponTemplateSet"; }
    bool Is_Lock_Shared_Across_Sets() const { return m_weaponLockSharedAcrossSets; }
    WeaponTemplate *Get_Nth(WeaponSlotType type) const { return m_template[type]; }
    uint32_t Get_Nth_Auto_Choose_Mask(WeaponSlotType type) const { return m_autoChooseMask[type]; }
    BitFlags<KINDOF_COUNT> Get_Nth_Preferred_Against_Mask(WeaponSlotType type) const { return m_preferredAgainst[type]; }
    const ThingTemplate *Get_Type() const { return m_type; }
    bool Is_Share_Weapon_Reload_Time() const { return m_shareWeaponReloadTime; }

private:
    const ThingTemplate *m_type;
    BitFlags<WEAPONSET_COUNT> m_conditions;
    WeaponTemplate *m_template[WEAPONSLOT_COUNT];
    uint32_t m_autoChooseMask[WEAPONSLOT_COUNT];
    BitFlags<KINDOF_COUNT> m_preferredAgainst[WEAPONSLOT_COUNT];
    bool m_shareWeaponReloadTime;
    bool m_weaponLockSharedAcrossSets;
};
