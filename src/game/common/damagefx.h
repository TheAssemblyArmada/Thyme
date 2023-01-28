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

    DFX m_dfx[9][4];
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

private:
    damagefxmap_t m_dfxmap;
};

#ifdef GAME_DLL
extern DamageFXStore *&g_theDamageFXStore;
#else
extern DamageFXStore *g_theDamageFXStore;
#endif
