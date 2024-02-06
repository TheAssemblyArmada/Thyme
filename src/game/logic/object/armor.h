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
#pragma once
#include "always.h"
#include "namekeygenerator.h"
#include "rtsutils.h"
#include "subsysteminterface.h"
#include "weaponset.h"

#ifdef THYME_USE_STLPORT
#include <hash_map>
#else
#include <unordered_map>
#endif

class ArmorTemplate
{
public:
    ArmorTemplate() { Clear(); }

    void Clear()
    {
        for (int i = 0; i < DAMAGE_NUM_TYPES; i++) {
            m_damageCoefficient[i] = 1.0f;
        }
    }

    float Adjust_Damage(DamageType t, float damage);

    static void Parse_Armor_Coefficients(INI *ini, void *formal, void *store, const void *user_data);

private:
    float m_damageCoefficient[DAMAGE_NUM_TYPES];
};

#ifdef THYME_USE_STLPORT
using armormap_t = std::hash_map<NameKeyType, ArmorTemplate, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#else
using armormap_t = std::unordered_map<NameKeyType, ArmorTemplate, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#endif

class ArmorStore : public SubsystemInterface
{
public:
    ArmorStore() { m_armorTemplates.clear(); }
    virtual ~ArmorStore() override { m_armorTemplates.clear(); }
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    const ArmorTemplate *Find_Armor_Template(Utf8String name) const;

    static void Parse_Armor_Definition(INI *ini);
    static void Parse_Armor_Template(INI *ini, void *formal, void *store, const void *user_data);

private:
    armormap_t m_armorTemplates;
};

#ifdef GAME_DLL
extern ArmorStore *&g_theArmorStore;
#else
extern ArmorStore *g_theArmorStore;
#endif
