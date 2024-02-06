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

class FXList;

class DamageFX
{
public:
    DamageFX() {}

    void Clear()
    {
        for (int i = 0; i < DAMAGE_NUM_TYPES; i++) {
            for (int j = 0; j < VETERANCY_COUNT; j++) {
                m_dfx[i][j].Clear();
            }
        }
    }

    unsigned int Get_Damage_FX_Throttle_Time(DamageType t, const Object *obj) const;
    void Do_Damage_FX(DamageType t, float damage_amount, const Object *source, const Object *victim) const;
    FXList *Get_Damage_FX_List(DamageType t, float damage_amount, const Object *obj) const;

    static const FieldParse *Get_Field_Parse();
    static void Parse_Amount(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Major_FX_List(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Minor_FX_List(INI *ini, void *formal, void *store, const void *user_data);
    static void Parse_Time(INI *ini, void *formal, void *store, const void *user_data);

private:
    struct DFX
    {
        DFX() { Clear(); }

        void Clear()
        {
            m_amount = 0.0f;
            m_majorFXList = nullptr;
            m_minorFXList = nullptr;
            m_time = 0;
        }

        float m_amount;
        FXList *m_majorFXList;
        FXList *m_minorFXList;
        unsigned int m_time;
    };

    DFX m_dfx[DAMAGE_NUM_TYPES][VETERANCY_COUNT];
};

#ifdef THYME_USE_STLPORT
using damagefxmap_t = std::hash_map<NameKeyType, DamageFX, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#else
using damagefxmap_t = std::unordered_map<NameKeyType, DamageFX, rts::hash<NameKeyType>, std::equal_to<NameKeyType>>;
#endif

class DamageFXStore : public SubsystemInterface
{
public:
    DamageFXStore() { m_dfxmap.clear(); }
    virtual ~DamageFXStore() override { m_dfxmap.clear(); }
    virtual void Init() override {}
    virtual void Reset() override {}
    virtual void Update() override {}

    const DamageFX *Find_Damage_FX(Utf8String name);
    static void Parse_Damage_FX_Definition(INI *ini);
    static void Parse_Damage_FX(INI *ini, void *formal, void *store, const void *user_data);

private:
    damagefxmap_t m_dfxmap;
};

#ifdef GAME_DLL
extern DamageFXStore *&g_theDamageFXStore;
#else
extern DamageFXStore *g_theDamageFXStore;
#endif
