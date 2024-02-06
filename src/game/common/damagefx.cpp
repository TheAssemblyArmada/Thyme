/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Damage FX
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "damagefx.h"
#include "fxlist.h"

#ifndef GAME_DLL
DamageFXStore *g_theDamageFXStore;
#endif

const FieldParse *DamageFX::Get_Field_Parse()
{
    static FieldParse myFieldParse[] = { { "AmountForMajorFX", &DamageFX::Parse_Amount, nullptr, 0 },
        { "MajorFX", &DamageFX::Parse_Major_FX_List, nullptr, 0 },
        { "MinorFX", &DamageFX::Parse_Minor_FX_List, nullptr, 0 },
        { "ThrottleTime", &DamageFX::Parse_Time, nullptr, 0 },
        { "VeterancyAmountForMajorFX", &DamageFX::Parse_Amount, g_veterancyNames, 0 },
        { "VeterancyMajorFX", &DamageFX::Parse_Major_FX_List, g_veterancyNames, 0 },
        { "VeterancyMinorFX", &DamageFX::Parse_Minor_FX_List, g_veterancyNames, 0 },
        { "VeterancyThrottleTime", &DamageFX::Parse_Time, g_veterancyNames, 0 },
        { nullptr, nullptr, nullptr, 0 } };

    return myFieldParse;
}

void Parse_Common_Stuff(INI *ini,
    const char *const *names,
    VeterancyLevel &min_veterancy_level,
    VeterancyLevel &max_veterancy_level,
    DamageType &min_damage_type,
    DamageType &max_damage_type)
{
    if (names != nullptr) {
        min_veterancy_level = static_cast<VeterancyLevel>(ini->Scan_IndexList(ini->Get_Next_Token(), names));
        max_veterancy_level = min_veterancy_level;
    } else {
        min_veterancy_level = VETERANCY_REGULAR;
        max_veterancy_level = VETERANCY_HEROIC;
    }

    const char *damage = ini->Get_Next_Token();

    if (strcasecmp(damage, "Default") == 0) {
        min_damage_type = DAMAGE_EXPLOSION;
        max_damage_type = DAMAGE_STATUS;
    } else {
        min_damage_type = static_cast<DamageType>(BitFlags<DAMAGE_NUM_TYPES>::Get_Single_Bit_From_Name(damage));
        max_damage_type = min_damage_type;
    }
}

void DamageFX::Parse_Amount(INI *ini, void *formal, void *store, const void *user_data)
{
    VeterancyLevel min_veterancy_level;
    VeterancyLevel max_veterancy_level;
    DamageType min_damage_type;
    DamageType max_damage_type;

    Parse_Common_Stuff(ini,
        static_cast<const char *const *>(user_data),
        min_veterancy_level,
        max_veterancy_level,
        min_damage_type,
        max_damage_type);

    float amount = INI::Scan_Real(ini->Get_Next_Token());

    for (int i = min_damage_type; i <= max_damage_type; i++) {
        for (int j = min_veterancy_level; j <= max_veterancy_level; j++) {
            static_cast<DamageFX *>(formal)->m_dfx[i][j].m_amount = amount;
        }
    }
}

void DamageFX::Parse_Major_FX_List(INI *ini, void *formal, void *store, const void *user_data)
{
    VeterancyLevel min_veterancy_level;
    VeterancyLevel max_veterancy_level;
    DamageType min_damage_type;
    DamageType max_damage_type;

    Parse_Common_Stuff(ini,
        static_cast<const char *const *>(user_data),
        min_veterancy_level,
        max_veterancy_level,
        min_damage_type,
        max_damage_type);

    FXList *list;
    FXList::Parse(ini, nullptr, &list, nullptr);

    for (int i = min_damage_type; i <= max_damage_type; i++) {
        for (int j = min_veterancy_level; j <= max_veterancy_level; j++) {
            static_cast<DamageFX *>(formal)->m_dfx[i][j].m_majorFXList = list;
        }
    }
}

void DamageFX::Parse_Minor_FX_List(INI *ini, void *formal, void *store, const void *user_data)
{
    VeterancyLevel min_veterancy_level;
    VeterancyLevel max_veterancy_level;
    DamageType min_damage_type;
    DamageType max_damage_type;

    Parse_Common_Stuff(ini,
        static_cast<const char *const *>(user_data),
        min_veterancy_level,
        max_veterancy_level,
        min_damage_type,
        max_damage_type);

    FXList *list;
    FXList::Parse(ini, nullptr, &list, nullptr);

    for (int i = min_damage_type; i <= max_damage_type; i++) {
        for (int j = min_veterancy_level; j <= max_veterancy_level; j++) {
            static_cast<DamageFX *>(formal)->m_dfx[i][j].m_minorFXList = list;
        }
    }
}

void DamageFX::Parse_Time(INI *ini, void *formal, void *store, const void *user_data)
{
    VeterancyLevel min_veterancy_level;
    VeterancyLevel max_veterancy_level;
    DamageType min_damage_type;
    DamageType max_damage_type;

    Parse_Common_Stuff(ini,
        static_cast<const char *const *>(user_data),
        min_veterancy_level,
        max_veterancy_level,
        min_damage_type,
        max_damage_type);

    unsigned int time;
    INI::Parse_Duration_Unsigned_Int(ini, nullptr, &time, nullptr);

    for (int i = min_damage_type; i <= max_damage_type; i++) {
        for (int j = min_veterancy_level; j <= max_veterancy_level; j++) {
            static_cast<DamageFX *>(formal)->m_dfx[i][j].m_time = time;
        }
    }
}

const DamageFX *DamageFXStore::Find_Damage_FX(Utf8String name)
{
    auto it = m_dfxmap.find(g_theNameKeyGenerator->Name_To_Key(name.Str()));

    if (it == m_dfxmap.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

void DamageFXStore::Parse_Damage_FX_Definition(INI *ini)
{
    DamageFX &fx = g_theDamageFXStore->m_dfxmap[g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token())];
    fx.Clear();
    ini->Init_From_INI(&fx, DamageFX::Get_Field_Parse());
}

void DamageFXStore::Parse_Damage_FX(INI *ini, void *formal, void *store, const void *user_data)
{
    const DamageFX **damage_fx = static_cast<const DamageFX **>(store);
    const char *name = ini->Get_Next_Token();

    if (strcasecmp(name, "None") == 0) {
        *damage_fx = nullptr;
    } else {
        const DamageFX *fx = g_theDamageFXStore->Find_Damage_FX(name);
        captainslog_dbgassert(fx != nullptr, "DamageFX %s not found!", name);
        *damage_fx = fx;
    }
}
