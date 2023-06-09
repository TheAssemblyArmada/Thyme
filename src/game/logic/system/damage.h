/**
 * @file
 *
 * @author Jonathan Wilson
 *
 * @brief Damage
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
#include "object.h"
#include "weaponset.h"

class DamageInfoInput : public SnapShot
{
public:
    DamageInfoInput() :
        m_sourceID(INVALID_OBJECT_ID),
        m_thingTemplate(nullptr),
        m_playerMask(0),
        m_damageType(DAMAGE_EXPLOSION),
        m_damageStatusType(OBJECT_STATUS_NONE),
        m_unk1(11),
        m_deathType(DEATH_NORMAL),
        m_amount(0.0f),
        m_unk2(false),
        m_shockWaveAmount(0.0f),
        m_shockWaveRadius(0.0f),
        m_shockWaveTaperOff(0.0f)
    {
        m_shockWavePos.Zero();
    }

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    ObjectID m_sourceID;
    ThingTemplate *m_thingTemplate;
    uint16_t m_playerMask;
    DamageType m_damageType;
    ObjectStatusTypes m_damageStatusType;
    unsigned int m_unk1; // seems to be some kind of enum, set to 11 in the constructor and to 9 in PoisonedBehavior::Update
    DeathType m_deathType;
    float m_amount;
    bool m_unk2; // set to true in Object::Kill
    Coord3D m_shockWavePos;
    float m_shockWaveAmount;
    float m_shockWaveRadius;
    float m_shockWaveTaperOff;
};

class DamageInfoOutput : public SnapShot
{
public:
    DamageInfoOutput() : m_actualDamageDealt(0.0f), m_actualDamageClipped(0.0f), m_noEffect(false) {}

    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    float m_actualDamageDealt;
    float m_actualDamageClipped;
    bool m_noEffect;
};

class DamageInfo : public SnapShot
{
public:
    virtual void CRC_Snapshot(Xfer *xfer) override {}
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override {}

    DamageInfoInput m_in;
    DamageInfoOutput m_out;
};

#ifdef GAME_DLL
extern BitFlags<DAMAGE_NUM_TYPES> &DAMAGE_TYPE_FLAGS_ALL;
#else
extern BitFlags<DAMAGE_NUM_TYPES> DAMAGE_TYPE_FLAGS_ALL;
#endif

extern BitFlags<DAMAGE_NUM_TYPES> DAMAGE_TYPE_FLAGS_NONE;

void Init_Damage_Type_Masks();
