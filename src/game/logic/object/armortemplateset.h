/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Templated information for object armor sets.
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
#include "ini.h"

class ArmorTemplate;
class DamageFX;

enum ArmorSetType
{
    ARMORSET_VETERAN,
    ARMORSET_ELITE,
    ARMORSET_HERO,
    ARMORSET_PLAYER_UPGRADE,
    ARMORSET_WEAK_VERSUS_BASEDEFENSES,
    ARMORSET_SECOND_LIFE,
    ARMORSET_CRATE_UPGRADE_ONE,
    ARMORSET_CRATE_UPGRADE_TWO,
    ARMORSET_COUNT,
};

class ArmorTemplateSet
{
public:
    ArmorTemplateSet() { Clear(); }
    void Clear();
    void Parse_Armor_Template_Set(INI *ini);
    const BitFlags<ARMORSET_COUNT> &Get_Conditions_Yes(int condition_idx) const { return m_conditions; }
    int Get_Conditions_Count() const { return 1; }
    Utf8String Get_Definition() const { return "ArmorTemplateSet"; }
    const ArmorTemplate *Get_Armor_Template() const { return m_template; }
    const DamageFX *Get_Damage_FX() const { return m_fx; }

private:
    BitFlags<ARMORSET_COUNT> m_conditions;
    ArmorTemplate *m_template;
    DamageFX *m_fx;
};
