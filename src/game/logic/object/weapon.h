/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Weapon objects.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#pragma once

#include "asciistring.h"
#include "mempoolobj.h"

class INI;
class Coord3D;
class Object;

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

class WeaponBonus
{
public:
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

    WeaponBonus() { Clear(); }
    void Clear()
    {
        for (int i = 0; i < COUNT; i++) {
            m_field[i] = 1.0f;
        }
    }
    float Get_Field(Field f) { return m_field[f]; }
    void Set_Field(Field f, float bonus) { m_field[f] = bonus; }
    void Append_Bonuses(WeaponBonus &bonus)
    {
        for (int i = 0; i < COUNT; i++) {
            bonus.m_field[i] = m_field[i] - 1.0f + bonus.m_field[i];
        }
    }

private:
    float m_field[COUNT];
};

class WeaponBonusSet : public MemoryPoolObject
{
    IMPLEMENT_POOL(WeaponBonusSet);
    friend class GlobalData;

protected:
    virtual ~WeaponBonusSet() override {}

public:
    static void Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, void const *user_data);
    void Parse_Weapon_Bonus_Set(INI *ini);
    WeaponBonusSet() {}
    void Append_Bonuses(unsigned int flags, WeaponBonus &bonus)
    {
        if (flags) {
            for (int i = 0; i < WEAPONBONUSCONDITION_COUNT; i++) {
                if ((1 << i & flags) != 0) {
                    m_bonus[i].Append_Bonuses(bonus);
                }
            }
        }
    }

private:
    WeaponBonus m_bonus[WEAPONBONUSCONDITION_COUNT];
};

class WeaponTemplate;

class Weapon
{
public:
    bool Is_Within_Attack_Range(const Object *source, const Object *target) const;
    float Get_Attack_Range(const Object *source) const;
};

class WeaponStore
{
public:
    const WeaponTemplate *Find_Weapon_Template(Utf8String name) const;
    void Create_And_Fire_Temp_Weapon(const WeaponTemplate *tmpl, const Object *obj, const Coord3D *pos);
    static void Parse_Weapon_Template(INI *ini, void *, void *store, const void *);
};

#ifdef GAME_DLL
extern WeaponStore *&g_theWeaponStore;
#else
extern WeaponStore *g_theWeaponStore;
#endif
