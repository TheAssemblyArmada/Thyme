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
#include "weapon.h"
#include "ini.h"

#ifndef GAME_DLL
WeaponStore *g_theWeaponStore = nullptr;
#endif

const char *TheWeaponBonusNames[] = {
    "GARRISONED",
    "HORDE",
    "CONTINUOUS_FIRE_MEAN",
    "CONTINUOUS_FIRE_FAST",
    "NATIONALISM",
    "PLAYER_UPGRADE",
    "DRONE_SPOTTING",
    "DEMORALIZED_OBSOLETE",
    "ENTHUSIASTIC",
    "VETERAN",
    "ELITE",
    "HERO",
    "BATTLEPLAN_BOMBARDMENT",
    "BATTLEPLAN_HOLDTHELINE",
    "BATTLEPLAN_SEARCHANDDESTROY",
    "SUBLIMINAL",
    "SOLO_HUMAN_EASY",
    "SOLO_HUMAN_NORMAL",
    "SOLO_HUMAN_HARD",
    "SOLO_AI_EASY",
    "SOLO_AI_NORMAL",
    "SOLO_AI_HARD",
    "TARGET_FAERIE_FIRE",
    "FANATICISM",
    "FRENZY_ONE",
    "FRENZY_TWO",
    "FRENZY_THREE",
    nullptr,
};

const char *TheWeaponBonusFieldNames[] = {
    "DAMAGE",
    "RADIUS",
    "RANGE",
    "RATE_OF_FIRE",
    "PRE_ATTACK",
    nullptr,
};

void WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, void const *user_data)
{
    WeaponBonusSet *wbs = *static_cast<WeaponBonusSet **>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, void const *user_data)
{
    WeaponBonusSet *wbs = static_cast<WeaponBonusSet *>(store);
    wbs->Parse_Weapon_Bonus_Set(ini);
}

void WeaponBonusSet::Parse_Weapon_Bonus_Set(INI *ini)
{
    int set = INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusNames);
    WeaponBonus::Field field = WeaponBonus::Field(INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusFieldNames));
    m_bonus[set].Set_Field(field, INI::Scan_PercentToReal(ini->Get_Next_Token()));
}

const WeaponTemplate *WeaponStore::Find_Weapon_Template(Utf8String name) const
{
#ifdef GAME_DLL
    return Call_Method<const WeaponTemplate *, const WeaponStore, Utf8String>(
        PICK_ADDRESS(0x004C51B0, 0x006D6703), this, name);
#else
    return nullptr;
#endif
}

void WeaponStore::Create_And_Fire_Temp_Weapon(const WeaponTemplate *tmpl, const Object *obj, const Coord3D *pos)
{
#ifdef GAME_DLL
    Call_Method<void, WeaponStore, const WeaponTemplate *, const Object *, const Coord3D *>(
        PICK_ADDRESS(0x004C50A0, 0x006D6661), this, tmpl, obj, pos);
#endif
}

void WeaponStore::Parse_Weapon_Template(INI *ini, void *, void *store, const void *)
{
    const char *name = ini->Get_Next_Token();
    const WeaponTemplate *w = g_theWeaponStore->Find_Weapon_Template(name);
    captainslog_dbgassert(w != nullptr && !strcasecmp(name, "None"), "WeaponTemplate %s not found!", name);
    *static_cast<const WeaponTemplate **>(store) = w;
}

bool Weapon::Is_Within_Attack_Range(const Object *source, const Object *target) const
{
#ifdef GAME_DLL
    return Call_Method<bool, const Weapon, const Object *, const Object *>(
        PICK_ADDRESS(0x004C72A0, 0x006D7E7F), this, source, target);
#else
    return false;
#endif
}

float Weapon::Get_Attack_Range(const Object *source) const
{
#ifdef GAME_DLL
    return Call_Method<float, const Weapon, const Object *>(PICK_ADDRESS(0x004C77A0, 0x006D843A), this, source);
#else
    return 0;
#endif
}
