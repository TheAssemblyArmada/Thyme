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

#ifndef ARMORTEMPLATESET_H
#define ARMORTEMPLATESET_H

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
    void Parse_Armor_Template_Set(INI *ini);

private:
    BitFlags<ARMORSET_COUNT> m_conditions;
    ArmorTemplate *m_template;
    DamageFX *m_fx;
};

#endif // ARMORTEMPLATESET_H
