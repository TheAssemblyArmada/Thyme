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

#include "audioeventrts.h"
#include "coord.h"
#include "mempoolobj.h"
#include "namekeygenerator.h"
#include "subsysteminterface.h"
#include "weaponset.h"
#include <list>
#include <vector>

enum DeathType : int32_t;

class INI;
class ThingTemplate;
class FXList;
class ParticleSystemTemplate;
class Weapon;

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

enum AntiWeapon
{
    ANTI_WEAPON_AIRBORNE_VEHICLE,
    ANTI_WEAPON_GROUND,
    ANTI_WEAPON_PROJECTILE,
    ANTI_WEAPON_SMALL_MISSILE,
    ANTI_WEAPON_MINE,
    ANTI_WEAPON_AIRBORNE_INFANTRY,
    ANTI_WEAPON_BALLISTIC_MISSILE,
    ANTI_WEAPON_PARACHUTE,
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
    uint32_t frame;
    Coord3D location;
};

class WeaponTemplate : public MemoryPoolObject
{
    IMPLEMENT_POOL(WeaponTemplate);

public:
    WeaponTemplate &operator=(WeaponTemplate &rhs);

    bool Has_Next_Template() const { return m_nextTemplate != nullptr; }
    const Utf8String &Get_Name() const { return m_name; }
    NameKeyType Get_Name_Key() const { return m_nameKey; }
    const Utf8String &Get_Projectile_Name() const { return m_projectileObject; }
    const AudioEventRTS &Get_Fire_Sound() const { return m_fireSound; }
    uint32_t Get_Damage_Type() const { return m_damageType; }

    // wb: 0x006D6B4E friend?
    WeaponTemplate *Clear_Next_Template()
    {
        auto *ret = m_nextTemplate;
        m_nextTemplate = nullptr;
        return ret;
    }
    void Clear_Projectile_Name() { m_projectileObject.Clear(); }
    void Set_Name(const Utf8String &newName);
    void Set_Next(WeaponTemplate *next) { m_nextTemplate = next; }
    // wb: 0x006D3E85
    void Reset() { m_historicDamage.clear(); }
    void Deal_Damage_Internal(
        uint32_t sourceID, uint32_t victimID, Coord3D *pos, WeaponBonus &bonus, bool isProjectileDetonation);
    void PostProcessLoad();

    static void Parse_Scatter_Target(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Weapon_Bonus_Set(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Shot_Delay(INI *ini, void *formal, void *store, void const *user_data);
    static void Parse_Weapon_Template(INI *ini, void *formal, void *store, const void *user_data);
    static FieldParse *Get_Weapon_Template_Field_Parse_Table() { return s_fieldParseTable; }

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
    uint32_t unk;
    Utf8String m_fireOCL[VETERANCY_COUNT];
    Utf8String m_projectileDetonationOCL[VETERANCY_COUNT];
    ParticleSystemTemplate *m_projectileExhaust[VETERANCY_COUNT];
    Utf8String m_projectileName[VETERANCY_COUNT];
    ThingTemplate *m_projectileTmpl[VETERANCY_COUNT];
    FXList *m_fireFX[VETERANCY_COUNT];
    FXList *m_projectileDetonateFX[VETERANCY_COUNT];
    AudioEventRTS m_fireSound;
    uint32_t m_fireSoundLoopTime;
    WeaponBonusSet *m_extraBonus;
    int32_t m_clipSize;
    int32_t m_clipReloadTime;
    int32_t m_minDelayBetweenShots;
    int32_t m_maxDelayBetweenShots;
    int32_t m_continuousFireOneShotsNeeded;
    int32_t m_continuousFireTwoShotsNeeded;
    uint32_t m_continuousFireCoastFrames;
    uint32_t m_autoReloadWhenIdle;
    int32_t m_shotsPerBarrel;
    int32_t m_antiMask;
    int32_t m_affectsMask;
    int32_t m_collideMask;
    int32_t m_damageDealtAtSelfPosition;
    AutoReloadsClip m_autoReloadsClip;
    AttackType m_preAttackType;
    uint32_t m_historicBonusTime;
    float m_historicBonusRadius;
    int32_t m_historicBonusCount;
    WeaponTemplate *m_historicBonusWeapon;
    bool m_leechRangeWeapon;
    bool m_capableOfFollowingWaypoint;
    bool m_showAmmoPips;
    bool m_allowAttackGarrisonedBldgs;
    bool m_playFXWhenStealthed;
    uint32_t m_preAttackDelay;
    float m_continueAttackRange;
    float m_scatterRadiusVsInfantry;
    ObjectStatusTypes m_damageStatusType;
    uint32_t m_suspendFXDelay;
    bool m_missileCallsOnDie;
    // TODO: Consider replacing the container type
    std::list<HistoricWeaponDamageInfo> m_historicDamage;

    static FieldParse s_fieldParseTable[];
};

class WeaponStore : public SubsystemInterface
{
    struct WeaponDelayedDamageInfo
    {
        WeaponTemplate *m_delayedWeapon;
        Coord3D m_delayDamagePos;
        uint32_t m_delayDamageFrame;
        uint32_t m_delaySourceID;
        uint32_t m_delayIntendedVictimID;
        WeaponBonus m_bonus;
    };

public:
    WeaponStore() {}
    virtual ~WeaponStore() override;

    virtual void Init() override {}
    virtual void PostProcessLoad() override;
    virtual void Reset() override;
    virtual void Update() override;

    WeaponTemplate *Find_Weapon_Template(Utf8String name);
    // zh: 0x004C4DD0
    WeaponStore *Hook_Ctor() { return new (this) WeaponStore; }

private:
    WeaponTemplate *Find_Weapon_Template_By_Name_Key(NameKeyType); //??
    Weapon Make_Weapon(WeaponTemplate *tmpl);
    WeaponTemplate *Find_Weapon_Template_Private(NameKeyType key);
    WeaponTemplate *New_Weapon_Template(Utf8String name);
    WeaponTemplate *New_Override(WeaponTemplate *weapon_template);
    // wb: 0x006D6AA9
    void Delete_All_Delayed_Damage() { m_weaponDDI.clear(); }
    void Reset_Weapon_Templates();
    // wb: 0x006D6B8B
    void Set_Delayed_Damage(
        WeaponTemplate *weapon, Coord3D *pos, uint32_t whichFrame, uint32_t sourceID, uint32_t victimID, WeaponBonus &bonus)
    {
        m_weaponDDI.push_front(WeaponDelayedDamageInfo{ weapon, *pos, whichFrame, sourceID, victimID, bonus });
    }

public:
    static void Parse_Weapon_Template_Definition(INI *ini);

private:
    std::vector<WeaponTemplate *> m_weaponTemplateVector;
    // TODO: Consider replacing container type
    std::list<WeaponDelayedDamageInfo> m_weaponDDI;
};

#ifdef GAME_DLL
extern WeaponStore *&g_theWeaponStore;
#else
extern WeaponStore *g_theWeaponStore;
#endif
