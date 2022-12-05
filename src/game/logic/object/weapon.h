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
#include "audioeventrts.h"
#include "coord.h"
#include "gametype.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "snapshot.h"
#include "weaponset.h"
#include <list>
#include <vector>

class FXList;
class INI;
class Object;
class ObjectCreationList;
class ParticleSystemTemplate;
class ThingTemplate;

enum AttackType
{
    ATTACK_TYPE_PER_SHOT,
    ATTACK_TYPE_PER_ATTACK,
    ATTACK_TYPE_PER_CLIP,
};

enum AutoReloadsClip
{
    AUTO_RELOADS_CLIP_YES,
    AUTO_RELOADS_CLIP_NO,
    AUTO_RELOADS_CLIP_RETURN_TO_BASE,
};

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

struct HistoricWeaponDamageInfo
{
    unsigned int frame;
    Coord3D location;
    HistoricWeaponDamageInfo(unsigned int f, Coord3D loc) : frame(f), location(loc) {}
};

class WeaponTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(WeaponTemplate);

public:
    WeaponTemplate();

    virtual ~WeaponTemplate() override;
    WeaponTemplate &operator=(const WeaponTemplate &that);

    float Get_Min_Target_Pitch() const { return m_minTargetPitch; }
    float Get_Max_Target_Pitch() const { return m_maxTargetPitch; }
    int Get_Shots_Per_Barrel() const { return m_shotsPerBarrel; }
    unsigned int Get_Suspend_FX_Delay() const { return m_suspendFXDelay; }
    Utf8String Get_Name() const { return m_name; }

private:
    WeaponTemplate *m_nextTemplate;
    Utf8String m_name;
    NameKeyType m_nameKey;
    Utf8String m_projectileStreamName;
    Utf8String m_laserName;
    Utf8String m_laserBoneName;
    float m_primaryDamage;
    float m_primaryDamageRadius;
    float m_secondaryDamage;
    float m_secondaryDamageRadius;
    float m_shockWaveAmount;
    float m_shockWaveRadius;
    float m_shockWaveTaperOff;
    float m_attackRange;
    float m_minimumAttackRange;
    float m_requestAssistRange;
    float m_aimDelta;
    float m_scatterRadius;
    float m_scatterTargetScalar;
    std::vector<Coord2D> m_scatterTarget;
    DamageType m_damageType;
    DeathType m_deathType;
    float m_weaponSpeed;
    float m_minWeaponSpeed;
    bool m_scaleWeaponSpeed;
    float m_weaponRecoil;
    float m_minTargetPitch;
    float m_maxTargetPitch;
    float m_radiusDamageAngle;
    Utf8String m_projectileObject;
    ThingTemplate *m_projectileTemplate;
    Utf8String m_fireOCLName[4];
    Utf8String m_projectileDetonationOCLName[4];
    ParticleSystemTemplate *m_projectileExhaust[4];
    ObjectCreationList *m_fireOCL[4];
    ObjectCreationList *m_projectileDetonationOCL[4];
    FXList *m_fireFX[4];
    FXList *m_projectileDetonateFX[4];
    AudioEventRTS m_fireSound;
    unsigned int m_fireSoundLoopTime;
    WeaponBonusSet *m_extraBonus;
    int m_clipSize;
    int m_clipReloadTime;
    int m_minDelayBetweenShots;
    int m_maxDelayBetweenShots;
    int m_continuousFireOneShotsNeeded;
    int m_continuousFireTwoShotsNeeded;
    unsigned int m_continuousFireCoastFrames;
    unsigned int m_autoReloadWhenIdle;
    int m_shotsPerBarrel;
    int m_antiMask;
    int m_affectsMask;
    int m_collideMask;
    bool m_damageDealtAtSelfPosition;
    AutoReloadsClip m_autoReloadsClip;
    AttackType m_preAttackType;
    unsigned int m_historicBonusTime;
    float m_historicBonusRadius;
    int m_historicBonusCount;
    WeaponTemplate *m_historicBonusWeapon;
    bool m_leechRangeWeapon;
    bool m_capableOfFollowingWaypoint;
    bool m_showAmmoPips;
    bool m_allowAttackGarrisonedBldgs;
    bool m_playFXWhenStealthed;
    unsigned int m_preAttackDelay;
    float m_continueAttackRange;
    float m_scatterRadiusVsInfantry;
    ObjectStatusTypes m_damageStatusType;
    unsigned int m_suspendFXDelay;
    bool m_missileCallsOnDie;
    std::list<HistoricWeaponDamageInfo> m_historicDamage;
};

class Weapon : public MemoryPoolObject, public SnapShot
{
    IMPLEMENT_POOL(Weapon);

public:
    enum WeaponStatus
    {
        READY_TO_FIRE,
        OUT_OF_AMMO,
        BETWEEN_FIRING_SHOTS,
        RELOADING_CLIP,
    };

    Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot);
    Weapon(const Weapon &that);

    virtual ~Weapon() override {}
    virtual void CRC_Snapshot(Xfer *xfer) override;
    virtual void Xfer_Snapshot(Xfer *xfer) override;
    virtual void Load_Post_Process() override;

    Weapon &operator=(const Weapon &that);

    bool Is_Within_Attack_Range(const Object *source, const Object *target) const;
    bool Is_Within_Attack_Range(const Object *source, const Coord3D *target) const;
    float Get_Attack_Range(const Object *source) const;

    unsigned int Get_Suspend_FX_Delay() const { return m_suspendFXDelay; }

#ifdef GAME_DLL
    Weapon *Hook_Ctor(const WeaponTemplate *tmpl, WeaponSlotType wslot) { return new (this) Weapon(tmpl, wslot); }
#endif

private:
    const WeaponTemplate *m_template;
    WeaponSlotType m_wslot;
    WeaponStatus m_status;
    unsigned int m_ammoInClip;
    unsigned int m_whenWeCanFireAgain;
    unsigned int m_whenPreAttackFinished;
    unsigned int m_whenLastReloadStarted;
    unsigned int m_lastFireFrame;
    unsigned int m_suspendFXDelay;
    ObjectID m_projectileStreamID;
    int m_maxShotCount;
    int m_curBarrel;
    int m_numShotsForCurBarrel;
    std::vector<int> m_scatterTargetIndexes;
    bool m_pitchLimited;
    bool m_leechWeaponRangeActive;
};

class WeaponStore
{
public:
    const WeaponTemplate *Find_Weapon_Template(Utf8String name) const;
    void Create_And_Fire_Temp_Weapon(const WeaponTemplate *tmpl, const Object *obj, const Coord3D *pos);
    static void Parse_Weapon_Template(INI *ini, void *, void *store, const void *);
    Weapon *Allocate_New_Weapon(const WeaponTemplate *tmpl, WeaponSlotType wslot) const;
};

#ifdef GAME_DLL
extern WeaponStore *&g_theWeaponStore;
#else
extern WeaponStore *g_theWeaponStore;
#endif
