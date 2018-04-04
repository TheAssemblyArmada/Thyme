/**
 * @file
 *
 * @Author OmniBlade
 *
 * @brief Weapon objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "mempoolobj.h"

#ifndef THYME_STANDALONE
#include "hooker.h"
#endif

class INI;

enum WeaponBonusConditionType
{
    WEAPONBONUSCONDITION_INVALID = -1,
    WEAPONBONUSCONDITION_GARRISONED,
    WEAPONBONUSCONDITION_HORDE,
    WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN,
    WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST,
    WEAPONBONUSCONDITION_NATIONALISM,
    WEAPONBONUSCONDITION_PLAYER_UPGRADE,
    WEAPONBONUSCONDITION_DRONE_SPOTTING,
    WEAPONBONUSCONDITION_DEMORALIZED_OBSOLETE,
    WEAPONBONUSCONDITION_ENTHUSIASTIC,
    WEAPONBONUSCONDITION_VETERAN,
    WEAPONBONUSCONDITION_ELITE,
    WEAPONBONUSCONDITION_HERO,
    WEAPONBONUSCONDITION_BATTLEPLAN_BOMBARDMENT,
    WEAPONBONUSCONDITION_BATTLEPLAN_HOLDTHELINE,
    WEAPONBONUSCONDITION_BATTLEPLAN_SEARCHANDDESTROY,
    WEAPONBONUSCONDITION_SUBLIMINAL,
    WEAPONBONUSCONDITION_SOLO_HUMAN_EASY,
    WEAPONBONUSCONDITION_SOLO_HUMAN_NORMAL,
    WEAPONBONUSCONDITION_SOLO_HUMAN_HARD,
    WEAPONBONUSCONDITION_SOLO_AI_EASY,
    WEAPONBONUSCONDITION_SOLO_AI_NORMAL,
    WEAPONBONUSCONDITION_SOLO_AI_HARD,
    WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE,
    WEAPONBONUSCONDITION_FANATICISM,
    WEAPONBONUSCONDITION_FRENZY_ONE,
    WEAPONBONUSCONDITION_FRENZY_TWO,
    WEAPONBONUSCONDITION_FRENZY_THREE,
    WEAPONBONUSCONDITION_COUNT,
};

struct WeaponBonus
{
    enum Field
    {
        INVALID = -1,
        DAMAGE,
        RADIUS,
        RANGE,
        RATE_OF_FIRE,
        PRE_ATTACK,
        COUNT,
    };

    float field[COUNT];
};

class WeaponBonusSet : public MemoryPoolObject
{
    friend class GlobalData;

    IMPLEMENT_POOL(WeaponBonusSet);

public:
    static void Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, void const *user_data);
#ifndef THYME_STANDALONE
    static void Hook_Me();
#endif
private:
    WeaponBonus m_bonus[WEAPONBONUSCONDITION_COUNT];
};

#ifndef THYME_STANDALONE
inline void WeaponBonusSet::Hook_Me()
{
    Hook_Function(0x004C9860, Parse_Weapon_Bonus_Set_Ptr);
}
#endif