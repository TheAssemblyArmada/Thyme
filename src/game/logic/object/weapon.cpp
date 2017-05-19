////////////////////////////////////////////////////////////////////////////////
//                               --  THYME  --                                //
////////////////////////////////////////////////////////////////////////////////
//
//  Project Name:: Thyme
//
//          File:: WEAPON.CPP
//
//        Author:: OmniBlade
//
//  Contributors:: 
//
//   Description:: Weapon objects.
//
//       License:: Thyme is free software: you can redistribute it and/or 
//                 modify it under the terms of the GNU General Public License 
//                 as published by the Free Software Foundation, either version 
//                 2 of the License, or (at your option) any later version.
//
//                 A full copy of the GNU General Public License can be found in
//                 LICENSE
//
////////////////////////////////////////////////////////////////////////////////
#include "weapon.h"
#include "ini.h"

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
    nullptr
};

const char *TheWeaponBonusFieldNames[] = {
    "DAMAGE",
    "RADIUS",
    "RANGE",
    "RATE_OF_FIRE",
    "PRE_ATTACK",
    nullptr
};

void WeaponBonusSet::Parse_Weapon_Bonus_Set_Ptr(INI *ini, void *formal, void *store, void const *user_data)
{
    DEBUG_LOG("Parsing weapon bonus set.\n");
    WeaponBonusSet *wbs = static_cast<WeaponBonusSet*>(store);
    int set = INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusNames);
    int field = INI::Scan_IndexList(ini->Get_Next_Token(), TheWeaponBonusFieldNames);
    DEBUG_LOG("Parsing weapon bonus set set %d, field %d.\n", set, field);
    wbs->m_bonus[set].field[field] = INI::Scan_PercentToReal(ini->Get_Next_Token());
    DEBUG_LOG("Retrieved float %f.\n", wbs->m_bonus[set].field[field]);
}
