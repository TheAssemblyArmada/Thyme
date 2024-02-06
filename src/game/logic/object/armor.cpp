/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Armor
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "armor.h"

#ifndef GAME_DLL
ArmorStore *g_theArmorStore;
#endif

float ArmorTemplate::Adjust_Damage(DamageType t, float damage)
{
    if (t == DAMAGE_UNRESISTABLE) {
        return damage;
    }

    if (t == DAMAGE_SUBDUAL_UNRESISTABLE) {
        return damage;
    }

    float adjusted_damage = damage * m_damageCoefficient[t];

    if (adjusted_damage < 0.0f) {
        return 0.0f;
    }

    return adjusted_damage;
}

void ArmorTemplate::Parse_Armor_Coefficients(INI *ini, void *formal, void *store, const void *user_data)
{
    ArmorTemplate *armor = static_cast<ArmorTemplate *>(formal);
    const char *name = ini->Get_Next_Token();
    float value = ini->Scan_PercentToReal(ini->Get_Next_Token());

    if (strcasecmp(name, "Default") == 0) {
        for (int i = 0; i < DAMAGE_NUM_TYPES; ++i) {
            armor->m_damageCoefficient[i] = value;
        }
    } else {
        armor->m_damageCoefficient[BitFlags<DAMAGE_NUM_TYPES>::Get_Single_Bit_From_Name(name)] = value;
    }
}

const ArmorTemplate *ArmorStore::Find_Armor_Template(Utf8String name) const
{
    auto it = m_armorTemplates.find(g_theNameKeyGenerator->Name_To_Key(name.Str()));

    if (it == m_armorTemplates.end()) {
        return nullptr;
    } else {
        return &it->second;
    }
}

void ArmorStore::Parse_Armor_Definition(INI *ini)
{
    // clang-format off
    static const FieldParse myFieldParse[] = 
    {
        {"Armor", &ArmorTemplate::Parse_Armor_Coefficients, nullptr, 0},
        {nullptr, nullptr, nullptr, 0}
    };
    // clang-format on

    ArmorTemplate &armor = g_theArmorStore->m_armorTemplates[g_theNameKeyGenerator->Name_To_Key(ini->Get_Next_Token())];
    armor.Clear();
    ini->Init_From_INI(&armor, myFieldParse);
}

void ArmorStore::Parse_Armor_Template(INI *ini, void *formal, void *store, const void *user_data)
{
    const char *name = ini->Get_Next_Token();
    const ArmorTemplate **armor = static_cast<const ArmorTemplate **>(store);

    if (strcasecmp(name, "None") == 0) {
        *armor = nullptr;
    } else {
        const ArmorTemplate *tmplate = g_theArmorStore->Find_Armor_Template(name);
        captainslog_dbgassert(tmplate != nullptr, "ArmorTemplate %s not found!", name);
        *armor = tmplate;
    }
}
